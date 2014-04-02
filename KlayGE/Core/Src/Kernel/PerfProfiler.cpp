/**
* @file PerfProfiler.cpp
* @author Minmin Gong
*
* @section DESCRIPTION
*
* This source file is part of KlayGE
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

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Query.hpp>

#include <fstream>

#include <KlayGE/PerfProfiler.hpp>

namespace KlayGE
{
	PerfRange::PerfRange()
		: cpu_time_(0), gpu_time_(0), dirty_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		gpu_timer_query_ = rf.MakeTimerQuery();
	}

	void PerfRange::Begin()
	{
		dirty_ = true;
		cpu_timer_.restart();
		gpu_timer_query_->Begin();
	}

	void PerfRange::End()
	{
		cpu_time_ = cpu_timer_.elapsed();
		gpu_timer_query_->End();
	}

	void PerfRange::CollectData()
	{
		if (dirty_)
		{
			gpu_time_ = checked_pointer_cast<TimerQuery>(gpu_timer_query_)->TimeElapsed();
			dirty_ = false;
		}
	}

	double PerfRange::CPUTime() const
	{
		return cpu_time_;
	}

	double PerfRange::GPUTime() const
	{
		return gpu_time_;
	}


	PerfRangePtr PerfProfiler::CreatePerfRange(int category, std::string const & name)
	{
		PerfRangePtr range = MakeSharedPtr<PerfRange>();
		perf_ranges_.insert(std::make_pair(std::make_pair(category, name), range));
		return range;
	}

	void PerfProfiler::CollectData()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		re.UpdateGPUTimestampsFrequency();

		typedef KLAYGE_DECLTYPE(perf_ranges_) PerfRangesType;
		KLAYGE_FOREACH(PerfRangesType::reference range, perf_ranges_)
		{
			range.second->CollectData();
		}
	}

	void PerfProfiler::ExportToCSV(std::string const & file_name)
	{
		std::ofstream ofs(file_name.c_str());
		ofs << "Category" << ',' << "Name" << ','
			<< "CPU Timing (ms)" << ',' << "GPU Timing (ms)" << std::endl;

		typedef KLAYGE_DECLTYPE(perf_ranges_) PerfRangesType;
		KLAYGE_FOREACH(PerfRangesType::reference range, perf_ranges_)
		{
			ofs << range.first.first << ',' << range.first.second << ','
				<< range.second->CPUTime() * 1000 << ',' << range.second->GPUTime() * 1000
				<< std::endl;
		}

		ofs << std::endl;
	}
}
