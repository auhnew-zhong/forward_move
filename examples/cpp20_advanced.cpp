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
#include <type_traits>
#include <sstream>

/**
 * C++20 风格的移动语义与完美转发示例
 * 
 * 本文件展示了现代C++特性与移动语义、完美转发的结合应用：
 * 1. 概念约束风格 (使用SFINAE模拟) 与完美转发
 * 2. 现代模板设计与移动语义
 * 3. 高级完美转发技术
 * 4. 模块化设计与性能优化
 * 5. 高级模板元编程
 */

// ============================================================================
// 1. 概念约束风格与完美转发 (使用SFINAE模拟C++20概念)
// ============================================================================

namespace ConceptsStyleForwarding {
    
    // 使用SFINAE模拟概念约束
    template<typename T>
    struct is_movable {
        static constexpr bool value = 
            std::is_move_constructible<T>::value &&
            std::is_move_assignable<T>::value &&
            !std::is_reference<T>::value;
    };
    
    template<typename T>
    struct is_copyable_or_movable {
        static constexpr bool value = 
            (std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value) ||
            is_movable<T>::value;
    };
    
    template<typename T, typename = void>
    struct has_move_constructor : std::false_type {};
    
    template<typename T>
    struct has_move_constructor<T, 
        typename std::enable_if<
            std::is_constructible<T, T&&>::value
        >::type
    > : std::true_type {};
    
    template<typename T, typename = void>
    struct is_streamable : std::false_type {};
    
    template<typename T>
    struct is_streamable<T, 
        typename std::enable_if<
            std::is_same<
                decltype(std::declval<std::ostream&>() << std::declval<const T&>()),
                std::ostream&
            >::value
        >::type
    > : std::true_type {};
    
    // 资源管理类，展示概念约束风格
    template<typename T>
    class ResourceManager {
        static_assert(is_movable<T>::value, "T must be movable");
        
    private:
        std::unique_ptr<T> resource;
        std::string name;
        
    public:
        // 完美转发构造函数，使用SFINAE约束
        template<typename... Args,
                 typename = typename std::enable_if<std::is_constructible<T, Args...>::value>::type>
        explicit ResourceManager(std::string n, Args&&... args) 
            : resource(std::make_unique<T>(std::forward<Args>(args)...))
            , name(std::move(n)) {
            std::cout << "ResourceManager '" << name << "' 创建资源\n";
        }
        
        // 移动构造函数
        ResourceManager(ResourceManager&& other) noexcept
            : resource(std::move(other.resource))
            , name(std::move(other.name)) {
            std::cout << "ResourceManager '" << name << "' 移动构造\n";
        }
        
        // 移动赋值操作符
        ResourceManager& operator=(ResourceManager&& other) noexcept {
            if (this != &other) {
                resource = std::move(other.resource);
                name = std::move(other.name);
                std::cout << "ResourceManager '" << name << "' 移动赋值\n";
            }
            return *this;
        }
        
        // 删除拷贝操作
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        
        // 访问资源
        T* get() const noexcept { return resource.get(); }
        T& operator*() const { return *resource; }
        T* operator->() const { return resource.get(); }
        
        const std::string& getName() const noexcept { return name; }
        
        ~ResourceManager() {
            if (resource) {
                std::cout << "ResourceManager '" << name << "' 析构\n";
            }
        }
    };
    
    // 工厂函数，使用SFINAE约束和完美转发
    template<typename T, typename... Args>
    typename std::enable_if<
        is_movable<T>::value && std::is_constructible<T, Args...>::value,
        ResourceManager<T>
    >::type make_resource_manager(std::string name, Args&&... args) {
        return ResourceManager<T>(std::move(name), std::forward<Args>(args)...);
    }
    
