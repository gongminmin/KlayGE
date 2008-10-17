// D3D10Query.hpp
// KlayGE D3D10查询类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10OCCLUSIONQUERY_HPP
#define _D3D10OCCLUSIONQUERY_HPP

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class D3D10OcclusionQuery : public OcclusionQuery
	{
	public:
		D3D10OcclusionQuery();

		void Begin();
		void End();

		uint64_t SamplesPassed();

	private:
		ID3D10QueryPtr query_;
	};

	class D3D10ConditionalRender : public ConditionalRender
	{
	public:
		D3D10ConditionalRender();

		void Begin();
		void End();

		void BeginConditionalRender();
		void EndConditionalRender();

	private:
		ID3D10PredicatePtr predicate_;
	};
}

#endif		// _D3D10QUERY_HPP
