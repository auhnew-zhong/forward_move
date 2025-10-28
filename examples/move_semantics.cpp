#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>

/**
 * 移动语义详细示例
 * 
 * 本文件深入演示移动语义的概念和实现：
 * 1. 移动构造函数的实现
 * 2. 移动赋值运算符的实现
 * 3. 移动语义的性能优势
 * 4. 移动语义的最佳实践
 * 5. 资源管理和RAII
 */

/**
 * 资源管理类 - 演示移动语义的重要性
 * 模拟管理大量内存资源的类
 */
class ResourceManager {
private:
    std::unique_ptr<int[]> data;    // 管理的资源
    size_t size;                    // 资源大小
    std::string name;               // 资源名称
    
public:
    // 默认构造函数
    ResourceManager() : data(nullptr), size(0), name("empty") {
        std::cout << "ResourceManager 默认构造: " << name << "\n";
    }
    
    // 带参数构造函数
    ResourceManager(size_t s, const std::string& n) 
        : data(std::make_unique<int[]>(s)), size(s), name(n) {
        
        // 初始化数据
        for (size_t i = 0; i < size; ++i) {
            data[i] = static_cast<int>(i);
        }
        
        std::cout << "ResourceManager 构造: " << name 
                  << " (大小: " << size << ")\n";
    }
    
    // 拷贝构造函数 - 深拷贝，开销大
    ResourceManager(const ResourceManager& other) 
        : data(other.size > 0 ? std::make_unique<int[]>(other.size) : nullptr),
          size(other.size), 
          name(other.name + "_copy") {
        
        // 深拷贝数据
        if (data && other.data) {
            std::copy(other.data.get(), other.data.get() + size, data.get());
        }
        
        std::cout << "ResourceManager 拷贝构造: " << name 
                  << " (从 " << other.name << " 拷贝，大小: " << size << ")\n";
    }
    
    // 移动构造函数 - 转移资源所有权，开销小
    ResourceManager(ResourceManager&& other) noexcept 
        : data(std::move(other.data)),    // 转移unique_ptr的所有权
          size(other.size),
          name(std::move(other.name)) {   // 移动string
        
        // 重置源对象状态
        other.size = 0;
        other.name = "moved_from";
        
        std::cout << "ResourceManager 移动构造: " << name 
                  << " (大小: " << size << ")\n";
    }
    
    // 拷贝赋值运算符
    ResourceManager& operator=(const ResourceManager& other) {
        if (this != &other) {
            // 重新分配资源
            if (other.size > 0) {
                data = std::make_unique<int[]>(other.size);
                std::copy(other.data.get(), other.data.get() + other.size, data.get());
            } else {
                data.reset();
            }
            
            size = other.size;
            name = other.name + "_assigned";
            
            std::cout << "ResourceManager 拷贝赋值: " << name 
                      << " (从 " << other.name << " 赋值，大小: " << size << ")\n";
        }
        return *this;
    }
    
    // 移动赋值运算符
    ResourceManager& operator=(ResourceManager&& other) noexcept {
        if (this != &other) {
            // 转移资源
            data = std::move(other.data);
            size = other.size;
            name = std::move(other.name);
            
            // 重置源对象
            other.size = 0;
            other.name = "moved_from";
            
            std::cout << "ResourceManager 移动赋值: " << name 
                      << " (大小: " << size << ")\n";
        }
        return *this;
    }
    
    // 析构函数
    ~ResourceManager() {
        std::cout << "ResourceManager 析构: " << name 
                  << " (大小: " << size << ")\n";
    }
    
    // 获取信息的方法
    const std::string& getName() const { return name; }
    size_t getSize() const { return size; }
    
    // 获取数据指针（用于验证）
    const int* getData() const { return data.get(); }
    
    // 打印部分数据
    void printSample() const {
        if (data && size > 0) {
            std::cout << "  数据样本: ";
            size_t sampleSize = std::min(size, size_t(5));
            for (size_t i = 0; i < sampleSize; ++i) {
                std::cout << data[i] << " ";
            }
            if (size > 5) std::cout << "...";
            std::cout << "\n";
        } else {
            std::cout << "  无数据\n";
        }
    }
};

/**
 * 演示移动构造函数 vs 拷贝构造函数
 */
