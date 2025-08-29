#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>

#include "Semaphore.h" 
using namespace wxm;

/// ==================================================================================
/// NOTE: 这个单元测试旨在验证您的信号量实现在两个关键场景下的行为是否正确：

/// @brief Test Semaphore: 基本阻塞与唤醒 (test_basic_semaphore)
void test_basic_semaphore() {
    std::cout << "--- Testing Basic Semaphore Operations ---" << std::endl;
    wxm::Semaphore sem(1); // 初始计数为1

    std::atomic<bool> flag(false);
    std::thread t1([&]() {
        // 线程1先获取信号量
        sem.wait();
        flag = true;
        std::cout << "Thread 1: Semaphore acquired." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sem.signal();
        std::cout << "Thread 1: Semaphore freed." << std::endl;
        });

    // 主线程等待一小段时间，确保 t1 已经执行
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::thread t2([&]() {
        // 线程2尝试获取信号量，由于 t1 还没有释放，它应该被阻塞
        std::cout << "Thread 2: Trying to acquire semaphore..." << std::endl;
        assert(flag.load() == true); // 确保在 t2 wait() 之前 t1 已经执行了
        sem.wait();
        std::cout << "Thread 2: Semaphore acquired." << std::endl;
        sem.signal();
        std::cout << "Thread 2: Semaphore freed." << std::endl;
        });

    t1.join();
    t2.join();
    std::cout << "--- Basic Semaphore Test Passed ---" << std::endl;
}

/// @brief Test Semaphore: 多线程并发控制 (test_concurrency_with_worker_threads)：
void test_concurrency_with_worker_threads() {
    std::cout << "--- Testing Concurrent Access ---" << std::endl;

    const int thread_count = 10;
    const int initial_count = 3;
    wxm::Semaphore sem(initial_count); // 允许3个线程同时访问

    std::atomic<int> running_count(0);
    std::vector<std::thread> workers;

    for (int i = 0; i < thread_count; ++i) {
        workers.emplace_back(std::thread([&]() {
            sem.wait();
            int current_running_count = ++running_count;
            std::cout << "Thread " << std::this_thread::get_id() << " acquired semaphore. Current running: " << current_running_count << std::endl;

            // 检查并发数是否超过了初始值
            assert(current_running_count <= initial_count);

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟工作

            --running_count;
            sem.signal();
            }));
    }

    for (auto& t : workers) {
        t.join();
    }

    std::cout << "--- Concurrent Access Test Passed ---" << std::endl;
}


int main() {
    test_basic_semaphore();
    std::cout << "\n";
    test_concurrency_with_worker_threads();
    std::cout << "\n";

    std::cout << "All tests completed successfully!\n" << std::endl;
    return 0;
}