// D3D11Query.hpp
// KlayGE D3D11查询类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11QUERY_HPP
#define _D3D11QUERY_HPP

#pragma once

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class D3D11Query
	{
	public:
		D3D11Query();
		virtual ~D3D11Query() noexcept;

		void SignalFence();
		void WaitForFence();

	protected:
		FencePtr fence_;
		uint64_t fence_val_;
	};

	class D3D11OcclusionQuery final : public OcclusionQuery, public D3D11Query
	{
	public:
		D3D11OcclusionQuery();

		void Begin() override;
		void End() override;

		uint64_t SamplesPassed() override;

	private:
		ID3D11QueryPtr query_;
	};

	class D3D11ConditionalRender final : public ConditionalRender, public D3D11Query
	{
	public:
		D3D11ConditionalRender();

		void Begin() override;
		void End() override;

		void BeginConditionalRender() override;
		void EndConditionalRender() override;

		bool AnySamplesPassed() override;

	private:
		ID3D11PredicatePtr predicate_;
	};

	class D3D11TimerQuery final : public TimerQuery, public D3D11Query
	{
	public:
		D3D11TimerQuery();

		void Begin() override;
		void End() override;

		double TimeElapsed() override;

	private:
		ID3D11QueryPtr timestamp_disjoint_query_;
		ID3D11QueryPtr timestamp_start_query_;
		ID3D11QueryPtr timestamp_end_query_;
	};

	class D3D11SOStatisticsQuery final : public SOStatisticsQuery, public D3D11Query
	{
	public:
		D3D11SOStatisticsQuery();

		void Begin() override;
		void End() override;

		uint64_t NumPrimitivesWritten() override;
		uint64_t PrimitivesGenerated() override;

	private:
		ID3D11QueryPtr so_stat_query_;
	};
}

#endif		// _D3D11QUERY_HPP