void demonstrateMoveVsCopy() {
    std::cout << "\n=== 移动构造 vs 拷贝构造 ===\n";
    
    // 创建原始对象
    ResourceManager original(1000, "原始对象");
    original.printSample();
    
    std::cout << "\n--- 拷贝构造 ---\n";
    ResourceManager copied = original;  // 调用拷贝构造函数
    copied.printSample();
    
    std::cout << "\n--- 移动构造 ---\n";
    ResourceManager moved = std::move(original);  // 调用移动构造函数
    moved.printSample();
    
    std::cout << "原始对象状态: " << original.getName() 
              << " (大小: " << original.getSize() << ")\n";
}

/**
 * 演示移动赋值 vs 拷贝赋值
 */
void demonstrateMoveAssignment() {
    std::cout << "\n=== 移动赋值 vs 拷贝赋值 ===\n";
    
    ResourceManager source(500, "源对象");
    ResourceManager target1(100, "目标对象1");
    ResourceManager target2(200, "目标对象2");
    
    std::cout << "\n--- 拷贝赋值 ---\n";
    target1 = source;  // 调用拷贝赋值运算符
    
    std::cout << "\n--- 移动赋值 ---\n";
    target2 = std::move(source);  // 调用移动赋值运算符
    
    std::cout << "源对象状态: " << source.getName() 
              << " (大小: " << source.getSize() << ")\n";
}

/**
 * 演示容器中的移动语义
 */
void demonstrateContainerMoveSemantics() {
    std::cout << "\n=== 容器中的移动语义 ===\n";
    
    std::vector<ResourceManager> vec;
    
    std::cout << "--- 使用拷贝添加元素 ---\n";
    ResourceManager obj1(100, "对象1");
    vec.push_back(obj1);  // 拷贝
    
    std::cout << "\n--- 使用移动添加元素 ---\n";
    vec.push_back(ResourceManager(200, "临时对象"));  // 移动（临时对象）
    vec.push_back(std::move(obj1));  // 移动（显式移动）
    
    std::cout << "\n--- 容器内容 ---\n";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "元素 " << i << ": " << vec[i].getName() 
                  << " (大小: " << vec[i].getSize() << ")\n";
    }
}

/**
 * 性能比较：移动 vs 拷贝
 */
void performanceComparison() {
    std::cout << "\n=== 性能比较 ===\n";
    
    const size_t iterations = 1000;
    const size_t objectSize = 10000;
    
    // 测试拷贝性能
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<ResourceManager> copyVec;
    for (size_t i = 0; i < iterations; ++i) {
        ResourceManager temp(objectSize, "temp_" + std::to_string(i));
        copyVec.push_back(temp);  // 拷贝
    }
    
    auto copyTime = std::chrono::high_resolution_clock::now() - start;
    
    // 测试移动性能
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<ResourceManager> moveVec;
    for (size_t i = 0; i < iterations; ++i) {
        ResourceManager temp(objectSize, "temp_" + std::to_string(i));
        moveVec.push_back(std::move(temp));  // 移动
    }
    
    auto moveTime = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "拷贝操作耗时: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(copyTime).count() 
              << " ms\n";
    std::cout << "移动操作耗时: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(moveTime).count() 
              << " ms\n";
    
    double speedup = static_cast<double>(copyTime.count()) / moveTime.count();
    std::cout << "移动语义性能提升: " << speedup << "x\n";
}

/**
 * 演示返回值优化和移动语义
 */
ResourceManager createLargeObject(size_t size, const std::string& name) {
    std::cout << "创建大对象: " << name << "\n";
    return ResourceManager(size, name);  // 返回值优化 (RVO) 或移动
}

void demonstrateRVO() {
    std::cout << "\n=== 返回值优化 (RVO) ===\n";
    
    // 现代编译器通常会进行返回值优化，避免不必要的拷贝/移动
    ResourceManager result = createLargeObject(1000, "RVO对象");
    std::cout << "结果对象: " << result.getName() << "\n";
}

/**
 * 移动语义的最佳实践
 */
class BestPracticeExample {
private:
    std::vector<int> data;
    std::string name;
    
public:
    // 构造函数
    BestPracticeExample(std::vector<int> d, std::string n) 
        : data(std::move(d)), name(std::move(n)) {  // 参数移动
        std::cout << "BestPracticeExample 构造: " << name << "\n";
    }
    
