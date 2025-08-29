/**
 * @file Semaphore.h
 * @brief 信号量类
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


/// =============================================================================
/// NOTE: Declaration of class
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



/// =============================================================================
/// NOTE: Definition of class's member functions

inline Semaphore::Semaphore(int _count = 0) : count(_count) {}

inline Semaphore::~Semaphore() {}

inline void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(mtx); // std::unique_lock 提供了灵活的锁定和解锁能力；std::lock_guard 完全 RAII，没有提供手动解锁的接口。
    cv.wait(lock, [=]() {
        return this->count != 0;
        });
    --count;
}

inline void Semaphore::signal() {
    std::unique_lock<std::mutex> lock(mtx);
    ++count;
    cv.notify_one();
}


}

