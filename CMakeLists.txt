cmake_minimum_required(VERSION 3.16)
project(ThreadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