    // 移动构造函数 - 标记为 noexcept
    BestPracticeExample(BestPracticeExample&& other) noexcept 
        : data(std::move(other.data)), name(std::move(other.name)) {
        std::cout << "BestPracticeExample 移动构造\n";
    }
    
    // 移动赋值运算符 - 标记为 noexcept
    BestPracticeExample& operator=(BestPracticeExample&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            name = std::move(other.name);
        }
        std::cout << "BestPracticeExample 移动赋值\n";
        return *this;
    }
    
    // 删除拷贝操作（如果只需要移动语义）
    BestPracticeExample(const BestPracticeExample&) = delete;
    BestPracticeExample& operator=(const BestPracticeExample&) = delete;
    
    const std::string& getName() const { return name; }
    size_t getDataSize() const { return data.size(); }
};

void demonstrateBestPractices() {
    std::cout << "\n=== 移动语义最佳实践 ===\n";
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::string str = "最佳实践示例";
    
    // 构造时移动参数
    BestPracticeExample example(std::move(vec), std::move(str));
    
    std::cout << "示例对象: " << example.getName() 
              << " (数据大小: " << example.getDataSize() << ")\n";
    
    // vec和str现在处于有效但未指定状态
    std::cout << "移动后 vec 大小: " << vec.size() << "\n";
    std::cout << "移动后 str: '" << str << "'\n";
}

/**
 * 移动语义的陷阱和注意事项
 */
void demonstrateMovePitfalls() {
    std::cout << "\n=== 移动语义陷阱 ===\n";
    
    // 陷阱1：对已移动对象的使用
    std::cout << "--- 陷阱1: 使用已移动对象 ---\n";
    ResourceManager obj1(100, "原对象");
    ResourceManager obj2 = std::move(obj1);
    
    // obj1现在处于有效但未指定状态，应避免使用
    std::cout << "已移动对象状态: " << obj1.getName() 
              << " (大小: " << obj1.getSize() << ")\n";
    
    // 陷阱2：移动const对象
    std::cout << "\n--- 陷阱2: 移动const对象 ---\n";
    const ResourceManager constObj(50, "常量对象");
    // ResourceManager moved = std::move(constObj);  // 实际上会调用拷贝构造！
    
    // 陷阱3：返回局部变量时不要使用std::move
    std::cout << "\n--- 陷阱3: 返回局部变量 ---\n";
    auto createObject = []() -> ResourceManager {
        ResourceManager local(100, "局部对象");
        return local;  // 正确：依赖RVO，不要使用std::move(local)
    };
    
    ResourceManager result = createObject();
    std::cout << "返回的对象: " << result.getName() << "\n";
}

int main() {
    std::cout << "C++ 移动语义详细示例\n";
    std::cout << "=====================\n";
    
    try {
        demonstrateMoveVsCopy();
        demonstrateMoveAssignment();
        demonstrateContainerMoveSemantics();
        demonstrateRVO();
        demonstrateBestPractices();
        demonstrateMovePitfalls();
        
        // 性能比较（可能耗时较长）
        std::cout << "\n是否进行性能测试？这可能需要一些时间...\n";
        performanceComparison();
        
        std::cout << "\n=== 程序执行完成 ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
 * 移动语义关键要点：
 * 
 * 1. 移动构造函数：
 *    - 转移资源所有权而不是拷贝
 *    - 应标记为 noexcept
 *    - 源对象应处于有效但未指定状态
 * 
 * 2. 移动赋值运算符：
 *    - 检查自赋值
 *    - 释放当前资源
 *    - 转移新资源
 *    - 应标记为 noexcept
 * 
 * 3. 性能优势：
 *    - 避免深拷贝
 *    - 减少内存分配
 *    - 提高容器操作效率
 * 
 * 4. 最佳实践：
 *    - 移动操作标记 noexcept
 *    - 参数传递时考虑移动
 *    - 合理使用 std::move
 *    - 避免移动 const 对象
 * 
 * 5. 注意事项：
 *    - 不要使用已移动对象
 *    - 返回局部变量时依赖RVO
 *    - 移动操作应该是安全的
 */