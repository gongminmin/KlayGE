// Timer.hpp
// KlayGE 定时器类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
//
// 3.2.0
// 初次建立 (2005.12.30)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _TIMER_HPP
#define _TIMER_HPP

#pragma once

#ifdef KLAYGE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <ctime>
#endif

namespace KlayGE
{
	class KLAYGE_CORE_API Timer
	{
	public:
		Timer(); // postcondition: elapsed()==0
		void restart(); // postcondition: elapsed()==0

		// return elapsed time in seconds
		double elapsed() const;

		// return estimated maximum value for elapsed()
		double elapsed_max() const;

		// return minimum value for elapsed()
		double elapsed_min() const;

		double current_time() const;

	private:
		double start_time_;
	};
}

#endif		// _TIMER_HPP
