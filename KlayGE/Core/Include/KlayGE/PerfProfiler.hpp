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

#ifndef _KLAYGE_PERFPROFILER_HPP
#define _KLAYGE_PERFPROFILER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Timer.hpp>

#include <map>
#include <string>
#include <tuple>

namespace KlayGE
{
	class KLAYGE_CORE_API PerfRange final : boost::noncopyable
	{
	public:
		PerfRange();

		void Begin();
		void End();

		void CollectData();

		double CPUTime() const;
		double GPUTime() const;
		bool Dirty() const;

	private:
		Timer cpu_timer_;
		QueryPtr gpu_timer_query_;

		double cpu_time_;
		double gpu_time_;

		bool dirty_;
	};

	class KLAYGE_CORE_API PerfProfiler final : boost::noncopyable
	{
	public:
		PerfProfiler();

		static PerfProfiler& Instance();
		static void Destroy();

		void Suspend();
		void Resume();

		PerfRangePtr CreatePerfRange(int category, std::string const & name);
		void CollectData();

		void ExportToCSV(std::string const & file_name) const;

	private:
		static std::unique_ptr<PerfProfiler> perf_profiler_instance_;

		std::vector<std::tuple<int, std::string, PerfRangePtr,
			std::vector<std::tuple<uint32_t, double, double>>>> perf_ranges_;
		uint32_t frame_id_;
	};
}

#endif			// _KLAYGE_PERFPROFILER_HPP
