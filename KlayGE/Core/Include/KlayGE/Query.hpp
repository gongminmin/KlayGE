// Query.hpp
// KlayGE 查询抽象类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 增加了ConditionalRender::AnySamplesPassed (2010.4.3)
//
// 3.8.0
// 增加ConditionalRender (2008.10.11)
//
// 3.0.0
// 初次建立 (2005.10.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _QUERY_HPP
#define _QUERY_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Query
	{
	public:
		virtual ~Query()
		{
		}

		static QueryPtr NullObject();

		virtual void Begin() = 0;
		virtual void End() = 0;
	};

	class KLAYGE_CORE_API OcclusionQuery : public Query
	{
	public:
		virtual ~OcclusionQuery()
		{
		}

		virtual uint64_t SamplesPassed() = 0;
	};

	class KLAYGE_CORE_API ConditionalRender : public Query
	{
	public:
		virtual ~ConditionalRender()
		{
		}

		virtual void BeginConditionalRender() = 0;
		virtual void EndConditionalRender() = 0;

		virtual bool AnySamplesPassed() = 0;
	};
}

#endif		// _QUERY_HPP
