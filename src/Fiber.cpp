/**
 * @file Fiber.cpp
 * @brief 协程类
 * @details 自定义实现 C++ 协程
 * @author wenxingming
 * @date 2025-08-31
 * @note My project address: https://github.com/WenXingming/Coroutine
 * @cite https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */

#include <iostream>
#include "Fiber.h"
#include "FiberControl.h" // 可以都包含

namespace wxm {

    wxm::Fiber::Fiber(uint64_t _id) {
        /* 不可在构造函数调用 shared_from_this()，见 C++ 标准库（第二版）5.2.3。移动到了工厂函数 FiberControl::first_create_fiber 里。
        FiberControl::set_running_fiber(shared_from_this()); */
        id = _id;

        int retGetContext = getcontext(&context);
        if (retGetContext != 0) {
            std::cerr << "Fiber() failed." << std::endl;
            assert(retGetContext == 0);
        }
        stackSize = 128 * 1024; // 128 KB
        stackPtr = operator new(stackSize);
        context.uc_link = nullptr;
        context.uc_stack.ss_sp = stackPtr;
        context.uc_stack.ss_size = stackSize;
        // makecontext(&context, &Fiber::main_func, 0); // 主协程。不需要 main_func 函数作为 context 上下文的下一条执行指令
        task = nullptr;
        state = READY;
        // runInScheduler = false;
        if (FiberControl::get_debug()) std::cout << "Fiber(): main id = " << id << std::endl;
    }


    wxm::Fiber::Fiber(uint64_t _id, std::function<void()> _cb, size_t _stacksize, bool _run_in_scheduler) {
        id = _id;

        int retGetContext = getcontext(&context); // 使用 getcontext 是先将大部分信息初始化，我们只需要修改我们所使用的部分信息即可
        if (retGetContext != 0) {
            std::cerr << "Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler) failed." << std::endl;
            assert(retGetContext == 0);
        }
        stackSize = _stacksize != 0 ? _stacksize : 128 * 1024; // 128 KB
        stackPtr = operator new(stackSize);
        context.uc_link = nullptr;
        context.uc_stack.ss_sp = stackPtr;
        context.uc_stack.ss_size = stackSize;
        makecontext(&context, &Fiber::main_func, 0); // makecontext 修改通过 getcontext 取得的上下文 ucp (这意味着调用 makecontext 前必须先调用getcontext)

        task = _cb;
        state = READY;
        runInScheduler = _run_in_scheduler;

        if (FiberControl::get_debug()) std::cout << "Fiber(): child id = " << id << std::endl;
    }


    wxm::Fiber::~Fiber() {
        // 这个没办法...FiberControl::threadFiberCount 只能你维护一下了...
        int threadFiberCount = FiberControl::get_thread_fiber_count();
        FiberControl::set_thread_fiber_count(threadFiberCount - 1);

        if (stackPtr) {
            operator delete(stackPtr);
        }

        if (FiberControl::get_debug()) std::cout << "~Fiber(): fiber id = " << id << std::endl;
    }


    void wxm::Fiber::reset(std::function<void()> _cb) {
        if (!stackPtr || state != TERM) {
            std::cerr << "reset error." << std::endl;
            assert(stackPtr && state == TERM);
        }

        int retGetContext = getcontext(&context);
        if (retGetContext != 0) {
            std::cerr << "reset() failed." << std::endl;
            assert(retGetContext == 0);
        }
        context.uc_link = nullptr;
        context.uc_stack.ss_sp = stackPtr;
        context.uc_stack.ss_size = stackSize;
        makecontext(&context, &Fiber::main_func, 0);

        task = _cb;
        state = READY;
    }


    /// @brief 当前 context 放到 cpu，cpu 状态放到调度协程（或主协程）。维护 FiberControl（不够高内聚、低耦合）。
    /// @details 总之，流程可以统一为先维护 runningFiber，然后把 runningFiber->context 放到 cpu，保存 cpu 到调度协程（或主协程）
    void wxm::Fiber::resume() {
        assert(state == READY);

        // FiberControl::set_running_fiber(shared_from_this()); // 提前设置当前协程为运行协程
        auto thisFiber = shared_from_this();
        FiberControl::set_running_fiber(thisFiber);
        if (runInScheduler) {
            auto runningFiber = FiberControl::get_running_fiber();
            auto schedulerFiber = FiberControl::get_scheduler_fiber();

            // 保存 cpu 到 schedulerFiber->context，切换 runningFiber->context 到 cpu 并执行。当切换回 schedulerFiber->context 时返回 retSwapContext，-1 为切换失败，0 为成功
            // 调度的关键！把当前线程的状态存入主协程！然后运行子协程。子协程运行完后自动 yield，回到主协程（现在线程的状态）
            int retSwapContext = swapcontext(&(schedulerFiber->context), &(runningFiber->context));
            if (retSwapContext != 0) {
                std::cerr << "resume() from other to schedulerFiber failed\n";
                assert(retSwapContext == 0);
            }
        }
        else {
            auto runningFiber = FiberControl::get_running_fiber();
            auto mainFiber = FiberControl::get_main_fiber();

            int retSwapContext = swapcontext(&(mainFiber->context), &(runningFiber->context));
            if (retSwapContext != 0) {
                std::cerr << "resume() from other to mainFiber failed\n";
                assert(retSwapContext == 0);
            }
        }
    }


    /// @brief 调度协程（或主协程） context 放到 cpu，cpu 状态放到当前协程。维护 FiberControl（不够高内聚、低耦合）
    /// @details 总之，流程可以统一为先维护 runningFiber，然后把 runningFiber->context 放到 cpu，保存 cpu 到当前协程
    void wxm::Fiber::yield() {
        assert(state == RUNNING || state == TERM);
        if (state == RUNNING) state = READY;

        if (runInScheduler) {
            auto schedulerFiber = FiberControl::get_scheduler_fiber();
            FiberControl::set_running_fiber(schedulerFiber); // 提前设置调度协程为运行协程
            auto runningFiber = FiberControl::get_running_fiber();

            int retSwapContext = swapcontext(&context, &(runningFiber->context)); // 让出执行权
            if (retSwapContext != 0) {
                std::cerr << "resume() from other(schedulerFiber) to this->context failed\n";
                assert(retSwapContext == 0);
            }
        }
        else {
            auto mainFiber = FiberControl::get_main_fiber();
            FiberControl::set_running_fiber(mainFiber); // 提前设置调度协程为运行协程
            auto runningFiber = FiberControl::get_running_fiber();

            int retSwapContext = swapcontext(&context, &(runningFiber->context)); // 让出执行权
            if (retSwapContext != 0) {
                std::cerr << "resume() from other(mainFiber) to this->context failed\n";
                assert(retSwapContext == 0);
            }
        }
    }


    uint64_t wxm::Fiber::get_id() const {
        return id;
    }


    wxm::Fiber::State wxm::Fiber::get_state() const {
        return state;
    }


    void wxm::Fiber::main_func() {
        /* 大坑！如果 curr 是智能指针，yeild 后如果没有被 resume 则永远没有销毁！内存泄漏！正常情况下智能指针离开作用域计数器自动减少，关键是这里 yield 不会离开作用域！
        auto curr = FiberControl::get_running_fiber(); */
        auto curr = FiberControl::get_running_fiber().get(); // 方法 1 (缺点是混用了智能指针和普通指针,不是很安全)
        assert(curr);

        curr->task();
        curr->task = nullptr;
        curr->state = TERM;

        // 运行完毕 ——> 让出执行权
        curr->yield();
    }


}