    // 条件转发函数，基于类型特征选择策略
    template<typename T>
    void conditional_forward(T&& value) {
        using DecayedT = typename std::decay<T>::type;
        /* std::decay 是一个类型转换模板（type transformation trait），它执行以下转换：
            移除引用：去掉 T 中的引用修饰（& 和 &&）
            移除 CV 限定符：去掉 const 和 volatile 限定
            数组到指针转换：将数组类型转换为对应指针类型
            函数到指针转换：将函数类型转换为函数指针类型 
        */
        if (is_movable<DecayedT>::value) {
            std::cout << "使用移动语义转发: " << typeid(DecayedT).name() << "\n";
        } else {
            std::cout << "类型不支持移动语义: " << typeid(DecayedT).name() << "\n";
        }
        (void)value;
        // 这里只是演示类型检测，不返回值以避免拷贝问题
    }
    
    // 测试用的资源类
    class ExpensiveResource {
    private:
        std::vector<int> data;
        std::string identifier;
        
    public:
        ExpensiveResource(std::string id, size_t size) 
            : data(size, 42), identifier(std::move(id)) {
            std::cout << "ExpensiveResource '" << identifier << "' 构造 (大小: " << size << ")\n";
        }
        
        ExpensiveResource(ExpensiveResource&& other) noexcept
            : data(std::move(other.data)), identifier(std::move(other.identifier)) {
            std::cout << "ExpensiveResource '" << identifier << "' 移动构造\n";
        }
        
        ExpensiveResource& operator=(ExpensiveResource&& other) noexcept {
            if (this != &other) {
                data = std::move(other.data);
                identifier = std::move(other.identifier);
                std::cout << "ExpensiveResource '" << identifier << "' 移动赋值\n";
            }
            return *this;
        }
        
        // 禁用拷贝
        ExpensiveResource(const ExpensiveResource&) = delete;
        ExpensiveResource& operator=(const ExpensiveResource&) = delete;
        
        const std::string& getId() const { return identifier; }
        size_t getSize() const { return data.size(); }
        
        ~ExpensiveResource() {
            std::cout << "ExpensiveResource '" << identifier << "' 析构\n";
        }
    };
    
    void demonstrate() {
        std::cout << "\n=== 概念约束风格与完美转发演示 ===\n";
        
        // 使用工厂函数创建资源管理器
        auto manager1 = make_resource_manager<ExpensiveResource>("资源1", "ID_001", 1000);
        auto manager2 = make_resource_manager<ExpensiveResource>("资源2", "ID_002", 2000);
        
        std::cout << "管理器1: " << manager1->getId() << " (大小: " << manager1->getSize() << ")\n";
        
        // 移动语义
        auto manager3 = std::move(manager1);
        std::cout << "移动后管理器3: " << manager3.getName() << "\n";
        
        // 条件转发演示
        std::string copyable_str = "可拷贝字符串";
        conditional_forward(copyable_str);
        conditional_forward(std::move(manager2));
    }
}

// ============================================================================
// 2. 现代异步设计与移动语义
// ============================================================================

namespace ModernAsyncDesign {
    
    // 移动语义友好的任务类
    class Task {
    private:
        std::function<void()> task_func;
        std::string task_name;
        int priority;
        
    public:
        template<typename Func>
        Task(std::string name, int prio, Func&& func)
            : task_func(std::forward<Func>(func))
            , task_name(std::move(name))
            , priority(prio) {
            std::cout << "Task '" << task_name << "' 创建 (优先级: " << priority << ")\n";
        }
        
