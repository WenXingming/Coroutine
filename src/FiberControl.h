/**
 * @file FiberControl.h
 * @brief 线程内部协程的控制
 * @details 主要是维护当前线程上的协程控制信息，方便进行线程内部协程的控制例如协程间的切换、后续调度
 * @author wenxingming
 * @date 2025-08-31
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
private:
	static thread_local std::shared_ptr<Fiber> runningFiber; // 运行中的协程
	static thread_local std::shared_ptr<Fiber> mainFiber; // 主协程
	static thread_local std::shared_ptr<Fiber> schedulerFiber; // 调度协程
	static thread_local int threadFiberCount;		// 全局协程 ID 计数器
	static thread_local bool debug;

public:
	// 如果没有协程，则调用此函数创建协程（并初始化线程中协程的 FiberControl 信息）
	static void first_create_fiber(); 
	// 工厂模式。Fiber 构造函数私有化，使用 FiberControl 接口进行 Fiber 创建
	static std::shared_ptr<Fiber> create_fiber(std::function<void()> _cb, size_t _stacksize = 0, bool _run_in_scheduler = true);

	static std::shared_ptr<Fiber> get_running_fiber();
	static std::shared_ptr<Fiber> get_main_fiber();
	static std::shared_ptr<Fiber> get_scheduler_fiber();
	static void set_running_fiber(std::shared_ptr<Fiber> f);
	static void set_main_fiber(std::shared_ptr<Fiber> f);
	static void set_scheduler_fiber(std::shared_ptr<Fiber> f);

	static int get_thread_fiber_count();
	static void set_thread_fiber_count(int val);

	static bool get_debug();
};



}