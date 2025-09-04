/**
 * @file UnitTest.cpp
 * @brief 单元测试
 * @author wenxingming
 * @date 2025-08-31
 * @note My project address: https://github.com/WenXingming/Coroutine
 */

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>
#include "Semaphore.h" 
#include "Fiber.h"
#include "FiberControl.h"


/// @brief Test Semaphore: 基本阻塞与唤醒 (test_basic_semaphore)
void test_basic_semaphore() {
    std::cout << "--- Testing Basic Semaphore Operations ---" << std::endl;
    wxm::Semaphore sem(1); // 初始计数为1

    std::atomic<bool> flag(false);
    std::thread t1([&]() { // 线程 1 先获取信号量
        sem.wait();
        flag = true;
        std::cout << "Thread 1: Semaphore acquired." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 先不释放，看线程 2 是否能获取
        sem.signal();
        std::cout << "Thread 1: Semaphore freed." << std::endl;
        });

    // 主线程等待一小段时间，确保 t1 已经执行
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::thread t2([&]() { // 线程 2 尝试获取信号量，由于 t1 还没有释放，它应该被阻塞
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

    const int thread_count = 12;
    const int initial_count = 3;
    wxm::Semaphore sem(initial_count); // 允许 3 个线程同时访问

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


/// @brief Test Fiber、FiberControl 功能
/// @details 单线程调度器（类似于线程池，更加简单，无需加锁且 FCFS 先来先服务）
class Scheduler {
private:
    std::vector<std::shared_ptr<wxm::Fiber>> tasks;

public:
    // 添加协程调度任务
    void submit_task(std::shared_ptr<wxm::Fiber> task) {
        tasks.push_back(task);
    }

    // 执行调度任务
    void run() {
        std::cout << "Thread's fiber number: " << wxm::FiberControl::get_thread_fiber_count() << std::endl;

        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
            // std::shared_ptr<wxm::Fiber> task = *it;
            // task->resume(); // 由主协程切换到子协程，子协程函数运行完毕后自动切换到主协程
            (*it)->resume();
            // std::cout << "智能指针引用计数, Fiber Id: " << (*it)->get_id() << " reference num: " << (*it).use_count() << std::endl;
        }

        // C++标准库（第2版）：由于在最后一个shared_ptr销毁前内存都不会释放，保证shared_ptr在无用之后不再保留就非常重要了。如果你忘记了销毁程序不再需要的shared_ptr，程序仍会正确执行，但会浪费内存。share_ptr在无用之后仍然保留的一种可能情况是，你将shared_ptr存放在一个容器中，随后重排了容器，从而不再需要某些元素。在这种情况下，你应该确保用erase删除那些不再需要的shared_ptr元素。
        tasks.clear(); // 即当容器中的 shared_ptr 不再被需要时，记得 erase() 这些 shared_ptr
    }

};


/// @brief 测试 Fiber、FiberControl
void test_fiber_total() {
    std::cout << "--- Testing test_fiber_total ---" << std::endl;

    // wxm::FiberControl::first_create_fiber(); // 初始化当前线程的主协程
    // 创建调度器、添加调度任务（任务和子协程绑定）
    {
        Scheduler sc;
        for (auto i = 0; i < 20; i++) {
            // 创建子协程
            // 使用共享指针自动管理资源 -> 过期后自动释放子协程创建的资源
            // bind 函数 -> 绑定函数和参数用来返回一个函数对象
            auto func = [](int i) {
                std::cout << "hello world: " << i << std::endl;
                };
            std::shared_ptr<wxm::Fiber> fiber = wxm::FiberControl::create_fiber(std::bind(func, i), 0, false);
            // std::cout << "智能指针引用计数, Fiber Id: " << fiber->get_id() << " reference num: " << fiber.use_count() << std::endl;
            sc.submit_task(fiber);
        }

        // 执行调度任务
        sc.run();

    }
    std::cout << "--- test_fiber_total Passed ---" << std::endl;
}


int main() {
    test_basic_semaphore();
    std::cout << "\n";
    test_concurrency_with_worker_threads();
    std::cout << "\n";
    test_fiber_total();
    std::cout << "\n";

    std::cout << "All tests completed successfully!\n" << std::endl;
    return 0;
}