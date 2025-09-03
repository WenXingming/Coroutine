cmake_minimum_required(VERSION 3.10)
project(coroutine)

# 设置 C++ 标准。设置 C++ 标准是一个好习惯，这能确保你的代码在不同编译器上行为一致，并能使用最新的语言特性。
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成可执行文件
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB source_files ${source_dir}/*.cpp)
add_executable(unitTest ${source_files})

# 链接 -lpthread。Linux 下 std::thread 依赖 pthread
find_package(Threads REQUIRED)
target_link_libraries(unitTest PRIVATE Threads::Threads)