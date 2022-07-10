// Query.hpp
// KlayGE ��ѯ������ ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ������ConditionalRender::AnySamplesPassed (2010.4.3)
//
// 3.8.0
// ����ConditionalRender (2008.10.11)
//
// 3.0.0
// ���ν��� (2005.10.18)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _QUERY_HPP
#define _QUERY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Query : boost::noncopyable
	{
	public:
		virtual ~Query() noexcept;

		virtual void Begin() = 0;
		virtual void End() = 0;
	};

	class KLAYGE_CORE_API OcclusionQuery : public Query
	{
	public:
		virtual uint64_t SamplesPassed() = 0;
	};

	class KLAYGE_CORE_API ConditionalRender : public Query
	{
	public:
		virtual void BeginConditionalRender() = 0;
		virtual void EndConditionalRender() = 0;

		virtual bool AnySamplesPassed() = 0;
	};

	class KLAYGE_CORE_API TimerQuery : public Query
	{
	public:
		virtual double TimeElapsed() = 0; // In second.
	};

	class KLAYGE_CORE_API SOStatisticsQuery : public Query
	{
	public:
		virtual uint64_t NumPrimitivesWritten() = 0;
		virtual uint64_t PrimitivesGenerated() = 0;
	};
}

#endif		// _QUERY_HPP
