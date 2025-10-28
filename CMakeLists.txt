cmake_minimum_required(VERSION 3.16)
project(RValueReferenceAndPerfectForwarding VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型为Debug（如果未指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    # 添加C++20特性支持
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
    # 调试模式：添加调试符号，禁用优化，启用调试友好选项
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -DDEBUG -fno-omit-frame-pointer -fno-inline")
    # 发布模式：启用优化
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# 强制启用调试信息
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

# 创建源文件目录
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)

# 包含头文件目录
include_directories(${INCLUDE_DIR})

# 添加可执行文件
add_executable(rvalue_basics ${EXAMPLES_DIR}/rvalue_basics.cpp)
add_executable(move_semantics ${EXAMPLES_DIR}/move_semantics.cpp)
add_executable(perfect_forwarding ${EXAMPLES_DIR}/perfect_forwarding.cpp)
add_executable(comprehensive_example ${EXAMPLES_DIR}/comprehensive_example.cpp)
add_executable(cpp20_advanced ${EXAMPLES_DIR}/cpp20_advanced.cpp)

# 设置输出目录
set_target_properties(rvalue_basics move_semantics perfect_forwarding comprehensive_example cpp20_advanced
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 添加自定义目标用于运行所有示例
add_custom_target(run_all_examples
    COMMAND echo "运行右值引用基础示例:"
    COMMAND $<TARGET_FILE:rvalue_basics>
    COMMAND echo "\\n运行移动语义示例:"
    COMMAND $<TARGET_FILE:move_semantics>
    COMMAND echo "\\n运行完美转发示例:"
    COMMAND $<TARGET_FILE:perfect_forwarding>
    COMMAND echo "\\n运行综合示例:"
    COMMAND $<TARGET_FILE:comprehensive_example>
    COMMAND echo "\\n运行C++20高级示例:"
    COMMAND $<TARGET_FILE:cpp20_advanced>
    DEPENDS rvalue_basics move_semantics perfect_forwarding comprehensive_example cpp20_advanced
    COMMENT "运行所有示例程序"
)