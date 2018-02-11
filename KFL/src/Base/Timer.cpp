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

#include <KFL/Timer.hpp>

namespace KlayGE
{
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
		return std::chrono::duration<double>::max().count();
	}

	// return minimum value for elapsed()
	double Timer::elapsed_min() const
	{
		return std::chrono::duration<double>::min().count();
	}

	double Timer::current_time() const
	{
		std::chrono::high_resolution_clock::time_point const tp = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(tp.time_since_epoch()).count();
	}
}
