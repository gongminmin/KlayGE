/**
 * @file Timer.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KFL/KFL.hpp>

#include <limits>
#include <chrono>
#ifdef KLAYGE_COMPILER_MSVC
#include <windows.h>
#endif

#include <KFL/Timer.hpp>

namespace KlayGE
{
#if defined(KLAYGE_COMPILER_MSVC) && (KLAYGE_COMPILER_VERSION < 140)
	// In vc11 and vc12, a system_clock is synonymous with a high_resolution_clock.
	//  So using QueryPerformance* for high resolution timing.
	#define USE_QUERY_PERFORMANCE
#endif

#ifdef USE_QUERY_PERFORMANCE
	uint64_t CPS()
	{
		static uint64_t cps = 0;
		if (0 == cps)
		{
			::LARGE_INTEGER frequency;
			::QueryPerformanceFrequency(&frequency);
			cps = static_cast<uint64_t>(frequency.QuadPart);
		}

		return cps;
	}
#endif

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
#ifdef USE_QUERY_PERFORMANCE
		return static_cast<double>(std::numeric_limits<uint64_t>::max()) / CPS() - start_time_;
#else
		return std::chrono::duration<double>::max().count();
#endif
	}

	// return minimum value for elapsed()
	double Timer::elapsed_min() const
	{
#ifdef USE_QUERY_PERFORMANCE
		return 1.0 / CPS();
#else
		return std::chrono::duration<double>::min().count();
#endif
	}

	double Timer::current_time() const
	{
#ifdef USE_QUERY_PERFORMANCE
		::LARGE_INTEGER count;
		::QueryPerformanceCounter(&count);
		return static_cast<double>(count.QuadPart) / CPS();
#else
		std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(tp.time_since_epoch()).count();
#endif
	}
}