        Task(Task&& other) noexcept
            : task_func(std::move(other.task_func))
            , task_name(std::move(other.task_name))
            , priority(other.priority) {
            std::cout << "Task '" << task_name << "' 移动构造\n";
        }
        
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                task_func = std::move(other.task_func);
                task_name = std::move(other.task_name);
                priority = other.priority;
                std::cout << "Task '" << task_name << "' 移动赋值\n";
            }
            return *this;
        }
        
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;
        
        void execute() const {
            std::cout << "执行任务: " << task_name << "\n";
            if (task_func) {
                task_func();
            }
        }
        
        const std::string& getName() const { return task_name; }
        int getPriority() const { return priority; }
        
        ~Task() {
            std::cout << "Task '" << task_name << "' 析构\n";
        }
    };
    
    // 任务调度器
    class TaskScheduler {
    private:
        std::vector<Task> tasks;
        std::string scheduler_name;
        
    public:
        explicit TaskScheduler(std::string name) : scheduler_name(std::move(name)) {
            std::cout << "TaskScheduler '" << scheduler_name << "' 创建\n";
        }
        
        TaskScheduler(TaskScheduler&& other) noexcept
            : tasks(std::move(other.tasks)), scheduler_name(std::move(other.scheduler_name)) {
            std::cout << "TaskScheduler '" << scheduler_name << "' 移动构造\n";
        }
        
        TaskScheduler& operator=(TaskScheduler&& other) noexcept {
            if (this != &other) {
                tasks = std::move(other.tasks);
                scheduler_name = std::move(other.scheduler_name);
                std::cout << "TaskScheduler '" << scheduler_name << "' 移动赋值\n";
            }
            return *this;
        }
        
        TaskScheduler(const TaskScheduler&) = delete;
        TaskScheduler& operator=(const TaskScheduler&) = delete;
        
        // 完美转发添加任务
        template<typename Func>
        void add_task(std::string name, int priority, Func&& func) {
            tasks.emplace_back(std::move(name), priority, std::forward<Func>(func));
        }
        
        void execute_all() {
            std::cout << "调度器 '" << scheduler_name << "' 开始执行所有任务\n";
            
            // 按优先级排序
            std::sort(tasks.begin(), tasks.end(), 
                [](const Task& a, const Task& b) {
                    return a.getPriority() > b.getPriority();
                });
            
            for (const auto& task : tasks) {
                task.execute();
            }
            
            std::cout << "调度器 '" << scheduler_name << "' 完成所有任务\n";
        }
        
        size_t getTaskCount() const { return tasks.size(); }
        
        ~TaskScheduler() {
            std::cout << "TaskScheduler '" << scheduler_name << "' 析构\n";
        }
    };
    
    void demonstrate() {
        std::cout << "\n=== 现代异步设计与移动语义演示 ===\n";
        
        // 创建任务调度器
        auto scheduler = TaskScheduler("主调度器");

        // 添加各种任务
        scheduler.add_task("数据处理", 3, []() {
            std::cout << "  -> 处理数据中...\n";
        });
        
        scheduler.add_task("网络请求", 5, []() {
            std::cout << "  -> n...\n";
        });
        
        scheduler.add_task("文件操作", 1, []() {
            std::cout << "  -> 执行文件操作...\n";
        });
        
        std::cout << "调度器任务数量: " << scheduler.getTaskCount() << "\n";
        
        // 执行所有任务
        scheduler.execute_all();
        
        // 移动调度器
        auto moved_scheduler = std::move(scheduler);
        std::cout << "移动后调度器任务数量: " << moved_scheduler.getTaskCount() << "\n";
    }
}

// ============================================================================
// 3. 高级完美转发技术
// ============================================================================

namespace AdvancedForwarding {
    
    // 通用包装器，支持完美转发
    template<typename Callable>
    class CallableWrapper {
    private:
        Callable callable;
        std::string wrapper_name;
        
    public:
        template<typename C>
        CallableWrapper(std::string name, C&& c)
            : callable(std::forward<C>(c)), wrapper_name(std::move(name)) {
            std::cout << "CallableWrapper '" << wrapper_name << "' 创建\n";
        }
        
        CallableWrapper(CallableWrapper&& other) noexcept
            : callable(std::move(other.callable)), wrapper_name(std::move(other.wrapper_name)) {
            std::cout << "CallableWrapper '" << wrapper_name << "' 移动构造\n";
        }
        
