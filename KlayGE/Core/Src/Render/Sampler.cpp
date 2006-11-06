// Sampler.cpp
// KlayGE 渲染样本类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 去掉了TextureMatrix (2006.5.9)
//
// 2.8.0
// 初次建立 (2005.7.30)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <KlayGE/Sampler.hpp>

namespace KlayGE
{
	Sampler::Sampler()
		: border_clr(0, 0, 0, 0),
			addr_mode_u(TAM_Wrap), addr_mode_v(TAM_Wrap), addr_mode_w(TAM_Wrap),
			filter(TFO_Point),
			anisotropy(1),
			max_mip_level(1),
			mip_map_lod_bias(0)
	{
	}
}
