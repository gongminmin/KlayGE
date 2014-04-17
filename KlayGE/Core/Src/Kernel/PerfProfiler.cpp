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
#include <KFL/Thread.hpp>

#include <fstream>

#include <KlayGE/PerfProfiler.hpp>

namespace
{
	KlayGE::mutex singleton_mutex;
}

namespace KlayGE
{
	shared_ptr<PerfProfiler> PerfProfiler::perf_profiler_instance_;

	PerfRange::PerfRange()
		: cpu_time_(0), gpu_time_(0), dirty_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		gpu_timer_query_ = rf.MakeTimerQuery();
	}

	void PerfRange::Begin()
	{
		if (Context::Instance().Config().perf_profiler_on)
		{
			dirty_ = true;
			cpu_timer_.restart();
			gpu_timer_query_->Begin();
		}
	}

	void PerfRange::End()
	{
		if (Context::Instance().Config().perf_profiler_on)
		{
			cpu_time_ = cpu_timer_.elapsed();
			gpu_timer_query_->End();
		}
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
			unique_lock<mutex> lock(singleton_mutex);
			if (!perf_profiler_instance_)
			{
				perf_profiler_instance_ = MakeSharedPtr<PerfProfiler>();
			}
		}
		return *perf_profiler_instance_;
	}

	void PerfProfiler::Destroy()
	{
		unique_lock<mutex> lock(singleton_mutex);
		perf_profiler_instance_.reset();
	}

	PerfRangePtr PerfProfiler::CreatePerfRange(int category, std::string const & name)
	{
		PerfRangePtr range = MakeSharedPtr<PerfRange>();
		typedef KLAYGE_DECLTYPE(get<3>(perf_ranges_[0])) PerfDataType;
		perf_ranges_.push_back(KlayGE::make_tuple(category, name, range, PerfDataType()));
		return range;
	}

	void PerfProfiler::CollectData()
	{
		if (Context::Instance().Config().perf_profiler_on)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();
			re.UpdateGPUTimestampsFrequency();

			typedef KLAYGE_DECLTYPE(perf_ranges_) PerfRangesType;
			KLAYGE_FOREACH(PerfRangesType::reference range, perf_ranges_)
			{
				if (get<2>(range)->Dirty())
				{
					get<2>(range)->CollectData();
					get<3>(range).push_back(KlayGE::make_tuple(frame_id_,
						get<2>(range)->CPUTime(), get<2>(range)->GPUTime()));
				}
			}

			++ frame_id_;
		}
	}

	void PerfProfiler::ExportToCSV(std::string const & file_name) const
	{
		if (Context::Instance().Config().perf_profiler_on)
		{
			std::ofstream ofs(file_name.c_str());
			ofs << "Frame" << ',' << "Category" << ',' << "Name" << ','
				<< "CPU Timing (ms)" << ',' << "GPU Timing (ms)" << std::endl;

			typedef KLAYGE_DECLTYPE(perf_ranges_) PerfRangesType;
			KLAYGE_FOREACH(PerfRangesType::const_reference range, perf_ranges_)
			{
				typedef KLAYGE_DECLTYPE(get<3>(range)) PerfDataType;
				KLAYGE_FOREACH(PerfDataType::const_reference data, get<3>(range))
				{
					ofs << get<0>(data) << ',' << get<0>(range) << ',' << get<1>(range) << ','
						<< get<1>(data) * 1000 << ',';
					if (get<2>(data) >= 0)
					{
						ofs << get<2>(data) * 1000;
					}
					ofs << std::endl;
				}
			}

			ofs << std::endl;
		}
	}
}
