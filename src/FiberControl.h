/**
 * @file Fiber.h
 * @brief 线程内部协程的控制
 * @details 主要是维护当前线程上的协程控制信息，进行线程内部协程的控制，例如调度什么的
 * @author wenxingming
 * @date 2025-08-29
 * @note My project address:
 * @cite https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */
#pragma once
#include <memory>
#include <cassert>
#include <functional>

namespace wxm {

class Fiber; // 头文件不可相互包含，使用前向声明
class FiberControl {
public:
    static thread_local std::shared_ptr<Fiber> runningFiber; // 运行中的协程
    static thread_local std::shared_ptr<Fiber> mainFiber; // 主协程
    static thread_local std::shared_ptr<Fiber> schedulerFiber; // 调度协程
    static thread_local int fiberId; // 协程 ID
    static thread_local int fiberCount; // 协程计数器
    static thread_local bool debug;

public:
    static void set_running_fiber(std::shared_ptr<Fiber> f); // 设置当前运行的协程
    static std::shared_ptr<Fiber> get_running_fiber(); // 得到当前运行的协程
    static void first_create_fiber(); // 如果没有协程，则调用此函数创建协程（并初始化线程中协程的 FiberControl 信息）
    static void set_scheduler_fiber(std::shared_ptr<Fiber> fiber);
    static uint64_t get_fiber_id();

};


}