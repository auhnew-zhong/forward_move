#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <type_traits>
#include <chrono>

/**
 * 完美转发 (Perfect Forwarding) 详细示例
 * 
 * 本文件深入演示完美转发的概念和实现：
 * 1. 万能引用 (Universal Reference) 的概念
 * 2. std::forward 的使用
 * 3. 模板参数推导规则
 * 4. 完美转发的实际应用
 * 5. 工厂函数和包装器的实现
 */

/**
 * 用于演示的测试类
 */
class TestObject {
private:
    std::string data;
    int id;
    
public:
    // 默认构造函数
    TestObject() : data("default"), id(0) {
        std::cout << "TestObject 默认构造 (id: " << id << ")\n";
    }
    
    // 带参数构造函数
    TestObject(const std::string& str, int i) : data(str), id(i) {
        std::cout << "TestObject 构造 (data: " << data << ", id: " << id << ")\n";
    }
    
    // 拷贝构造函数
    TestObject(const TestObject& other) : data(other.data), id(other.id) {
        std::cout << "TestObject 拷贝构造 (data: " << data << ", id: " << id << ")\n";
    }
    
    // 移动构造函数
    TestObject(TestObject&& other) noexcept 
        : data(std::move(other.data)), id(other.id) {
        other.id = -1;  // 标记为已移动
        std::cout << "TestObject 移动构造 (data: " << data << ", id: " << id << ")\n";
    }
    
    // 拷贝赋值运算符
    TestObject& operator=(const TestObject& other) {
        if (this != &other) {
            data = other.data;
            id = other.id;
            std::cout << "TestObject 拷贝赋值 (data: " << data << ", id: " << id << ")\n";
        }
        return *this;
    }
    
    // 移动赋值运算符
    TestObject& operator=(TestObject&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            id = other.id;
            other.id = -1;
            std::cout << "TestObject 移动赋值 (data: " << data << ", id: " << id << ")\n";
        }
        return *this;
    }
    
    // 析构函数
    ~TestObject() {
        std::cout << "TestObject 析构 (data: " << data << ", id: " << id << ")\n";
    }
    
    // 获取信息
    const std::string& getData() const { return data; }
    int getId() const { return id; }
    
    // 打印信息
    void print() const {
        std::cout << "TestObject{data: '" << data << "', id: " << id << "}\n";
    }
};

/**
 * 演示万能引用 (Universal Reference)
 * T&& 在模板中是万能引用，可以绑定到左值和右值
 */
template<typename T>
void demonstrateUniversalReference(T&& param) {
    std::cout << "\n--- 万能引用演示 ---\n";
    
    // 使用 type_traits 检查类型
    if (std::is_lvalue_reference<T>::value) {
        std::cout << "参数是左值引用类型\n";
    } else if (std::is_rvalue_reference<T>::value) {
        std::cout << "参数是右值引用类型\n";
    } else {
        std::cout << "参数是值类型\n";
    }
    
    // 检查 param 的值类别
    std::cout << "param 的类型: " << typeid(T).name() << "\n";
    std::cout << "param 是左值: " << std::is_lvalue_reference<decltype((param))>::value << "\n";
}

/**
 * 不完美的转发示例 - 展示问题
 */
template<typename T>
void imperfectForwarding(T&& param) {
    std::cout << "\n--- 不完美转发 ---\n";
    
    // 直接传递 param - 总是作为左值传递
    TestObject obj1(param);  // 总是调用拷贝构造函数
    
    // 使用 std::move - 总是作为右值传递
    TestObject obj2(std::move(param));  // 总是调用移动构造函数
}

/**
 * 完美转发示例 - 使用 std::forward
 */
template<typename T>
void perfectForwarding(T&& param) {
    std::cout << "\n--- 完美转发 ---\n";
    
    // 使用 std::forward 保持参数的值类别
    TestObject obj(std::forward<T>(param));
    
    std::cout << "转发完成\n";
}

/**
 * 演示引用折叠规则
 * T& + & = T&
 * T& + && = T&
 * T&& + & = T&
 * T&& + && = T&&
 */
void demonstrateReferenceCollapsing() {
    std::cout << "\n=== 引用折叠规则演示 ===\n";
    
    TestObject obj("左值对象", 1);
    
    std::cout << "\n--- 传递左值 ---\n";
    // T 被推导为 TestObject&
    // T&& 变成 TestObject& && = TestObject& (引用折叠)
    demonstrateUniversalReference(obj);
    
    std::cout << "\n--- 传递右值 ---\n";
    // T 被推导为 TestObject
    // T&& 变成 TestObject&&
    demonstrateUniversalReference(TestObject("右值对象", 2));
}

