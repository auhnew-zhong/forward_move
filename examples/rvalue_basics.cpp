#include <iostream>
#include <string>
#include <vector>

/**
 * 右值引用基础示例
 * 
 * 本文件演示了右值引用的基本概念和用法：
 * 1. 左值 vs 右值的区别
 * 2. 右值引用的语法 (&&)
 * 3. 右值引用的绑定规则
 * 4. std::move的使用
 */

// 用于演示的简单类
class SimpleClass {
private:
    std::string data;
    
public:
    // 默认构造函数
    SimpleClass() : data("default") {
        std::cout << "SimpleClass 默认构造函数被调用\n";
    }
    
    // 带参数的构造函数
    SimpleClass(const std::string& str) : data(str) {
        std::cout << "SimpleClass 构造函数被调用，参数: " << str << "\n";
    }
    
    // 拷贝构造函数
    SimpleClass(const SimpleClass& other) : data(other.data) {
        std::cout << "SimpleClass 拷贝构造函数被调用，数据: " << data << "\n";
    }
    
    // 移动构造函数
    SimpleClass(SimpleClass&& other) noexcept : data(std::move(other.data)) {
        std::cout << "SimpleClass 移动构造函数被调用，数据: " << data << "\n";
    }
    
    // 拷贝赋值运算符
    SimpleClass& operator=(const SimpleClass& other) {
        if (this != &other) {
            data = other.data;
            std::cout << "SimpleClass 拷贝赋值运算符被调用，数据: " << data << "\n";
        }
        return *this;
    }
    
    // 移动赋值运算符
    SimpleClass& operator=(SimpleClass&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            std::cout << "SimpleClass 移动赋值运算符被调用，数据: " << data << "\n";
        }
        return *this;
    }
    
    // 析构函数
    ~SimpleClass() {
        std::cout << "SimpleClass 析构函数被调用，数据: " << data << "\n";
    }
    
    // 获取数据
    const std::string& getData() const { return data; }
    
    // 设置数据
    void setData(const std::string& newData) { data = newData; }
};

/**
 * 演示左值和右值的区别
 */
void demonstrateLValueVsRValue() {
    std::cout << "\n=== 左值 vs 右值演示 ===\n";
    
    // 左值：有名字的变量，可以取地址
    int x = 10;          // x是左值
    int y = 20;          // y是左值
    
    std::cout << "x的地址: " << &x << ", 值: " << x << "\n";
    std::cout << "y的地址: " << &y << ", 值: " << y << "\n";
    
    // 右值：临时对象，字面量，表达式结果等
    // 以下都是右值：
    // - 字面量: 42, 3.14, "hello"
    // - 临时对象: SimpleClass("temp")
    // - 表达式结果: x + y
    
    std::cout << "x + y的结果（右值）: " << x + y << "\n";
    
    // 左值引用只能绑定到左值
    int& lref = x;       // 正确：左值引用绑定到左值
    (void)lref;          // 避免未使用变量警告
    
    // int& lref2 = x + y;  // 错误：左值引用不能绑定到右值
    
    // 常量左值引用可以绑定到右值（生命周期延长）
    const int& clref = x + y;  // 正确：常量左值引用可以绑定到右值
    std::cout << "常量左值引用绑定的右值: " << clref << "\n";
}

/**
 * 演示右值引用的基本用法
 */
void demonstrateRValueReference() {
    std::cout << "\n=== 右值引用基本用法 ===\n";
    
    // 右值引用使用 && 语法
    int&& rref = 42;     // 右值引用绑定到字面量
    std::cout << "右值引用绑定的值: " << rref << "\n";
    
    // 右值引用绑定后，可以像左值一样使用
    rref = 100;
    std::cout << "修改后的右值引用: " << rref << "\n";
    
    // 右值引用可以绑定到临时对象
    SimpleClass&& obj_rref = SimpleClass("临时对象");
    std::cout << "右值引用绑定的对象数据: " << obj_rref.getData() << "\n";
    
    // 修改通过右值引用绑定的对象
    obj_rref.setData("修改后的数据");
    std::cout << "修改后的对象数据: " << obj_rref.getData() << "\n";
}

/**
 * 演示std::move的使用
 */
