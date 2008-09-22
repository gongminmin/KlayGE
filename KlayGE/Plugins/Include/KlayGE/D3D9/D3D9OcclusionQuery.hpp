// D3D9OcclusionQuery.hpp
// KlayGE D3D9遮挡检测类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.27)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9OCCLUSIONQUERY_HPP
#define _D3D9OCCLUSIONQUERY_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/OcclusionQuery.hpp>

namespace KlayGE
{
	class D3D9OcclusionQuery : public OcclusionQuery
	{
	public:
		D3D9OcclusionQuery();

		void Begin();
		void End();

		uint64_t SamplesPassed();

	private:
		ID3D9QueryPtr query_;
	};
}

#endif		// _D3D9OCCLUSIONQUERY_HPP