        CallableWrapper& operator=(CallableWrapper&& other) noexcept {
            if (this != &other) {
                callable = std::move(other.callable);
                wrapper_name = std::move(other.wrapper_name);
                std::cout << "CallableWrapper '" << wrapper_name << "' 移动赋值\n";
            }
            return *this;
        }
        
        CallableWrapper(const CallableWrapper&) = delete;
        CallableWrapper& operator=(const CallableWrapper&) = delete;
        
        // 完美转发调用操作符
        template<typename... Args>
        auto operator()(Args&&... args) 
            -> decltype(callable(std::forward<Args>(args)...)) {
            std::cout << "CallableWrapper '" << wrapper_name << "' 调用\n";
            return callable(std::forward<Args>(args)...);
        }
        
        const std::string& getName() const { return wrapper_name; }
        
        ~CallableWrapper() {
            std::cout << "CallableWrapper '" << wrapper_name << "' 析构\n";
        }
    };
    
    // 工厂函数
    template<typename Callable>
    auto make_wrapper(std::string name, Callable&& callable) 
        -> CallableWrapper<typename std::decay<Callable>::type> {
        return CallableWrapper<typename std::decay<Callable>::type>(
            std::move(name), std::forward<Callable>(callable)
        );
    }
    
    // 链式调用支持
    template<typename T>
    class ChainableProcessor {
    private:
        T value;
        std::string chain_name;
        
    public:
        template<typename U>
        ChainableProcessor(std::string name, U&& val)
            : value(std::forward<U>(val)), chain_name(std::move(name)) {
            std::cout << "ChainableProcessor '" << chain_name << "' 创建\n";
        }
        
        ChainableProcessor(ChainableProcessor&& other) noexcept
            : value(std::move(other.value)), chain_name(std::move(other.chain_name)) {
            std::cout << "ChainableProcessor '" << chain_name << "' 移动构造\n";
        }
        
        ChainableProcessor& operator=(ChainableProcessor&& other) noexcept {
            if (this != &other) {
                value = std::move(other.value);
                chain_name = std::move(other.chain_name);
                std::cout << "ChainableProcessor '" << chain_name << "' 移动赋值\n";
            }
            return *this;
        }
        
        ChainableProcessor(const ChainableProcessor&) = delete;
        ChainableProcessor& operator=(const ChainableProcessor&) = delete;
        
        // 链式处理方法
        template<typename Func>
        auto then(Func&& func) && 
            -> ChainableProcessor<decltype(func(std::move(value)))> {
            std::cout << "ChainableProcessor '" << chain_name << "' 链式调用\n";
            return ChainableProcessor<decltype(func(std::move(value)))>(
                chain_name + "->then", func(std::move(value))
            );
        }
        
        const T& get() const { return value; }
        T&& extract() && { return std::move(value); }
        
        ~ChainableProcessor() {
            std::cout << "ChainableProcessor '" << chain_name << "' 析构\n";
        }
    };
    
    // 创建链式处理器的工厂函数
    template<typename T>
    auto make_chainable(std::string name, T&& value) 
        -> ChainableProcessor<typename std::decay<T>::type> {
        return ChainableProcessor<typename std::decay<T>::type>(
            std::move(name), std::forward<T>(value)
        );
    }
    
    void demonstrate() {
        std::cout << "\n=== 高级完美转发技术演示 ===\n";
        
        // 创建可调用包装器
        auto wrapper1 = make_wrapper("乘法器", [](int x) { return x * 2; });
        auto wrapper2 = make_wrapper("加法器", [](int x) { return x + 10; });
        
        // 使用包装器
        int result1 = wrapper1(5);
        int result2 = wrapper2(result1);
        std::cout << "包装器处理结果: " << result2 << "\n";
        
        // 链式处理演示
        auto processor = make_chainable("数值处理", 42)
            .then([](int x) { 
                std::cout << "  -> 步骤1: " << x << " * 3 = " << (x * 3) << "\n";
                return x * 3; 
            })
            .then([](int x) { 
                std::cout << "  -> 步骤2: " << x << " + 100 = " << (x + 100) << "\n";
                return x + 100; 
            })
            .then([](int x) { 
                std::cout << "  -> 步骤3: " << x << " / 2 = " << (x / 2) << "\n";
                return x / 2; 
            });
        
        std::cout << "链式处理最终结果: " << processor.get() << "\n";
        
        // 移动提取结果
        auto final_result = std::move(processor).extract();
        std::cout << "提取的最终结果: " << final_result << "\n";
    }
}

