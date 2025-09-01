/**
 * @file Semaphore.h
 * @brief 信号量类。Declaration of class
 * @details 自定义实现 C++ 信号量
 * @author wenxingming
 * @date 2025-08-29
 * @note My project address:
 * @cite https://github.com/youngyangyang04/coroutine-lib/blob/main/fiber_lib/1thread/thread.h
 */

#pragma once
#include <iostream>
#include <mutex>
#include <condition_variable>
namespace wxm {



class Semaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;                      // 使用原子变量还是锁？需要使用锁，因为 Semaphore::wait() 里要保护临界区（count 的判断和修改）

public:
    explicit Semaphore(int _count); // explicit: 构造函数只能被显式地调用，不允许进行隐式转换。
    ~Semaphore();

    // PV 操作
    void wait();
    void signal();
};



}