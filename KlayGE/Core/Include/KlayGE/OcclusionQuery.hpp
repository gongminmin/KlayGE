// OcclusionQuery.hpp
// KlayGE 遮挡查询类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.27)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OCCLUSIONQUERY_HPP
#define _OCCLUSIONQUERY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Query.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class OcclusionQuery : public Query
	{
	public:
		virtual ~OcclusionQuery()
		{
		}

		virtual uint32_t SamplesPassed() = 0;
	};
}

#endif		// _OCCLUSIONQUERY_HPP
