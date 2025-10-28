#include <iostream>
#include <type_traits>
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

// 模拟LRUCache的put函数
template<typename K, typename V>
void put(K&& key, V&& value) {
    std::cout << "=== 模板参数推导结果 ===\n";
    std::cout << "K 类型: " << getTypeName<K>() << "\n";
    std::cout << "V 类型: " << getTypeName<V>() << "\n";
    std::cout << "key 参数类型: " << getTypeName<decltype(key)>() << "\n";
    std::cout << "value 参数类型: " << getTypeName<decltype(value)>() << "\n";
    
    // 检查是否为左值引用
    std::cout << "K 是左值引用: " << std::is_lvalue_reference<K>::value << "\n";
    std::cout << "K 是右值引用: " << std::is_rvalue_reference<K>::value << "\n";
    std::cout << "V 是左值引用: " << std::is_lvalue_reference<V>::value << "\n";
    std::cout << "V 是右值引用: " << std::is_rvalue_reference<V>::value << "\n";
    std::cout << "\n";
}

class TestObject {
public:
    TestObject(const std::string& name) : name_(name) {
        std::cout << "TestObject 构造: " << name_ << "\n";
    }
    
    TestObject(TestObject&& other) noexcept : name_(std::move(other.name_)) {
        std::cout << "TestObject 移动: " << name_ << "\n";
    }
    
    ~TestObject() {
        std::cout << "TestObject 析构: " << name_ << "\n";
    }
    
private:
    std::string name_;
};

int main() {
    std::cout << "=== 模板参数推导测试 ===\n\n";
    
    // 测试1: 字符串字面值
    std::cout << "测试1: 字符串字面值\n";
    put("obj1", TestObject("测试对象1"));
    
    // 测试2: 左值变量
    std::cout << "测试2: 左值变量\n";
    std::string key = "obj2";
    TestObject obj("测试对象2");
    put(key, obj);
    
    // 测试3: 显式右值
    std::cout << "测试3: 显式右值\n";
    put(std::move(key), std::move(obj));
    
    return 0;
}