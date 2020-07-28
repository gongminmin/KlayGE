/**
 * @file CpuInfo.hpp
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

#ifndef _KFL_CPUINFO_HPP
#define _KFL_CPUINFO_HPP

#pragma once

#include <string>

namespace KlayGE
{
	class CPUInfo final
	{
	public:
		enum CPUFeature
		{
			CF_HTT = 1UL << 0,
			CF_MMX = 1UL << 1,
			CF_SSE = 1UL << 2,
			CF_SSE2 = 1UL << 3,
			CF_SSE3 = 1UL << 4,
			CF_SSSE3 = 1UL << 5,
			CF_SSE41 = 1UL << 6,
			CF_SSE42 = 1UL << 7,
			CF_SSE4A = 1UL << 8,
			CF_MisalignedSSE = 1UL << 9,
			CF_X64 = 1UL << 10,
			CF_FMA3 = 1UL << 11,
			CF_MOVBE = 1UL << 12,
			CF_POPCNT = 1UL << 13,
			CF_AES = 1UL << 14,
			CF_AVX = 1UL << 15,
			CF_LZCNT = 1UL << 16,
			CF_AVX2 = 1UL << 17,
			CF_FMA4 = 1UL << 18,
			CF_F16C = 1UL << 19
		};

	public:
		CPUInfo();

		std::string const & CPUString() const
		{
			return cpu_string_;
		}
		std::string const & CPUBrandString() const
		{
			return cpu_brand_string_;
		}

		bool IsFeatureSupport(CPUFeature feature) const
		{
			return feature_mask_ & feature ? true : false;
		}

		int NumHWThreads() const
		{
			return num_hw_threads_;
		}
		int NumCores() const
		{
			return num_cores_;
		}

	private:
		std::string cpu_string_;
		std::string cpu_brand_string_;
		uint64_t feature_mask_{0};

		int num_hw_threads_;
		int num_cores_;
	};
}

#endif  // _KFL_CPUINFO_HPP