/**
 * 工厂函数示例 - 完美转发的经典应用
 */
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_perfect(Args&&... args) {
    std::cout << "\n--- 完美转发工厂函数 ---\n";
    std::cout << "创建对象，参数个数: " << sizeof...(args) << "\n";
    
    // 完美转发所有参数给构造函数
    return std::make_unique<T>(std::forward<Args>(args)...);
}

/**
 * 包装器函数示例
 */
template<typename Func, typename... Args>
auto wrapper_function(Func&& func, Args&&... args) 
    -> decltype(std::forward<Func>(func)(std::forward<Args>(args)...)) {
    
    std::cout << "\n--- 函数包装器 ---\n";
    std::cout << "调用被包装的函数\n";
    
    // 完美转发函数和所有参数
    return std::forward<Func>(func)(std::forward<Args>(args)...);
}

/**
 * 演示变参模板的完美转发
 */
template<typename... Args>
void variadic_perfect_forwarding(Args&&... args) {
    std::cout << "\n--- 变参模板完美转发 ---\n";
    std::cout << "参数个数: " << sizeof...(args) << "\n";
    
    // 创建一个 tuple 来存储所有参数
    auto tuple = std::make_tuple(std::forward<Args>(args)...);
    
    std::cout << "所有参数已完美转发到 tuple\n";
}

/**
 * 条件转发 - 根据类型特性进行不同处理
 */
template<typename T>
void conditional_forwarding(T&& param) {
    std::cout << "\n--- 条件转发 ---\n";
    
    if (std::is_move_constructible<typename std::decay<T>::type>::value) {
        std::cout << "类型支持移动构造，使用完美转发\n";
        TestObject obj(std::forward<T>(param));
    } else {
        std::cout << "类型不支持移动构造，使用拷贝\n";
        TestObject obj(param);
    }
}

/**
 * 自定义容器的 emplace 实现
 */
template<typename T>
class SimpleContainer {
private:
    std::vector<T> data;
    
public:
    // 完美转发的 emplace_back 实现
    template<typename... Args>
    void emplace_back(Args&&... args) {
        std::cout << "\n--- 容器 emplace_back ---\n";
        std::cout << "就地构造对象\n";
        
        // 完美转发参数给构造函数
        data.emplace_back(std::forward<Args>(args)...);
    }
    
    // 获取元素
    const T& operator[](size_t index) const {
        return data[index];
    }
    
    size_t size() const { return data.size(); }
};

/**
 * 演示完美转发在实际应用中的使用
 */
void demonstratePracticalUsage() {
    std::cout << "\n=== 完美转发实际应用 ===\n";
    
    // 1. 工厂函数
    std::cout << "\n--- 使用工厂函数 ---\n";
    
    // 传递左值
    std::string str = "工厂创建";
    int id = 100;
    auto ptr1 = make_unique_perfect<TestObject>(str, id);
    
    // 传递右值
    auto ptr2 = make_unique_perfect<TestObject>("临时字符串", 200);
    
    // 2. 函数包装器
    std::cout << "\n--- 使用函数包装器 ---\n";
    
    auto lambda = [](const TestObject& obj, int multiplier) {
        std::cout << "Lambda 调用: " << obj.getData() 
                  << " * " << multiplier << " = " << obj.getId() * multiplier << "\n";
        return obj.getId() * multiplier;
    };
    
    TestObject testObj("包装器测试", 5);
    int result = wrapper_function(lambda, testObj, 3);
    std::cout << "包装器返回值: " << result << "\n";
    
    // 3. 容器的 emplace 操作
    std::cout << "\n--- 使用容器 emplace ---\n";
    
    SimpleContainer<TestObject> container;
    
    // 就地构造，避免临时对象
    container.emplace_back("容器对象1", 301);
    container.emplace_back(std::string("容器对象2"), 302);
    
    // 移动已存在的对象
    TestObject existing("已存在对象", 303);
    container.emplace_back(std::move(existing));
    
    std::cout << "容器大小: " << container.size() << "\n";
}

/**
 * 完美转发的性能测试
 */