void demonstrateStdMove() {
    std::cout << "\n=== std::move 使用演示 ===\n";
    
    SimpleClass obj1("原始对象");
    std::cout << "obj1数据: " << obj1.getData() << "\n";
    
    // std::move将左值转换为右值引用
    // 注意：std::move本身不移动任何东西，只是类型转换
    SimpleClass obj2 = std::move(obj1);
    
    std::cout << "移动后 obj1数据: " << obj1.getData() << " (应该为空)\n";
    std::cout << "obj2数据: " << obj2.getData() << "\n";
    
    // 使用std::move进行赋值
    SimpleClass obj3;
    obj3 = std::move(obj2);
    
    std::cout << "赋值后 obj2数据: " << obj2.getData() << " (应该为空)\n";
    std::cout << "obj3数据: " << obj3.getData() << "\n";
}

/**
 * 演示右值引用在函数参数中的使用
 */
void processLValue(SimpleClass& obj) {
    std::cout << "处理左值引用: " << obj.getData() << "\n";
}

void processRValue(SimpleClass&& obj) {
    std::cout << "处理右值引用: " << obj.getData() << "\n";
    // 在函数内部，右值引用参数本身是左值
    // 如果要继续传递为右值，需要使用std::move
}

void processUniversal(const SimpleClass& obj) {
    std::cout << "处理常量引用（万能接收器）: " << obj.getData() << "\n";
}

void demonstrateFunctionOverloading() {
    std::cout << "\n=== 函数重载演示 ===\n";
    
    SimpleClass obj("左值对象");
    
    // 调用左值引用版本
    processLValue(obj);
    
    // 调用右值引用版本
    processRValue(SimpleClass("临时对象"));
    processRValue(std::move(obj));
    
    // 常量引用可以接收任何类型
    SimpleClass obj2("另一个对象");
    processUniversal(obj2);                    // 左值
    processUniversal(SimpleClass("临时"));      // 右值
}

/**
 * 演示右值引用的实际应用场景
 */
void demonstratePracticalUsage() {
    std::cout << "\n=== 实际应用场景演示 ===\n";
    
    // 场景1：容器操作优化
    std::vector<SimpleClass> vec;
    
    std::cout << "--- 向容器添加元素 ---\n";
    
    // 使用拷贝（效率较低）
    SimpleClass obj1("拷贝对象");
    vec.push_back(obj1);  // 调用拷贝构造函数
    
    // 使用移动（效率更高）
    vec.push_back(SimpleClass("临时对象"));  // 调用移动构造函数
    vec.push_back(std::move(obj1));          // 调用移动构造函数
    
    // 场景2：返回值优化
    std::cout << "\n--- 函数返回值优化 ---\n";
    
    auto createObject = []() -> SimpleClass {
        return SimpleClass("函数返回的对象");  // 返回值优化(RVO)
    };
    
    SimpleClass result = createObject();  // 可能触发移动构造或RVO
    std::cout << "结果对象数据: " << result.getData() << "\n";
}

int main() {
    std::cout << "C++ 右值引用基础示例\n";
    std::cout << "====================\n";
    
    try {
        demonstrateLValueVsRValue();
        demonstrateRValueReference();
        demonstrateStdMove();
        demonstrateFunctionOverloading();
        demonstratePracticalUsage();
        
        std::cout << "\n=== 程序执行完成 ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
 * 关键概念总结：
 * 
 * 1. 左值 (lvalue)：
 *    - 有名字的变量
 *    - 可以取地址
 *    - 生命周期较长
 * 
 * 2. 右值 (rvalue)：
 *    - 临时对象、字面量、表达式结果
 *    - 不能取地址（通常）
 *    - 生命周期较短
 * 
 * 3. 右值引用 (&&)：
 *    - 只能绑定到右值
 *    - 延长临时对象的生命周期
 *    - 支持移动语义
 * 
 * 4. std::move：
 *    - 将左值转换为右值引用
 *    - 本身不移动任何东西
 *    - 启用移动语义
 * 
 * 5. 应用场景：
 *    - 避免不必要的拷贝
 *    - 实现移动构造函数和移动赋值运算符
 *    - 优化容器操作
 *    - 完美转发（配合模板使用）
 */