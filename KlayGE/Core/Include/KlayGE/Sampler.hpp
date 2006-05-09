// Sampler.hpp
// KlayGE 渲染样本类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Math.hpp>

#include <string>
#include <boost/assert.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class Sampler
	{
	public:
		enum TexFilterOp
		{
			TFO_None,
			TFO_Point,
			TFO_Bilinear,
			TFO_Trilinear,
			TFO_Anisotropic
		};

		enum TexAddressingType
		{
			TAT_Addr_U,
			TAT_Addr_V,
			TAT_Addr_W
		};

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

	public:
		Sampler();

		void SetTexture(TexturePtr tex);
		TexturePtr GetTexture() const;

		void BorderColor(Color const & clr);
		Color const & BorderColor() const;

		// Sets the texture addressing mode for a texture unit.
		void AddressingMode(TexAddressingType type, TexAddressingMode tam);
		// Sets the texture filtering type for a texture unit.
		void Filtering(TexFilterOp op);
		// Sets the maximal anisotropy for the specified texture unit.
		void Anisotropy(uint32_t maxAnisotropy);

		TexAddressingMode AddressingMode(TexAddressingType type) const;
		TexFilterOp Filtering() const;
		uint32_t Anisotropy() const;

		void MaxMipLevel(uint32_t level);
		uint32_t MaxMipLevel() const;

		void MipMapLodBias(float bias);
		float MipMapLodBias() const;

	private:
		TexturePtr tex_;

		Color border_clr_;

		TexAddressingMode addr_mode_u_, addr_mode_v_, addr_mode_w_;
		TexFilterOp filter_;
		uint32_t anisotropy_;
		uint32_t max_mip_level_;
		float mip_map_lod_bias_;
	};
}

#endif			// _SAMPLER_HPP
