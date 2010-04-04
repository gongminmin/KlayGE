// D3D9Query.hpp
// KlayGE D3D9查询类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2005-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 加入ConditionalRender (2008.10.11)
//
// 3.0.0
// 初次建立 (2005.9.27)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9OCCLUSIONQUERY_HPP
#define _D3D9OCCLUSIONQUERY_HPP

#pragma once

#include <KlayGE/Query.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

namespace KlayGE
{
	class D3D9OcclusionQuery : public OcclusionQuery, public D3D9Resource
	{
	public:
		D3D9OcclusionQuery();

		void Begin();
		void End();

		uint64_t SamplesPassed();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		ID3D9QueryPtr query_;
	};
	typedef boost::shared_ptr<D3D9OcclusionQuery> D3D9OcclusionQueryPtr;

	class D3D9ConditionalRender : public ConditionalRender, public D3D9Resource
	{
	public:
		D3D9ConditionalRender();

		void Begin();
		void End();

		void BeginConditionalRender();
		void EndConditionalRender();

		bool AnySamplesPassed();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		ID3D9QueryPtr query_;
	};
	typedef boost::shared_ptr<D3D9ConditionalRender> D3D9ConditionalRenderPtr;
}

#endif		// _D3D9QUERY_HPP