// ============================================================================
// 4. 高级模板元编程与完美转发
// ============================================================================

namespace AdvancedTemplates {
    
    // 类型特征检测
    template<typename T, typename = void>
    struct has_to_string : std::false_type {};
    
    template<typename T>
    struct has_to_string<T, 
        typename std::enable_if<
            std::is_same<
                decltype(std::declval<const T&>().toString()),
                std::string
            >::value
        >::type
    > : std::true_type {};
    
    // 通用序列化器，使用完美转发和SFINAE约束
    template<typename T>
    class UniversalSerializer {
    private:
        T data;
        std::string type_name;
        
    public:
        template<typename U>
        explicit UniversalSerializer(U&& value,
            typename std::enable_if<
                std::is_same<typename std::decay<U>::type, T>::value
            >::type* = nullptr)
            : data(std::forward<U>(value))
            , type_name(typeid(T).name()) {
            std::cout << "UniversalSerializer<" << type_name << "> 创建\n";
        }
        
        UniversalSerializer(UniversalSerializer&& other) noexcept
            : data(std::move(other.data)), type_name(std::move(other.type_name)) {
            std::cout << "UniversalSerializer<" << type_name << "> 移动构造\n";
        }
        
        UniversalSerializer& operator=(UniversalSerializer&& other) noexcept {
            if (this != &other) {
                data = std::move(other.data);
                type_name = std::move(other.type_name);
                std::cout << "UniversalSerializer<" << type_name << "> 移动赋值\n";
            }
            return *this;
        }
        
        UniversalSerializer(const UniversalSerializer&) = delete;
        UniversalSerializer& operator=(const UniversalSerializer&) = delete;
        
        std::string serialize() const {
            return serialize_impl(typename has_to_string<T>::type{});
        }
        
    private:
        std::string serialize_impl(std::true_type) const {
            return type_name + ":" + data.toString();
        }
        
        std::string serialize_impl(std::false_type) const {
            return type_name + ":[不可序列化]";
        }
        
    public:
        const T& getData() const { return data; }
        
        ~UniversalSerializer() {
            std::cout << "UniversalSerializer<" << type_name << "> 析构\n";
        }
    };
    
    // 工厂函数，使用完美转发
    template<typename T, typename... Args>
    auto make_serializer(Args&&... args) 
        -> UniversalSerializer<T> {
        return UniversalSerializer<T>(T(std::forward<Args>(args)...));
    }
    
    // 测试类
    class SerializableObject {
    private:
        std::string name;
        int value;
        
    public:
        SerializableObject(std::string n, int v) 
            : name(std::move(n)), value(v) {
            std::cout << "SerializableObject '" << name << "' 创建\n";
        }
        
        SerializableObject(SerializableObject&& other) noexcept
            : name(std::move(other.name)), value(other.value) {
            std::cout << "SerializableObject '" << name << "' 移动构造\n";
        }
        
        SerializableObject& operator=(SerializableObject&& other) noexcept {
            if (this != &other) {
                name = std::move(other.name);
                value = other.value;
                std::cout << "SerializableObject '" << name << "' 移动赋值\n";
            }
            return *this;
        }
        
        SerializableObject(const SerializableObject&) = delete;
        SerializableObject& operator=(const SerializableObject&) = delete;
        
        std::string toString() const {
            return "{name:'" + name + "', value:" + std::to_string(value) + "}";
        }
        
        ~SerializableObject() {
            std::cout << "SerializableObject '" << name << "' 析构\n";
        }
    };
    
