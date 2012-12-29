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

#include <KFL/Timer.hpp>

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
