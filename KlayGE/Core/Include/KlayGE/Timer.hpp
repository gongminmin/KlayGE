// Timer.hpp
// KlayGE 定时器类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2005.12.30)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _TIMER_HPP
#define _TIMER_HPP

#ifdef KLAYGE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <ctime>
	#include <limits>
#endif

namespace KlayGE
{
	class Timer
	{
	public:
		Timer()
		{
			this->restart();
		} // postcondition: elapsed()==0
		void restart()
		{
			start_time_ = this->current_time();
		} // postcondition: elapsed()==0

		// return elapsed time in seconds
		double elapsed() const
		{
			return this->current_time() - start_time_;
		}

		// return estimated maximum value for elapsed()
		double elapsed_max() const   
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			return static_cast<double>(std::numeric_limits<uint64_t>::max()) / this->clocks_per_sec() - start_time_;
#else
			return static_cast<double>(std::numeric_limits<std::clock_t>::max()) / this->clocks_per_sec() - start_time_;
#endif
		}

		// return minimum value for elapsed()
		double elapsed_min() const
		{
			return 1.0 / this->clocks_per_sec();
		}

	private:
		double current_time() const
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			LARGE_INTEGER count;
			QueryPerformanceCounter(&count);
			return static_cast<double>(count.QuadPart) / this->clocks_per_sec();
#else
			return std::clock() / this->clocks_per_sec();
#endif
		}

		uint64_t clocks_per_sec() const
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			return static_cast<uint64_t>(frequency.QuadPart);
#else
			return CLOCKS_PER_SEC;
#endif
		}

	private:
		double start_time_;
	};
}

#endif		// _TIMER_HPP
