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

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class OcclusionQuery : public Query
	{
	public:
		virtual ~OcclusionQuery()
		{
		}

		virtual uint64_t SamplesPassed() = 0;
	};
}

#endif		// _OCCLUSIONQUERY_HPP
