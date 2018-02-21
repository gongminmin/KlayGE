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
#include <mutex>

#include <KlayGE/PerfProfiler.hpp>

namespace
{
	std::mutex singleton_mutex;
}

namespace KlayGE
{
	std::unique_ptr<PerfProfiler> PerfProfiler::perf_profiler_instance_;

	PerfRange::PerfRange()
		: cpu_time_(0), gpu_time_(0), dirty_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		gpu_timer_query_ = rf.MakeTimerQuery();
	}

	void PerfRange::Begin()
	{
		if (Context::Instance().Config().perf_profiler)
		{
			dirty_ = true;
			cpu_timer_.restart();
			if (gpu_timer_query_)
			{
				gpu_timer_query_->Begin();
			}
		}
	}

	void PerfRange::End()
	{
		if (Context::Instance().Config().perf_profiler)
		{
			cpu_time_ = cpu_timer_.elapsed();
			if (gpu_timer_query_)
			{
				gpu_timer_query_->End();
			}
		}
	}

	void PerfRange::CollectData()
	{
		if (dirty_)
		{
			if (gpu_timer_query_)
			{
				gpu_time_ = checked_pointer_cast<TimerQuery>(gpu_timer_query_)->TimeElapsed();
			}
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

	bool PerfRange::Dirty() const
	{
		return dirty_;
	}


	PerfProfiler::PerfProfiler()
		: frame_id_(0)
	{
	}

	PerfProfiler& PerfProfiler::Instance()
	{
		if (!perf_profiler_instance_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!perf_profiler_instance_)
			{
				perf_profiler_instance_ = MakeUniquePtr<PerfProfiler>();
			}
		}
		return *perf_profiler_instance_;
	}

	void PerfProfiler::Destroy()
	{
		std::lock_guard<std::mutex> lock(singleton_mutex);
		perf_profiler_instance_.reset();
	}

	void PerfProfiler::Suspend()
	{
	}

	void PerfProfiler::Resume()
	{
	}

	PerfRangePtr PerfProfiler::CreatePerfRange(int category, std::string const & name)
	{
		PerfRangePtr range = MakeSharedPtr<PerfRange>();
		typedef std::remove_reference<decltype(std::get<3>(perf_ranges_[0]))>::type PerfDataType;
		perf_ranges_.push_back(std::make_tuple(category, name, range, PerfDataType()));
		return range;
	}

	void PerfProfiler::CollectData()
	{
		if (Context::Instance().Config().perf_profiler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();
			re.UpdateGPUTimestampsFrequency();

			for (auto& range : perf_ranges_)
			{
				if (std::get<2>(range)->Dirty())
				{
					std::get<2>(range)->CollectData();
					std::get<3>(range).push_back(std::make_tuple(frame_id_,
						std::get<2>(range)->CPUTime(), std::get<2>(range)->GPUTime()));
				}
			}

			++ frame_id_;
		}
	}

	void PerfProfiler::ExportToCSV(std::string const & file_name) const
	{
		if (Context::Instance().Config().perf_profiler)
		{
			std::ofstream ofs(file_name.c_str());
			ofs << "Frame" << ',' << "Category" << ',' << "Name" << ','
				<< "CPU Timing (ms)" << ',' << "GPU Timing (ms)" << std::endl;

			for (auto const & range : perf_ranges_)
			{
				for (auto const & data : std::get<3>(range))
				{
					ofs << std::get<0>(data) << ',' << std::get<0>(range) << ',' << std::get<1>(range) << ','
						<< std::get<1>(data) * 1000 << ',';
					if (std::get<2>(data) >= 0)
					{
						ofs << std::get<2>(data) * 1000;
					}
					ofs << std::endl;
				}
			}

			ofs << std::endl;
		}
	}
}
