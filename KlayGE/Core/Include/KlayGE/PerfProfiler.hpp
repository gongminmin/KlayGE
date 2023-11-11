/**
 * @file PerfProfiler.hpp
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

#ifndef KLAYGE_CORE_PERF_PROFILER_HPP
#define KLAYGE_CORE_PERF_PROFILER_HPP

#pragma once

#include <KFL/Noncopyable.hpp>
#include <KFL/Timer.hpp>

#include <map>
#include <string>
#include <tuple>

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API PerfRegion final
	{
		KLAYGE_NONCOPYABLE(PerfRegion);

	public:
		PerfRegion();

		void Begin();
		void End();

		void CollectData();

		double CpuTime() const noexcept
		{
			return cpu_time_;
		}
		double GpuTime() const noexcept
		{
			return gpu_time_;
		}
		bool Dirty() const noexcept
		{
			return dirty_;
		}

	private:
		Timer cpu_timer_;
		QueryPtr gpu_timer_query_;

		double cpu_time_ = 0;
		double gpu_time_ = 0;

		bool dirty_ = false;
	};

	class KLAYGE_CORE_API PerfProfiler final
	{
		KLAYGE_NONCOPYABLE(PerfProfiler);

	public:
		PerfProfiler();

		static PerfProfiler& Instance();
		static void Destroy();

		void Suspend();
		void Resume();

		PerfRegion* CreatePerfRegion(int category, std::string const& name);
		void CollectData();

		void ExportToCSV(std::string const& file_name) const;

	private:
		static std::unique_ptr<PerfProfiler> perf_profiler_instance_;

		struct FramePerfInfo
		{
			uint32_t frame_id;
			double cpu_time;
			double gpu_time;
		};

		struct PerfInfo
		{
			int category;
			std::string name;
			std::unique_ptr<PerfRegion> perf_region;
			std::vector<FramePerfInfo> frames;
		};

		std::vector<PerfInfo> perf_regions_;
		uint32_t frame_id_ = 0;
	};
} // namespace KlayGE

#endif // KLAYGE_CORE_PERF_PROFILER_HPP
