// Timer.cpp
// KlayGE 定时器类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.7.0
// 初次建立 (2008.1.8)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <limits>

#include <KlayGE/Timer.hpp>

namespace KlayGE
{
	uint64_t CPS()
	{
		static uint64_t cps = 0;
		if (0 == cps)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			cps = static_cast<uint64_t>(frequency.QuadPart);
#else
			cps = CLOCKS_PER_SEC;
#endif
		}

		return cps;
	}		

	Timer::Timer()
	{
		this->restart();
	} // postcondition: elapsed()==0

	void Timer::restart()
	{
		start_time_ = this->current_time();
	} // postcondition: elapsed()==0

	// return elapsed time in seconds
	double Timer::elapsed() const
	{
		return this->current_time() - start_time_;
	}

	// return estimated maximum value for elapsed()
	double Timer::elapsed_max() const
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		return static_cast<double>(std::numeric_limits<uint64_t>::max()) / CPS() - start_time_;
#else
		return static_cast<double>(std::numeric_limits<std::clock_t>::max()) / CPS() - start_time_;
#endif
	}

	// return minimum value for elapsed()
	double Timer::elapsed_min() const
	{
		return 1.0 / CPS();
	}

	double Timer::current_time() const
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return static_cast<double>(count.QuadPart) / CPS();
#else
		return static_cast<double>(std::clock()) / CPS();
#endif
	}
}
