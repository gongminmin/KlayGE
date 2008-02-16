// CpuTopology.hpp
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

#ifndef _CPU_TOPOLOGY_HPP
#define _CPU_TOPOLOGY_HPP

namespace KlayGE
{
	class CpuTopology
	{
	public:
		explicit CpuTopology(bool force_cpuid = false);

		int NumHWThreads() const;
		int NumCores() const;
	};
}

#endif  // _CPU_TOPOLOGY_HPP
