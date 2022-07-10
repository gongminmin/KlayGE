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

#include <KlayGE/Query.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

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

	PerfRegion::PerfRegion()
	{
		if (Context::Instance().Config().perf_profiler)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			gpu_timer_query_ = rf.MakeTimerQuery();
		}
	}

	void PerfRegion::Begin()
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

	void PerfRegion::End()
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

	void PerfRegion::CollectData()
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

	PerfRegion* PerfProfiler::CreatePerfRegion(int category, std::string const& name)
	{
		auto perf_region = MakeUniquePtr<PerfRegion>();
		auto* ret = perf_region.get();
		perf_regions_.emplace_back(PerfInfo{category, name, std::move(perf_region), {}});
		return ret;
	}

	void PerfProfiler::CollectData()
	{
		if (Context::Instance().Config().perf_profiler)
		{
			for (auto& region : perf_regions_)
			{
				auto& perf_region = *region.perf_region;
				if (perf_region.Dirty())
				{
					perf_region.CollectData();
					region.frames.emplace_back(FramePerfInfo{frame_id_, perf_region.CpuTime(), perf_region.GpuTime()});
				}
			}

			++frame_id_;
		}
	}

	void PerfProfiler::ExportToCSV(std::string const& file_name) const
	{
		if (Context::Instance().Config().perf_profiler)
		{
			std::ofstream ofs(file_name.c_str());
			ofs << "Frame" << ',' << "Category" << ',' << "Name" << ',' << "CPU Timing (ms)" << ',' << "GPU Timing (ms)\n";

			for (auto const& region : perf_regions_)
			{
				for (auto const& frame : region.frames)
				{
					ofs << frame.frame_id << ',' << region.category << ',' << region.name << ',' << frame.cpu_time * 1000 << ',';
					if (frame.gpu_time >= 0)
					{
						ofs << frame.gpu_time * 1000;
					}
					ofs << '\n';
				}
			}

			ofs << '\n';
		}
	}
} // namespace KlayGE
