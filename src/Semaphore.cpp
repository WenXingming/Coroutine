/**
 * @file Semaphore.cpp
 * @brief 信号量类。Definition of class's member functions
 * @details 自定义实现 C++ 信号量
 * @attention cpp 文件定义的成员函数不应该加 inline, undefined reference to `wxm::Semaphore::Semaphore(int)'
 * @author wenxingming
 * @date 2025-08-29
 * @note My project address:
 * @cite https://github.com/youngyangyang04/coroutine-lib/blob/main/fiber_lib/1thread/thread.h
 */

#include "Semaphore.h"
namespace wxm {



Semaphore::Semaphore(int _count = 0) : count(_count) {}

Semaphore::~Semaphore() {}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(mtx); // std::unique_lock 提供了灵活的锁定和解锁能力；std::lock_guard 完全 RAII，没有提供手动解锁的接口。
    cv.wait(lock, [this]() {
        return this->count != 0;
        });
    --count;
}

void Semaphore::signal() {
    std::unique_lock<std::mutex> lock(mtx);
    ++count;
    cv.notify_one();
}



}