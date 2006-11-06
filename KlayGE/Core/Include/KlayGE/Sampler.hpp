// Sampler.hpp
// KlayGE 渲染样本类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 改为使用结构体 (2006.11.5)
//
// 3.2.0
// 去掉了TextureMatrix (2006.5.9)
//
// 3.0.0
// 增加了TAM_Border (2005.8.30)
//
// 2.8.0
// 初次建立 (2005.7.30)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SAMPLER_HPP
#define _SAMPLER_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Math.hpp>

#include <string>
#include <boost/assert.hpp>

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif
	struct Sampler
	{
		// Sampler addressing modes - default is TAM_Wrap.
		enum TexAddressingMode
		{
			// Texture wraps at values over 1.0
			TAM_Wrap,
			// Texture mirrors (flips) at joins over 1.0
			TAM_Mirror,
			// Texture clamps at 1.0
			TAM_Clamp,
			// Texture coordinates outside the range [0.0, 1.0] are set to the border color.
			TAM_Border
		};

		enum TexFilterOp
		{
			TFO_None = 1UL << 0,
			TFO_Point = 1UL << 1,
			TFO_Bilinear = 1UL << 2,
			TFO_Trilinear = 1UL << 3,
			TFO_Anisotropic = 1UL << 4
		};

		TexturePtr texture;

		Color border_clr;

		TexAddressingMode addr_mode_u : 3;
		TexAddressingMode addr_mode_v : 3;
		TexAddressingMode addr_mode_w : 3;

		TexFilterOp filter : 7;

		uint8_t anisotropy;
		uint8_t max_mip_level;
		float mip_map_lod_bias;

		Sampler();
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif
}

#endif			// _SAMPLER_HPP
