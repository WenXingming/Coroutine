cmake_minimum_required(VERSION 3.10)
project(coroutine)

# 生成可执行文件
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB source_files ${source_dir}/*.cpp)
add_executable(test ${source_files})

# 链接 -lpthread。Linux 下 std::thread 依赖 pthread
find_package(Threads REQUIRED)
target_link_libraries(test PRIVATE Threads::Threads)