    void demonstrate() {
        std::cout << "\n=== 高级模板元编程与完美转发演示 ===\n";
        
        // 创建序列化器
        auto serializer1 = make_serializer<SerializableObject>("对象1", 42);
        std::cout << "序列化结果: " << serializer1.serialize() << "\n";
        
        // 移动序列化器
        auto serializer2 = std::move(serializer1);
        std::cout << "移动后序列化结果: " << serializer2.serialize() << "\n";
    }
}

// ============================================================================
// 5. 性能基准测试
// ============================================================================

namespace PerformanceBenchmark {
    
    class LargeMovableObject {
    private:
        std::vector<double> data;
        std::string metadata;
        std::unique_ptr<int[]> buffer;
        
    public:
        LargeMovableObject(size_t size, std::string meta)
            : data(size, 3.14159), metadata(std::move(meta))
            , buffer(std::make_unique<int[]>(size)) {
            std::iota(buffer.get(), buffer.get() + size, 0);
        }
        
        // 移动构造函数
        LargeMovableObject(LargeMovableObject&& other) noexcept
            : data(std::move(other.data))
            , metadata(std::move(other.metadata))
            , buffer(std::move(other.buffer)) {}
        
        // 移动赋值操作符
        LargeMovableObject& operator=(LargeMovableObject&& other) noexcept {
            if (this != &other) {
                data = std::move(other.data);
                metadata = std::move(other.metadata);
                buffer = std::move(other.buffer);
            }
            return *this;
        }
        
        // 禁用拷贝
        LargeMovableObject(const LargeMovableObject&) = delete;
        LargeMovableObject& operator=(const LargeMovableObject&) = delete;
        
        size_t getSize() const { return data.size(); }
        const std::string& getMetadata() const { return metadata; }
    };
    
    template<typename Func>
    auto benchmark_operation(const std::string& operation_name, Func&& func, int iterations = 1000) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            func();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << operation_name << ": " << duration.count() << " 微秒 (" 
                  << iterations << " 次迭代)\n";
        return duration;
    }
    
    void demonstrate() {
        std::cout << "\n=== 性能基准测试 ===\n";
        
        constexpr size_t object_size = 10000;
        constexpr int iterations = 100;
        
        // 测试移动语义性能
        benchmark_operation("大对象移动构造", [&]() {
            auto obj1 = LargeMovableObject(object_size, "测试对象");
            auto obj2 = std::move(obj1);  // 移动构造
            (void)obj2;  // 避免未使用警告
        }, iterations);
        
        // 测试完美转发性能
        benchmark_operation("完美转发工厂", [&]() {
            auto manager = ConceptsStyleForwarding::make_resource_manager<ConceptsStyleForwarding::ExpensiveResource>(
                "性能测试", "PERF_ID", object_size / 10
            );
            (void)manager;
        }, iterations);
        
        // 测试链式处理性能
        benchmark_operation("链式处理", [&]() {
            auto result = AdvancedForwarding::make_chainable("性能测试", 100)
                .then([](int x) { return x * 2; })
                .then([](int x) { return x + 50; })
                .then([](int x) { return x / 3; });
            (void)result.get();
        }, iterations);
        
        // 测试模板元编程性能
        benchmark_operation("模板序列化", [&]() {
            auto serializer = AdvancedTemplates::make_serializer<AdvancedTemplates::SerializableObject>(
                "性能测试对象", 12345
            );
            auto result = serializer.serialize();
            (void)result;
        }, iterations);
    }
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "=== C++20 风格的移动语义与完美转发示例 ===\n";
    
    try {
        ConceptsStyleForwarding::demonstrate();
        ModernAsyncDesign::demonstrate();
        AdvancedForwarding::demonstrate();
        AdvancedTemplates::demonstrate();
        PerformanceBenchmark::demonstrate();
        
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\n=== 所有演示完成 ===\n";
    return 0;
}