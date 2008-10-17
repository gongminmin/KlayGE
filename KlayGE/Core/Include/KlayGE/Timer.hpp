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

#include <limits>
#ifdef KLAYGE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <ctime>
	#include <limits>
#endif

namespace KlayGE
{
	class KLAYGE_CORE_API Timer
	{
	public:
		Timer()
		{
			if (0 == cps_)
			{
#ifdef KLAYGE_PLATFORM_WINDOWS
				LARGE_INTEGER frequency;
				QueryPerformanceFrequency(&frequency);
				cps_ = static_cast<uint64_t>(frequency.QuadPart);
#else
				cps_ = CLOCKS_PER_SEC;
#endif
			}

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
			return static_cast<double>(std::numeric_limits<uint64_t>::max()) / cps_ - start_time_;
#else
			return static_cast<double>(std::numeric_limits<std::clock_t>::max()) / cps_ - start_time_;
#endif
		}

		// return minimum value for elapsed()
		double elapsed_min() const
		{
			return 1.0 / cps_;
		}

		double current_time() const
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			LARGE_INTEGER count;
			QueryPerformanceCounter(&count);
			return static_cast<double>(count.QuadPart) / cps_;
#else
			return static_cast<double>(std::clock()) / cps_;
#endif
		}

	private:
		double start_time_;

		static uint64_t cps_;
	};
}

#endif		// _TIMER_HPP
