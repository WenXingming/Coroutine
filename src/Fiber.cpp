/**
 * @file Fiber.cpp
 * @brief 协程类
 * @details 自定义实现 C++ 协程
 * @author wenxingming
 * @date 2025-08-31
 * @note My project address:
 * @cite https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */

#include <iostream>
#include "Fiber.h"
#include "FiberControl.h" // 可以都包含

namespace wxm {
    /// =========================================================================
    /// Definition of Fiber class's member functions

    wxm::Fiber::Fiber() {
        // FiberControl::set_running_fiber(shared_from_this());
        id = FiberControl::get_thread_fiber_count();

        int retGetContext = getcontext(&context); // 使用 getcontext 是先将大部分信息初始化，我们到时候只需要修改我们所使用的部分信息即可
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


    wxm::Fiber::Fiber(std::function<void()> _cb, size_t _stacksize, bool _run_in_scheduler) {
        id = FiberControl::get_thread_fiber_count();

        int retGetContext = getcontext(&context);
        if (retGetContext != 0) {
            std::cerr << "Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler) failed." << std::endl;
            assert(retGetContext == 0);
        }
        stackSize = _stacksize != 0 ? _stacksize : 128 * 1024; // 128 KB
        stackPtr = operator new(stackSize);
        context.uc_link = nullptr;
        context.uc_stack.ss_sp = stackPtr;
        context.uc_stack.ss_size = stackSize; // 给该上下文指定一个栈空间 ucp->stack
        makecontext(&context, &Fiber::main_func, 0); // makecontext 修改通过 getcontext 取得的上下文 ucp (这意味着调用makecontext前必须先调用getcontext)。

        task = _cb;
        state = READY;
        runInScheduler = _run_in_scheduler;

        if (FiberControl::get_debug()) std::cout << "Fiber(): child id = " << id << std::endl;
    }


    wxm::Fiber::~Fiber() {
        int threadId = FiberControl::get_thread_fiber_count();
        FiberControl::set_thread_fiber_count(--threadId);

        if (stackPtr) {
            operator delete(stackPtr);
        }

        if (FiberControl::get_debug()) std::cout << "~Fiber(): child id = " << id << std::endl;
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


    void wxm::Fiber::resume() {
        assert(state == READY);

        if (runInScheduler) {
            FiberControl::set_running_fiber(shared_from_this());
            int retSwapContext = swapcontext(&(FiberControl::get_scheduler_fiber()->context), &context);
            if (retSwapContext != 0) {
                std::cerr << "resume() from schedulerFiber to this->context failed\n";
                assert(retSwapContext == 0);
            }
        }
        else {
            FiberControl::set_running_fiber(shared_from_this());
            int retSwapContext = swapcontext(&(FiberControl::get_main_fiber()->context), &context);
            if (retSwapContext != 0) {
                std::cerr << "resume() from mainFiber to this->context failed\n";
                assert(retSwapContext == 0);
            }
        }
    }


    void wxm::Fiber::yield() {
        assert(state == RUNNING || state == TERM);

        if (state == RUNNING) state = READY;

        if (runInScheduler) {
            FiberControl::set_running_fiber(FiberControl::get_scheduler_fiber());
            int retSwapContext = swapcontext(&context, &(FiberControl::get_scheduler_fiber()->context));
            if (retSwapContext != 0) {
                std::cerr << "resume() from this->context to schedulerFiber failed\n";
                assert(retSwapContext == 0);
            }
        }
        else {
            FiberControl::set_running_fiber(FiberControl::get_main_fiber());
            int retSwapContext = swapcontext(&context, &(FiberControl::get_main_fiber()->context));
            if (retSwapContext != 0) {
                std::cerr << "resume() from this->context to mainFiber failed\n";
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
        auto curr = FiberControl::get_running_fiber();
        assert(curr);

        curr->task();
        curr->task = nullptr;
        curr->state = TERM;

        // 运行完毕 ——> 让出执行权
        curr->yield();
    }


}