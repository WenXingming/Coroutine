/**
 * @file Fiber.h
 * @brief 协程类
 * @details 自定义实现 C++ 协程
 * @author wenxingming
 * @date 2025-08-29
 * @note My project address:
 * @cite https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */

#pragma once
#include <iostream>
#include <ucontext.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <mutex>
#include <cassert>
#include <thread>

namespace wxm {
/// =========================================================================
/// Declaration of Fiber class

class FiberControl; // 头文件不可相互包含，使用前向声明
class Fiber : public std::enable_shared_from_this<Fiber> { // 允许一个类（Fiber）的对象安全地获取一个指向自身的 std::shared_ptr
// private:
public:
    enum State{ 
        READY, RUNNING, TERM
    }; // 协程状态：准备、运行、结束
    
    uint64_t id;                    // 协程的唯一标识符
    ucontext_t context;             // 协程的上下文 context
    void* stackPtr;                 // 协程栈的指针
    uint32_t stackSize;             // 栈的大小
    std::function<void()> task;     // 协程的执行函数（回调函数？）
    State state = READY;            // 协程状态
    bool runInScheduler;            // 是否让出执行权交给调度协程
public:
    std::mutex mutex;

public:
    Fiber();
    Fiber(std::function<void()> _cb, size_t _stacksize = 0, bool _run_in_scheduler = true);
    Fiber(const Fiber& other) = delete;
    Fiber& operator=(const Fiber& other) = delete;
    ~Fiber();

    void reset(std::function<void()> _cb); // 重用一个协程
    void resume();
    void yield();

    uint64_t get_id() const;
    State get_state() const;

    static void main_func();
};


}
