#include <iostream>
#include <type_traits>
#include <functional>
#include <typeinfo>
#include <cxxabi.h>

// 辅助函数：获取类型名称
template<typename T>
std::string getTypeName() {
    int status;
    char* demangled = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string result = demangled ? demangled : typeid(T).name();
    free(demangled);
    return result;
}

// 演示std::decay的作用
template<typename T>
void show_decay_effect() {
    std::cout << "原始类型 T: " << getTypeName<T>() << "\n";
    std::cout << "std::decay<T>::type: " << getTypeName<typename std::decay<T>::type>() << "\n";
    std::cout << "是否为引用: " << std::is_reference<T>::value << "\n";
    std::cout << "是否为const: " << std::is_const<T>::value << "\n";
    std::cout << "是否为数组: " << std::is_array<T>::value << "\n";
    std::cout << "是否为函数: " << std::is_function<T>::value << "\n";
    std::cout << "decay后是否为引用: " << std::is_reference<typename std::decay<T>::type>::value << "\n";
    std::cout << "decay后是否为const: " << std::is_const<typename std::decay<T>::type>::value << "\n";
    std::cout << "decay后是否为数组: " << std::is_array<typename std::decay<T>::type>::value << "\n";
    std::cout << "decay后是否为函数: " << std::is_function<typename std::decay<T>::type>::value << "\n";
    std::cout << "---\n";
}

// 问题演示：不使用decay的Composer
template<typename F, typename G>
class BadComposer {
private:
    F f;  // 如果F是引用类型，这里会有问题！
    G g;
    
public:
    BadComposer(F f_func, G g_func) : f(f_func), g(g_func) {
        std::cout << "BadComposer构造，F类型: " << getTypeName<F>() << "\n";
        std::cout << "BadComposer构造，G类型: " << getTypeName<G>() << "\n";
    }
};

// 正确版本：使用decay的Composer
template<typename F, typename G>
class GoodComposer {
private:
    typename std::decay<F>::type f;  // 去除引用和cv限定符
    typename std::decay<G>::type g;
    
public:
    template<typename F2, typename G2>
    GoodComposer(F2&& f_func, G2&& g_func) 
        : f(std::forward<F2>(f_func)), g(std::forward<G2>(g_func)) {
        std::cout << "GoodComposer构造，存储的F类型: " << getTypeName<typename std::decay<F>::type>() << "\n";
        std::cout << "GoodComposer构造，存储的G类型: " << getTypeName<typename std::decay<G>::type>() << "\n";
    }
};

// 模拟compose函数的模板参数推导
template<typename F, typename G>
void test_template_deduction(F&& f, G&& g) {
    std::cout << "=== 模板参数推导结果 ===\n";
    std::cout << "F推导类型: " << getTypeName<F>() << "\n";
    std::cout << "G推导类型: " << getTypeName<G>() << "\n";
    
    // 如果直接使用F和G作为类模板参数会怎样？
    std::cout << "\n如果不使用decay:\n";
    show_decay_effect<F>();
    show_decay_effect<G>();
    
    std::cout << "这就是为什么需要std::decay的原因！\n\n";
}

int add_one(int x) { return x + 1; }

int main() {
    std::cout << "=== std::decay 必要性详细演示 ===\n\n";
    
    // 测试各种类型的decay效果
    std::cout << "1. 基本类型:\n";
    show_decay_effect<int>();
    
    std::cout << "2. 引用类型:\n";
    show_decay_effect<int&>();
    
    std::cout << "3. const引用类型:\n";
    show_decay_effect<const int&>();
    
    std::cout << "4. 右值引用类型:\n";
    show_decay_effect<int&&>();
    
    std::cout << "5. 数组类型:\n";
    show_decay_effect<int[5]>();
    
    std::cout << "6. 函数类型:\n";
    show_decay_effect<int(int)>();
    
    std::cout << "7. 函数指针类型:\n";
    show_decay_effect<int(*)(int)>();
    
    // 演示在compose函数中的问题
    std::cout << "\n=== 在compose函数中的应用 ===\n";
    
    // 左值函数
    auto func = add_one;
    test_template_deduction(func, func);
    
    // lambda表达式
    auto lambda = [](int x) { return x * 2; };
    test_template_deduction(lambda, lambda);
    
    std::cout << "=== 关键问题说明 ===\n";
    std::cout << "当传入左值时，模板参数F会被推导为引用类型（如int&）\n";
    std::cout << "但是类的成员变量不能是引用类型（引用必须在构造时初始化且不能重新绑定）\n";
    std::cout << "std::decay<F>::type 会将 int& 转换为 int，解决这个问题\n";
    std::cout << "同时也会将数组转换为指针，函数转换为函数指针\n";
    
    return 0;
}