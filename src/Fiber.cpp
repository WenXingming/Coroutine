#include "Fiber.h"
#include "FiberControl.h" // 可以都包含


namespace wxm{
/// =========================================================================
/// Definition of Fiber class's member functions

wxm::Fiber::Fiber() {
    // FiberControl::set_running_fiber(shared_from_this());

    id = FiberControl::fiberCount++;
    int retGetContext = getcontext(&context);
    if (retGetContext == -1) {
        std::cerr << "Fiber() failed." << std::endl;
        assert(retGetContext == 0);
    }
    state = READY;
    // runInScheduler
    if (FiberControl::debug) std::cout << "Fiber(): main id = " << id << std::endl;
}


wxm::Fiber::Fiber(std::function<void()> _cb, size_t _stacksize, bool _run_in_scheduler) {
    id = FiberControl::fiberCount++;

    int retGetContext = getcontext(&context);
    if (retGetContext != 0) {
        std::cerr << "Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler) failed." << std::endl;
        assert(retGetContext == 0);
    }
    stackSize = _stacksize != 0 ? _stacksize : 128000;
    stackPtr = operator new(stackSize);
    context.uc_link = nullptr;
    context.uc_stack.ss_sp = stackPtr;
    context.uc_stack.ss_size = stackSize; // 给该上下文指定一个栈空间 ucp->stack
    makecontext(&context, /* task */ &Fiber::main_func, 0); // makecontext 修改通过 getcontext 取得的上下文 ucp (这意味着调用makecontext前必须先调用getcontext)。
    task = _cb;
    state = READY;
    runInScheduler = _run_in_scheduler;

    if (FiberControl::debug) std::cout << "Fiber(): child id = " << id << std::endl;
}


wxm::Fiber::~Fiber() {
    --FiberControl::fiberCount;
    if (stackPtr) {
        operator delete(stackPtr);
    }

    if (FiberControl::debug) std::cout << "~Fiber(): child id = " << id << std::endl;
}


inline void wxm::Fiber::reset(std::function<void()> _cb) {
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
    context.uc_stack.ss_size = stackSize; // 给该上下文指定一个栈空间 ucp->stack
    makecontext(&context, /* task */ &Fiber::main_func, 0);

    task = _cb;
    state = READY;
}


void wxm::Fiber::resume() {
    assert(state == READY);

    if (runInScheduler) {
        FiberControl::set_running_fiber(shared_from_this());
        int retSwapContext = swapcontext(&(FiberControl::schedulerFiber->context), &context);
        if (retSwapContext != 0) {
            std::cerr << "resume() from schedulerFiber to this->context failed\n";
            assert(retSwapContext == 0);
        }
    }
    else {
        FiberControl::set_running_fiber(shared_from_this());
        int retSwapContext = swapcontext(&(FiberControl::mainFiber->context), &context);
        if (retSwapContext != 0) {
            std::cerr << "resume() from mainFiber to this->context failed\n";
            assert(retSwapContext == 0);
        }
    }
}


inline void wxm::Fiber::yield() {
    assert(state == RUNNING || state == TERM);

    if (state == RUNNING) state = READY;

    if (runInScheduler) {
        FiberControl::set_running_fiber(FiberControl::schedulerFiber);
        int retSwapContext = swapcontext(&context, &(FiberControl::schedulerFiber->context));
        if (retSwapContext != 0) {
            std::cerr << "resume() from this->context to schedulerFiber failed\n";
            assert(retSwapContext == 0);
        }
    }
    else {
        FiberControl::set_running_fiber(FiberControl::mainFiber);
        int retSwapContext = swapcontext(&context, &(FiberControl::mainFiber->context));
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


inline void wxm::Fiber::main_func() {
    auto curr = FiberControl::get_running_fiber();
    assert(curr);

    curr->task();
    curr->task = nullptr;
    curr->state = TERM;

    // 运行完毕 ——> 让出执行权
    curr->yield();
}


}