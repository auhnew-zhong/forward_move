#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include <numeric>

/**
 * 综合示例：右值引用和完美转发的实际应用
 * 
 * 本文件展示了右值引用和完美转发在实际项目中的应用：
 * 1. 智能指针工厂
 * 2. 事件系统
 * 3. 任务调度器
 * 4. 缓存系统
 * 5. 函数式编程工具
 */

/**
 * 1. 智能指针工厂 - 演示完美转发在对象创建中的应用
 */
namespace SmartPointerFactory {
    
    // 基础资源类
    class Resource {
    private:
        std::string name;
        std::vector<int> data;
        int id;
        
    public:
        Resource(const std::string& n, int i, size_t dataSize = 0) 
            : name(n), data(dataSize), id(i) {
            std::iota(data.begin(), data.end(), 0);
            std::cout << "Resource 构造: " << name << " (id: " << id 
                      << ", 数据大小: " << dataSize << ")\n";
        }
        
        Resource(const Resource& other) 
            : name(other.name + "_copy"), data(other.data), id(other.id) {
            std::cout << "Resource 拷贝构造: " << name << "\n";
        }
        
        Resource(Resource&& other) noexcept 
            : name(std::move(other.name)), data(std::move(other.data)), id(other.id) {
            other.id = -1;
            std::cout << "Resource 移动构造: " << name << "\n";
        }
        
        ~Resource() {
            std::cout << "Resource 析构: " << name << " (id: " << id << ")\n";
        }
        
        const std::string& getName() const { return name; }
        int getId() const { return id; }
        size_t getDataSize() const { return data.size(); }
    };
    
