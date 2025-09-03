/**
 * @file Fiber.h
 * @brief 协程类。Declaration of Fiber class
 * @details 自定义实现 C++ 协程类
 * @author wenxingming
 * @date 2025-08-31
 * @note My project address:
 * @cite https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */

#pragma once
#include <ucontext.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <mutex>
#include <cassert>
namespace wxm {

    // 头文件不可相互包含，如何解决循环依赖问题？声明、实现分离（.h、.cpp），并在循环依赖的头文件中使用前向声明（源文件可以直接包含两个头文件声明）
    class FiberControl;
    
    class Fiber : public std::enable_shared_from_this<Fiber> { // 允许一个类（Fiber）的对象安全地获取一个指向自身的 std::shared_ptr
    private:
        enum State {  // 协程状态：准备、运行、结束
            READY, RUNNING, TERM
        };

        uint64_t id;                    // 协程的唯一标识符
        ucontext_t context;             // 协程的上下文 context
        void* stackPtr;                 // 协程栈的指针
        uint32_t stackSize;             // 栈的大小
        std::function<void()> task;     // 协程的执行函数（回调函数？）
        State state = READY;            // 协程状态
        bool runInScheduler;            // 是否让出执行权交给调度协程

        friend class FiberControl; // FiberControl 需要调用 Fiber 的（私有）构造函数构造 Fiber。（工厂模式）
        Fiber(uint64_t id);
        Fiber(uint64_t _id, std::function<void()> _cb, size_t _stacksize = 0, bool _run_in_scheduler = true);

    public:
        std::mutex mutex;

    public:
        // 没写移动构造且定义了构造函数，不会默认生成移动构造。std::move() 时退化到拷贝构造（浅拷贝...）。
        // 这里拷贝构造又被禁用了，可能直接导致编译错误。干脆直接禁用了吧...用智能指针管理即可
        Fiber(const Fiber& other) = delete;
        Fiber& operator=(const Fiber& other) = delete;
        Fiber(const Fiber&& other) = delete;
        Fiber& operator=(const Fiber&& other) = delete;
        ~Fiber();

        void reset(std::function<void()> _cb); // 重用一个协程
        void resume();
        void yield();

        uint64_t get_id() const;
        State get_state() const;

        static void main_func();
    };


}
