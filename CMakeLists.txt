cmake_minimum_required(VERSION 3.16)
project(ThreadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 解决 MSVC 中文乱码问题：指定源文件和执行字符集均为 UTF-8
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 收集源文件
file(GLOB SOURCES "src/*.cpp")

# 可执行文件
add_executable(ThreadPool
    main.cpp
    ${SOURCES}
)

# 链接 pthread（Linux/macOS 需要）
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(ThreadPool Threads::Threads)
endif()
