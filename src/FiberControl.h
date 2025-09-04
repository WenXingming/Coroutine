/**
 * @file FiberControl.h
 * @brief 线程内部协程的控制
 * @details 主要是维护当前线程上的协程控制信息，方便进行线程内部协程的控制例如协程间的切换、后续调度
 * @author wenxingming
 * @date 2025-08-31
 * @note My project address: https://github.com/WenXingming/Coroutine
 * @cite https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */

#pragma once
#include <memory>
#include <cassert>
#include <functional>

namespace wxm {

	// 循环依赖的解决：头文件和源文件分离，并使用前向声明
	class Fiber;

	class FiberControl {
	private:
		static thread_local std::shared_ptr<Fiber> runningFiber; // 运行中的协程
		static thread_local std::shared_ptr<Fiber> mainFiber; // 主协程
		static thread_local std::shared_ptr<Fiber> schedulerFiber; // 调度协程
		static thread_local uint32_t threadFiberCount; // 全局协程计数器
		static thread_local uint64_t threadFiberId; // 获取协程 id
		static thread_local bool debug; // 是否打印 debug 信息

		// 私有函数。如果没有协程，则调用此函数创建主协程（其会创建主协程，并初始化线程中协程的 FiberControl 信息）
		static void first_create_fiber();

	public:
		// 此方法创建子协程。工厂模式：Fiber 构造函数私有化，使用 FiberControl 接口进行 Fiber 创建（友元类）
		static std::shared_ptr<Fiber> create_fiber(std::function<void()> _cb, size_t _stacksize = 0, bool _run_in_scheduler = true);

		static std::shared_ptr<Fiber> get_running_fiber();
		static std::shared_ptr<Fiber> get_main_fiber();
		static std::shared_ptr<Fiber> get_scheduler_fiber();
		static void set_running_fiber(std::shared_ptr<Fiber> f);
		static void set_main_fiber(std::shared_ptr<Fiber> f);
		static void set_scheduler_fiber(std::shared_ptr<Fiber> f);

		static uint32_t get_thread_fiber_count();
		static void set_thread_fiber_count(uint32_t val);
		static uint64_t get_thread_fiber_id();
		static void set_thread_fiber_id(uint64_t val);

		static bool get_debug();
	};


}