/**
 * @file FiberControl.cpp
 * @brief 线程内部协程的控制
 * @details
 * 主要是维护当前线程上的协程控制信息，方便进行线程内部协程的控制例如协程间的切换、后续调度
 * @author wenxingming
 * @date 2025-08-31
 * @note My project address:
 * @cite
 * https://github.com/youngyangyang04/coroutine-lib/tree/main/fiber_lib/2fiber
 */

#include <memory>
#include "Fiber.h" // 可以都包含
#include "FiberControl.h"
namespace wxm {


	/// @brief 静态成员变量不像成员变量可以在定义对象时初始化，必须手动初始化！生命周期与程序的生命周期相同。注意，这里没有 "static"，因为它的作用域属性在类中已经声明了
	thread_local std::shared_ptr<Fiber> FiberControl::runningFiber(nullptr);
	thread_local std::shared_ptr<Fiber> FiberControl::mainFiber(nullptr);
	thread_local std::shared_ptr<Fiber> FiberControl::schedulerFiber(nullptr);
	thread_local int FiberControl::threadFiberCount(0);
	thread_local bool FiberControl::debug = true;



	void FiberControl::first_create_fiber() {
		// Fiber() 私有，make_shared<T>() 无法访问！编译错误
		// std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(); 
		std::shared_ptr<Fiber> fiber(new Fiber());
		FiberControl::set_running_fiber(fiber);
		FiberControl::set_main_fiber(fiber);
		FiberControl::set_scheduler_fiber(fiber); // 除非主动设置，主协程默认为调度协程

		assert(FiberControl::runningFiber == fiber);
		assert(FiberControl::mainFiber == fiber);
		assert(FiberControl::schedulerFiber == fiber);

		int threadFiberCount = get_thread_fiber_count();
		FiberControl::set_thread_fiber_count(++threadFiberCount);
	}


	std::shared_ptr<Fiber> FiberControl::create_fiber(std::function<void()> _cb, size_t _stacksize, bool _run_in_scheduler) {
		if (!FiberControl::runningFiber) {
			first_create_fiber();
		}
		// std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(_cb, _stacksize, _run_in_scheduler); // 构造函数私有，make_shared<>() 无法访问！编译错误
		std::shared_ptr<Fiber> fiber(new Fiber(_cb, _stacksize, _run_in_scheduler));
		auto id = get_thread_fiber_count();
		set_thread_fiber_count(id + 1);
		return fiber;
	}


	std::shared_ptr<Fiber> FiberControl::get_running_fiber() { // 首先运行该函数会创建主协程
		if (!FiberControl::runningFiber) {
			first_create_fiber();
		}
		return FiberControl::runningFiber;
	}


	std::shared_ptr<Fiber> FiberControl::get_main_fiber() {
		if (!FiberControl::mainFiber) {
			first_create_fiber();
		}
		return FiberControl::mainFiber;
	}


	std::shared_ptr<Fiber> FiberControl::get_scheduler_fiber() {
		if (!FiberControl::schedulerFiber) {
			first_create_fiber();
		}
		return FiberControl::schedulerFiber;
	}


	void FiberControl::set_running_fiber(std::shared_ptr<Fiber> fiber) {
		FiberControl::runningFiber = fiber;
	}


	void FiberControl::set_main_fiber(std::shared_ptr<Fiber> fiber) {
		FiberControl::mainFiber = fiber;
	}


	void FiberControl::set_scheduler_fiber(std::shared_ptr<Fiber> fiber) {
		FiberControl::schedulerFiber = fiber;
	}


	int FiberControl::get_thread_fiber_count() {
		return FiberControl::threadFiberCount;
	}


	void FiberControl::set_thread_fiber_count(int val) {
		FiberControl::threadFiberCount = val;
	}


	bool FiberControl::get_debug() {
		return FiberControl::debug;
	}


}