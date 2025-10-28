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

// 不使用std::decay的版本（会有问题）
template<typename F, typename G>
class ComposerBad {
private:
    F f;  // 这里F可能是引用类型！
    G g;  // 这里G可能是引用类型！
    
public:
    template<typename F2, typename G2>
    ComposerBad(F2&& f_func, G2&& g_func) 
        : f(std::forward<F2>(f_func)), g(std::forward<G2>(g_func)) {}
        
    template<typename... Args>
    auto operator()(Args&&... args) const 
        -> decltype(f(g(std::forward<Args>(args)...))) {
        return f(g(std::forward<Args>(args)...));
    }
};

// 使用std::decay的版本（正确）
template<typename F, typename G>
class ComposerGood {
private:
    F f;  // 这里F是去除引用和cv限定符的纯类型
    G g;  // 这里G是去除引用和cv限定符的纯类型
    
public:
    template<typename F2, typename G2>
    ComposerGood(F2&& f_func, G2&& g_func) 
        : f(std::forward<F2>(f_func)), g(std::forward<G2>(g_func)) {}
        
    template<typename... Args>
    auto operator()(Args&&... args) const 
        -> decltype(f(g(std::forward<Args>(args)...))) {
        return f(g(std::forward<Args>(args)...));
    }
};

// 不使用std::decay的compose函数
template<typename F, typename G>
auto compose_bad(F&& f, G&& g) {
    std::cout << "=== compose_bad 类型推导 ===\n";
    std::cout << "F 类型: " << getTypeName<F>() << "\n";
    std::cout << "G 类型: " << getTypeName<G>() << "\n";
    
    return ComposerBad<F, G>(std::forward<F>(f), std::forward<G>(g));
}

// 使用std::decay的compose函数
template<typename F, typename G>
auto compose_good(F&& f, G&& g) {
    std::cout << "=== compose_good 类型推导 ===\n";
    std::cout << "F 类型: " << getTypeName<F>() << "\n";
    std::cout << "G 类型: " << getTypeName<G>() << "\n";
    std::cout << "decay<F>::type: " << getTypeName<typename std::decay<F>::type>() << "\n";
    std::cout << "decay<G>::type: " << getTypeName<typename std::decay<G>::type>() << "\n";
    
    return ComposerGood<typename std::decay<F>::type, typename std::decay<G>::type>(
        std::forward<F>(f), std::forward<G>(g));
}

// 测试函数
int add_one(int x) { return x + 1; }
int multiply_two(int x) { return x * 2; }

int main() {
    std::cout << "=== std::decay 必要性演示 ===\n\n";
    
    // 测试函数指针
    auto func1 = add_one;
    auto func2 = multiply_two;
    
    std::cout << "测试1: 左值函数对象\n";
    auto bad_composer = compose_bad(func1, func2);
    std::cout << "\n";
    
    auto good_composer = compose_good(func1, func2);
    std::cout << "\n";
    
    // 测试lambda表达式
    std::cout << "测试2: lambda表达式\n";
    auto lambda1 = [](int x) { return x + 10; };
    auto lambda2 = [](int x) { return x * 3; };
    
    auto bad_lambda_composer = compose_bad(lambda1, lambda2);
    std::cout << "\n";
    
    auto good_lambda_composer = compose_good(lambda1, lambda2);
    std::cout << "\n";
    
    // 测试右值lambda
    std::cout << "测试3: 右值lambda\n";
    auto bad_rvalue_composer = compose_bad(
        [](int x) { return x + 5; }, 
        [](int x) { return x * 4; }
    );
    std::cout << "\n";
    
    auto good_rvalue_composer = compose_good(
        [](int x) { return x + 5; }, 
        [](int x) { return x * 4; }
    );
    std::cout << "\n";
    
    // 测试执行
    std::cout << "=== 执行测试 ===\n";
    std::cout << "good_composer(5): " << good_composer(5) << "\n";  // (5 * 2) + 1 = 11
    std::cout << "good_lambda_composer(5): " << good_lambda_composer(5) << "\n";  // (5 * 3) + 10 = 25
    std::cout << "good_rvalue_composer(5): " << good_rvalue_composer(5) << "\n";  // (5 * 4) + 5 = 25
    
    return 0;
}