void performanceTest() {
    std::cout << "\n=== 完美转发性能测试 ===\n";
    
    const int iterations = 1000;
    
    // 测试不完美转发
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        TestObject obj("性能测试", i);
        imperfectForwarding(std::move(obj));
    }
    
    auto imperfectTime = std::chrono::high_resolution_clock::now() - start;
    
    // 测试完美转发
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        TestObject obj("性能测试", i);
        perfectForwarding(std::move(obj));
    }
    
    auto perfectTime = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "不完美转发耗时: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(imperfectTime).count() 
              << " μs\n";
    std::cout << "完美转发耗时: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(perfectTime).count() 
              << " μs\n";
}

/**
 * 完美转发的陷阱和注意事项
 */
void demonstrateForwardingPitfalls() {
    std::cout << "\n=== 完美转发陷阱 ===\n";
    
    // 陷阱1：多次转发同一个参数
    std::cout << "\n--- 陷阱1: 多次转发 ---\n";
    
    auto multipleForward = [](auto&& param) {
        std::cout << "第一次转发:\n";
        TestObject obj1(std::forward<decltype(param)>(param));
        
        // 错误：param 已经被转发，不应该再次使用
        // std::cout << "第二次转发:\n";
        // TestObject obj2(std::forward<decltype(param)>(param));
        
        std::cout << "注意：参数只能转发一次！\n";
    };
    
    TestObject temp("多次转发测试", 400);
    multipleForward(std::move(temp));
    
    // 陷阱2：转发数组
    std::cout << "\n--- 陷阱2: 转发数组 ---\n";
    
    auto forwardArray = [](auto&& arr) {
        using ArrayType = typename std::decay<decltype(arr)>::type;
        std::cout << "数组类型: " << typeid(ArrayType).name() << "\n";
        
        // 数组会退化为指针
        std::cout << "是否为指针: " << std::is_pointer<ArrayType>::value << "\n";
    };
    
    int array[] = {1, 2, 3, 4, 5};
    forwardArray(array);
    
    // 陷阱3：转发重载函数
    std::cout << "\n--- 陷阱3: 转发重载函数 ---\n";
    
    auto overloadedFunc = [](int x) { return x * 2; };
    auto overloadedFunc2 = [](double x) { return x * 2.0; };
    
    // 需要明确指定类型
    auto callFunc = [](auto func, auto&& arg) {
        return func(std::forward<decltype(arg)>(arg));
    };
    
    int intArg = 5;
    double doubleArg = 3.14;
    
    std::cout << "整数结果: " << callFunc(overloadedFunc, intArg) << "\n";
    std::cout << "浮点结果: " << callFunc(overloadedFunc2, doubleArg) << "\n";
}

int main() {
    std::cout << "C++ 完美转发详细示例\n";
    std::cout << "=====================\n";
    
    try {
        demonstrateReferenceCollapsing();
        
        // 演示不完美转发 vs 完美转发
        TestObject obj1("测试对象1", 10);
        imperfectForwarding(obj1);
        
        TestObject obj2("测试对象2", 20);
        perfectForwarding(std::move(obj2));
        
        // 变参模板演示
        variadic_perfect_forwarding(1, "hello", 3.14, TestObject("变参", 30));
        
        // 条件转发演示
        TestObject obj3("条件转发", 40);
        conditional_forwarding(std::move(obj3));
        
        // 实际应用演示
        demonstratePracticalUsage();
        
        // 陷阱演示
        demonstrateForwardingPitfalls();
        
        // 性能测试（可选）
        std::cout << "\n进行性能测试...\n";
        performanceTest();
        
        std::cout << "\n=== 程序执行完成 ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
 * 完美转发关键概念总结：
 * 
 * 1. 万能引用 (Universal Reference)：
 *    - 模板中的 T&& 可以绑定左值和右值
 *    - 通过引用折叠规则确定最终类型
 *    - 只在类型推导时才是万能引用
 * 
 * 2. std::forward：
 *    - 条件性转换为右值引用
 *    - 保持参数的原始值类别
 *    - 必须提供模板参数类型
 * 
 * 3. 引用折叠规则：
 *    - & + & = &
 *    - & + && = &
 *    - && + & = &
 *    - && + && = &&
 * 
 * 4. 应用场景：
 *    - 工厂函数 (make_unique, make_shared)
 *    - 容器的 emplace 操作
 *    - 函数包装器
 *    - 变参模板函数
 * 
 * 5. 最佳实践：
 *    - 参数只能转发一次
 *    - 使用 std::decay_t 处理类型
 *    - 注意数组和函数的特殊处理
 *    - 结合 SFINAE 进行条件转发
 * 
 * 6. 性能优势：
 *    - 避免不必要的拷贝
 *    - 保持移动语义
 *    - 零开销抽象
 */