    // 完美转发工厂函数
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique_factory(Args&&... args) {
        std::cout << "工厂创建对象，参数数量: " << sizeof...(args) << "\n";
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    
    template<typename T, typename... Args>
    std::shared_ptr<T> make_shared_factory(Args&&... args) {
        std::cout << "共享指针工厂创建对象，参数数量: " << sizeof...(args) << "\n";
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
    
    void demonstrate() {
        std::cout << "\n=== 智能指针工厂演示 ===\n";
        
        // 使用不同类型的参数
        auto resource1 = make_unique_factory<Resource>("工厂资源1", 100, 1000);
        
        std::string name = "工厂资源2";
        auto resource2 = make_shared_factory<Resource>(name, 200, 500);
        
        auto resource3 = make_unique_factory<Resource>(std::string("临时名称"), 300);
        
        std::cout << "创建的资源:\n";
        std::cout << "- " << resource1->getName() << " (数据: " << resource1->getDataSize() << ")\n";
        std::cout << "- " << resource2->getName() << " (数据: " << resource2->getDataSize() << ")\n";
        std::cout << "- " << resource3->getName() << " (数据: " << resource3->getDataSize() << ")\n";
    }
}

/**
 * 2. 事件系统 - 演示移动语义在事件处理中的应用
 */
namespace EventSystem {
    
    // 事件基类
    class Event {
    private:
        std::string type;
        std::chrono::steady_clock::time_point timestamp;
        
    public:
        Event(std::string t) : type(std::move(t)), timestamp(std::chrono::steady_clock::now()) {
            std::cout << "Event 创建: " << type << "\n";
        }
        
        Event(const Event& other) : type(other.type), timestamp(other.timestamp) {
            std::cout << "Event 拷贝: " << type << "\n";
        }
        
        Event(Event&& other) noexcept 
            : type(std::move(other.type)), timestamp(other.timestamp) {
            std::cout << "Event 移动: " << type << "\n";
        }
        
        virtual ~Event() {
            std::cout << "Event 析构: " << type << "\n";
        }
        
        const std::string& getType() const { return type; }
        auto getTimestamp() const { return timestamp; }
    };
    
    // 具体事件类型
    class MouseEvent : public Event {
    private:
        int x, y;
        
    public:
        MouseEvent(int x_pos, int y_pos) 
            : Event("MouseEvent"), x(x_pos), y(y_pos) {
            std::cout << "MouseEvent 创建: (" << x << ", " << y << ")\n";
        }
        
        int getX() const { return x; }
        int getY() const { return y; }
    };
    
    class KeyboardEvent : public Event {
    private:
        char key;
        
    public:
        KeyboardEvent(char k) : Event("KeyboardEvent"), key(k) {
            std::cout << "KeyboardEvent 创建: '" << key << "'\n";
        }
        
        char getKey() const { return key; }
    };
    
    // 事件处理器
    class EventHandler {
    private:
        std::queue<std::unique_ptr<Event>> eventQueue;
        
    public:
        // 完美转发添加事件
        template<typename EventType, typename... Args>
        void emplace_event(Args&&... args) {
            auto event = std::make_unique<EventType>(std::forward<Args>(args)...);
            eventQueue.push(std::move(event));
            std::cout << "事件已添加到队列\n";
        }
        
        // 移动语义添加事件
        void add_event(std::unique_ptr<Event> event) {
            std::cout << "通过移动添加事件: " << event->getType() << "\n";
            eventQueue.push(std::move(event));
        }
        
        // 处理事件
        void process_events() {
            std::cout << "\n处理事件队列 (大小: " << eventQueue.size() << ")\n";
            
            while (!eventQueue.empty()) {
                auto event = std::move(eventQueue.front());
                eventQueue.pop();
                
                std::cout << "处理事件: " << event->getType() << "\n";
                
                // 根据事件类型进行不同处理
                if (auto mouseEvent = dynamic_cast<MouseEvent*>(event.get())) {
                    std::cout << "  鼠标位置: (" << mouseEvent->getX() 
                              << ", " << mouseEvent->getY() << ")\n";
                } else if (auto keyEvent = dynamic_cast<KeyboardEvent*>(event.get())) {
                    std::cout << "  按键: '" << keyEvent->getKey() << "'\n";
                }
            }
        }
        
        size_t getQueueSize() const { return eventQueue.size(); }
    };
    
    void demonstrate() {
        std::cout << "\n=== 事件系统演示 ===\n";
        
        EventHandler handler;
        
        // 使用 emplace 直接构造事件
        handler.emplace_event<MouseEvent>(100, 200);
        handler.emplace_event<KeyboardEvent>('A');
        handler.emplace_event<MouseEvent>(300, 400);
        
        // 使用移动语义添加事件
        auto keyEvent = std::make_unique<KeyboardEvent>('B');
        handler.add_event(std::move(keyEvent));
        
        // 处理所有事件
        handler.process_events();
    }
}

/**
 * 3. 任务调度器 - 演示函数对象的移动和完美转发
 */
namespace TaskScheduler {
    
    // 任务类型
    using Task = std::function<void()>;
    
    class Scheduler {
    private:
        std::vector<Task> tasks;
        std::map<std::string, Task> namedTasks;
        
    public:
        // 完美转发添加任务
        template<typename Func, typename... Args>
        void schedule_task(Func&& func, Args&&... args) {
            // 使用 lambda 捕获参数并完美转发
            auto scheduler_ptr = this;
            tasks.emplace_back([scheduler_ptr, func = std::forward<Func>(func), 
                               args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                // 手动展开tuple参数调用函数（C++11兼容）
                scheduler_ptr->call_with_tuple(std::move(func), std::move(args), 
                               typename std::make_index_sequence<sizeof...(Args)>{});
            });
            
            std::cout << "任务已调度 (总数: " << tasks.size() << ")\n";
        }
        
        // 移动语义添加命名任务
        void schedule_named_task(std::string name, Task task) {
            std::cout << "添加命名任务: " << name << "\n";
            namedTasks[std::move(name)] = std::move(task);
        }
        
        // 执行所有任务
        void execute_all() {
            std::cout << "\n执行所有任务...\n";
            
            // 执行普通任务
            for (auto& task : tasks) {
                task();
            }
            tasks.clear();
            
            // 执行命名任务
            for (auto& pair : namedTasks) {
                std::cout << "执行命名任务: " << pair.first << "\n";
                pair.second();
            }
        }
        
        size_t getTaskCount() const { return tasks.size() + namedTasks.size(); }
        
    private:
        // 辅助函数用于展开tuple参数
        template<typename Func, typename Tuple, std::size_t... I>
        void call_with_tuple(Func&& func, Tuple&& tuple, std::index_sequence<I...>) {
            func(std::get<I>(std::forward<Tuple>(tuple))...);
        }
    };
    
    // 示例任务函数
    void simple_task() {
        std::cout << "  执行简单任务\n";
    }
    
    void parameterized_task(const std::string& message, int value) {
        std::cout << "  执行参数化任务: " << message << " (值: " << value << ")\n";
    }
    
    class TaskObject {
    private:
        std::string name;
        
    public:
        TaskObject(std::string n) : name(std::move(n)) {
            std::cout << "TaskObject 创建: " << name << "\n";
        }
        
        TaskObject(const TaskObject& other) : name(other.name + "_copy") {
            std::cout << "TaskObject 拷贝: " << name << "\n";
        }
        
        TaskObject(TaskObject&& other) noexcept : name(std::move(other.name)) {
            std::cout << "TaskObject 移动: " << name << "\n";
        }
        
        void execute() const {
            std::cout << "  TaskObject 执行: " << name << "\n";
        }
    };
    
    void demonstrate() {
        std::cout << "\n=== 任务调度器演示 ===\n";
        
        Scheduler scheduler;
        
        // 调度不同类型的任务
        scheduler.schedule_task(simple_task);
        scheduler.schedule_task(parameterized_task, "Hello", 42);
        
        // 使用 lambda
        std::string message = "Lambda 任务";
        scheduler.schedule_task([](const std::string& msg) {
            std::cout << "  执行 Lambda: " << msg << "\n";
        }, message);
        
        // 使用对象方法
        TaskObject obj("任务对象");
        scheduler.schedule_task([obj = std::move(obj)]() mutable {
            obj.execute();
        });
        
        // 添加命名任务
        scheduler.schedule_named_task("清理任务", []() {
            std::cout << "  执行清理操作\n";
        });
        
        std::cout << "总任务数: " << scheduler.getTaskCount() << "\n";
        
        // 执行所有任务
        scheduler.execute_all();
    }
}

/**
 * 4. 缓存系统 - 演示移动语义在缓存管理中的应用
 */
namespace CacheSystem {
    
    template<typename Key, typename Value>
    class LRUCache {
    private:
        struct CacheNode {
            Key key;
            Value value;
            
            CacheNode(Key k, Value v) : key(std::move(k)), value(std::move(v)) {}
        };
        
        std::vector<CacheNode> cache;
        size_t maxSize;
        
    public:
        explicit LRUCache(size_t size) : maxSize(size) {
            cache.reserve(size);
        }
        
        // 完美转发插入
        template<typename K, typename V>
        void put(K&& key, V&& value) {
            // 查找是否已存在
            auto it = std::find_if(cache.begin(), cache.end(),
                [&key](const CacheNode& node) { return node.key == key; });
            
            if (it != cache.end()) {
                // 更新现有值并移动到末尾
                it->value = std::forward<V>(value);
                std::rotate(it, it + 1, cache.end());
                std::cout << "缓存更新: " << key << "\n";
            } else {
                // 添加新项
                if (cache.size() >= maxSize) {
                    std::cout << "缓存满，移除最旧项: " << cache.front().key << "\n";
                    cache.erase(cache.begin());
                }
                
                cache.emplace_back(std::forward<K>(key), std::forward<V>(value));
                std::cout << "缓存添加: " << key << "\n";
            }
        }
        
        // 获取值（移动语义）
        std::pair<bool, Value> get(const Key& key) {
            auto it = std::find_if(cache.begin(), cache.end(),
                [&key](const CacheNode& node) { return node.key == key; });
            
            if (it != cache.end()) {
                // 移动到末尾（最近使用）
                Value result = std::move(it->value);
                std::rotate(it, it + 1, cache.end());
                std::cout << "缓存命中: " << key << "\n";
                return {true, std::move(result)};
            }
            
            std::cout << "缓存未命中: " << key << "\n";
            // 为没有默认构造函数的类型创建一个临时对象
            static Value empty_value = Value("", 0);
            return {false, empty_value};
        }
        
        void print_cache() const {
            std::cout << "缓存内容 (从旧到新): ";
            for (const auto& node : cache) {
                std::cout << "[" << node.key << "] ";
            }
            std::cout << "\n";
        }
        
        size_t size() const { return cache.size(); }
    };
    
    // 大对象用于演示移动语义的优势
    class LargeObject {
    private:
        std::vector<int> data;
        std::string name;
        
    public:
        LargeObject(std::string n, size_t size) 
            : data(size), name(std::move(n)) {
            std::iota(data.begin(), data.end(), 0);
            std::cout << "LargeObject 构造: " << name << " (大小: " << size << ")\n";
        }
        
        LargeObject(const LargeObject& other) 
            : data(other.data), name(other.name + "_copy") {
            std::cout << "LargeObject 拷贝: " << name << "\n";
        }
        
        LargeObject(LargeObject&& other) noexcept 
            : data(std::move(other.data)), name(std::move(other.name)) {
            std::cout << "LargeObject 移动: " << name << "\n";
        }
        
        LargeObject& operator=(const LargeObject& other) {
            if (this != &other) {
                data = other.data;
                name = other.name + "_assigned";
                std::cout << "LargeObject 拷贝赋值: " << name << "\n";
            }
            return *this;
        }
        
        LargeObject& operator=(LargeObject&& other) noexcept {
            if (this != &other) {
                data = std::move(other.data);
                name = std::move(other.name);
                std::cout << "LargeObject 移动赋值: " << name << "\n";
            }
            return *this;
        }
        
        ~LargeObject() {
            std::cout << "LargeObject 析构: " << name << "\n";
        }
        
        const std::string& getName() const { return name; }
        size_t getDataSize() const { return data.size(); }
    };
    
    void demonstrate() {
        std::cout << "\n=== 缓存系统演示 ===\n";
        
        LRUCache<std::string, LargeObject> cache(3);
        
        // 添加对象到缓存
        cache.put("obj1", LargeObject("对象1", 1000));
        cache.put("obj2", LargeObject("对象2", 2000));
        cache.put("obj3", LargeObject("对象3", 3000));
        
        cache.print_cache();
        
        // 访问缓存
        auto [found1, obj1] = cache.get("obj1");
        if (found1) {
            std::cout << "获取到对象: " << obj1.getName() 
                      << " (数据大小: " << obj1.getDataSize() << ")\n";
        }
        
        cache.print_cache();
        
        // 添加新对象（会触发LRU淘汰）
        cache.put("obj4", LargeObject("对象4", 4000));
        
        cache.print_cache();
        
        // 尝试访问被淘汰的对象
        auto [found2, obj2] = cache.get("obj2");
        if (!found2) {
            std::cout << "对象2已被淘汰\n";
        }
    }
}

/**
 * 5. 函数式编程工具 - 演示完美转发在函数组合中的应用
 */
namespace FunctionalTools {
    
    // 函数组合器
    template<typename F, typename G>
    class Composer {
    private:
        F f;
        G g;
        
    public:
        template<typename F2, typename G2>
        Composer(F2&& f_func, G2&& g_func) 
            : f(std::forward<F2>(f_func)), g(std::forward<G2>(g_func)) {}
        
        template<typename... Args>
        auto operator()(Args&&... args) const 
            -> decltype(f(g(std::forward<Args>(args)...))) {
            return f(g(std::forward<Args>(args)...));
        }
    };
    
    // 完美转发的函数组合
    template<typename F, typename G>
    auto compose(F&& f, G&& g) {
        return Composer<typename std::decay<F>::type, typename std::decay<G>::type>(
            std::forward<F>(f), std::forward<G>(g));
    }
    
    // 管道操作符
    template<typename T, typename F>
    auto operator|(T&& value, F&& func) -> decltype(func(std::forward<T>(value))) {
        return func(std::forward<T>(value));
    }
    
    // 柯里化函数
    template<typename F>
    class Curried {
    private:
        F func;
        
    public:
        template<typename F2>
        explicit Curried(F2&& f) : func(std::forward<F2>(f)) {}
        
        template<typename Arg>
        auto operator()(Arg&& arg) const {
            return [func = this->func, arg = std::forward<Arg>(arg)](auto&&... rest) mutable {
                return func(std::move(arg), std::forward<decltype(rest)>(rest)...);
            };
        }
    };
    
    template<typename F>
    auto curry(F&& func) {
        return Curried<typename std::decay<F>::type>(std::forward<F>(func));
    }
    
    void demonstrate() {
        std::cout << "\n=== 函数式编程工具演示 ===\n";
        
        // 基础函数
        auto add = [](int a, int b) { return a + b; };
        auto multiply = [](int x) { return x * 2; };
        auto toString = [](int x) { return std::to_string(x); };
        
        // 函数组合
        auto addThenMultiply = compose(multiply, add);
        auto fullPipeline = compose(toString, addThenMultiply);
        
        std::cout << "函数组合结果: " << fullPipeline(3, 4) << "\n";  // (3+4)*2 = "14"
        
        // 管道操作
        auto result = 5 | [](int x) { return x * x; }     // 25
                        | [](int x) { return x + 10; }    // 35
                        | [](int x) { return std::to_string(x); };  // "35"
        
        std::cout << "管道操作结果: " << result << "\n";
        
        // 柯里化
        auto curriedAdd = curry(add);
        auto add5 = curriedAdd(5);
        
        std::cout << "柯里化结果: " << add5(10) << "\n";  // 15
        
        // 复杂的函数组合
        auto complexFunc = [](const std::string& prefix, int value, const std::string& suffix) {
            return prefix + std::to_string(value) + suffix;
        };
        
        auto curriedComplex = curry(curry(complexFunc)("Result: "))(42);
        std::cout << "复杂柯里化: " << curriedComplex("!") << "\n";  // "Result: 42!"
    }
}

/**
 * 性能基准测试
 */
namespace PerformanceBenchmark {
    
    void benchmark_move_vs_copy() {
        std::cout << "\n=== 性能基准测试 ===\n";
        
        const size_t iterations = 10000;
        const size_t objectSize = 1000;
        
        // 测试拷贝性能
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<SmartPointerFactory::Resource> copyVec;
        copyVec.reserve(iterations);
        
        for (size_t i = 0; i < iterations; ++i) {
            SmartPointerFactory::Resource temp("copy_" + std::to_string(i), 
                                               static_cast<int>(i), objectSize);
            copyVec.push_back(temp);  // 拷贝
        }
        
        auto copyTime = std::chrono::high_resolution_clock::now() - start;
        
        // 测试移动性能
        start = std::chrono::high_resolution_clock::now();
        
        std::vector<SmartPointerFactory::Resource> moveVec;
        moveVec.reserve(iterations);
        
        for (size_t i = 0; i < iterations; ++i) {
            SmartPointerFactory::Resource temp("move_" + std::to_string(i), 
                                               static_cast<int>(i), objectSize);
            moveVec.push_back(std::move(temp));  // 移动
        }
        
        auto moveTime = std::chrono::high_resolution_clock::now() - start;
        
        auto copyMs = std::chrono::duration_cast<std::chrono::milliseconds>(copyTime).count();
        auto moveMs = std::chrono::duration_cast<std::chrono::milliseconds>(moveTime).count();
        
        std::cout << "拷贝操作耗时: " << copyMs << " ms\n";
        std::cout << "移动操作耗时: " << moveMs << " ms\n";
        
        if (moveMs > 0) {
            double speedup = static_cast<double>(copyMs) / moveMs;
            std::cout << "移动语义性能提升: " << speedup << "x\n";
        }
    }
}

int main() {
    std::cout << "C++ 右值引用和完美转发综合应用示例\n";
    std::cout << "=====================================\n";
    
    try {
        // 演示各个模块
        SmartPointerFactory::demonstrate();
        EventSystem::demonstrate();
        TaskScheduler::demonstrate();
        CacheSystem::demonstrate();
        FunctionalTools::demonstrate();
        
        // 性能测试
        std::cout << "\n进行性能基准测试...\n";
        PerformanceBenchmark::benchmark_move_vs_copy();
        
        std::cout << "\n=== 所有演示完成 ===\n";
        std::cout << "\n总结：\n";
        std::cout << "1. 智能指针工厂展示了完美转发在对象创建中的应用\n";
        std::cout << "2. 事件系统演示了移动语义在资源管理中的优势\n";
        std::cout << "3. 任务调度器展示了函数对象的高效传递\n";
        std::cout << "4. 缓存系统演示了移动语义在容器操作中的性能提升\n";
        std::cout << "5. 函数式工具展示了完美转发在高阶函数中的应用\n";
        
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
 * 综合应用要点总结：
 * 
 * 1. 工厂模式：
 *    - 使用完美转发避免不必要的拷贝
 *    - 支持任意数量和类型的构造参数
 *    - 保持参数的值类别
 * 
 * 2. 事件系统：
 *    - 移动语义减少事件对象的拷贝开销
 *    - 完美转发支持就地构造事件
 *    - 智能指针管理事件生命周期
 * 
 * 3. 任务调度：
 *    - 函数对象的高效存储和传递
 *    - lambda 表达式的移动捕获
 *    - 参数的完美转发
 * 
 * 4. 缓存系统：
 *    - 大对象的移动而非拷贝
 *    - 容器操作的性能优化
 *    - LRU 算法的高效实现
 * 
 * 5. 函数式编程：
 *    - 高阶函数的零开销抽象
 *    - 函数组合的完美转发
 *    - 柯里化的参数捕获
 * 
 * 6. 性能优势：
 *    - 显著减少拷贝操作
 *    - 提高内存使用效率
 *    - 保持代码的表达力和安全性
 */