#include "Fiber.h" // 可以都包含
#include "FiberControl.h"

namespace wxm {

// 静态成员变量不像成员变量可以在定义对象时初始化，必须手动初始化！生命周期与程序的生命周期相同
// 注意，这里没有 "static"，因为它的属性在类中已经声明了
thread_local std::shared_ptr<Fiber> FiberControl::runningFiber(nullptr);
thread_local std::shared_ptr<Fiber> FiberControl::mainFiber(nullptr);
thread_local std::shared_ptr<Fiber> FiberControl::schedulerFiber(nullptr);
thread_local int FiberControl::fiberId(0); // 创建协程时分配 id
thread_local int FiberControl::fiberCount(0);
thread_local bool FiberControl::debug = false; // 正确定义！

inline void FiberControl::set_running_fiber(std::shared_ptr<Fiber> fiber) {
    FiberControl::runningFiber = fiber;
}


std::shared_ptr<Fiber> FiberControl::get_running_fiber() { // 首先运行该函数会创建主协程
    if (!FiberControl::runningFiber) {
        first_create_fiber();
    }
    return FiberControl::runningFiber;
}

inline void FiberControl::first_create_fiber() {
    // 这里会调用构造函数，构造函数会调用 set_running_fiber 设置 runningFiber（构造函数是不是应该调用 set_runninng_fiber？）
    /// TODO: 待优化，组织代码结构。设计模式：工厂模式？
    std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>();
    FiberControl::set_running_fiber(fiber); // what():  bad_weak_ptr。构造函数里面无法调用 set_running_fiber。

    assert(FiberControl::runningFiber == fiber);
    FiberControl::mainFiber = fiber;
    FiberControl::schedulerFiber = fiber; // 除非主动设置，主协程默认为调度协程

    FiberControl::fiberId = FiberControl::get_fiber_id();
    ++FiberControl::fiberCount;
}


inline void FiberControl::set_scheduler_fiber(std::shared_ptr<Fiber> fiber) {
    FiberControl::schedulerFiber = fiber;
}


inline uint64_t FiberControl::get_fiber_id() {
    if (FiberControl::runningFiber) { // 我在想是否需要调用 init_fiber_control
        return FiberControl::runningFiber->get_id();
    }
    return UINT64_MAX; // uint64_t(-1)
}

}