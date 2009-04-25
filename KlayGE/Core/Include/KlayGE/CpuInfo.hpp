// CpuInfo.hpp
// KlayGE CPU检测类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.2.16)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CPU_INFO_HPP
#define _CPU_INFO_HPP

#pragma once

#include <string>

namespace KlayGE
{
	class KLAYGE_CORE_API CPUInfo
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
		uint64_t feature_mask_;

		int num_hw_threads_;
		int num_cores_;
	};
}

#endif  // _CPU_INFO_HPP
