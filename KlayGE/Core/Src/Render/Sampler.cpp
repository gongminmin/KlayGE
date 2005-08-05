// Sampler.cpp
// KlayGE 渲染样本类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
		: border_clr_(0, 0, 0, 0),
			addr_mode_u_(TAM_Wrap), addr_mode_v_(TAM_Wrap), addr_mode_w_(TAM_Wrap),
			filter_(TFO_Point),
			anisotropy_(0),
			max_mip_level_(1),
			mip_map_lod_bias_(0),
			mat_(Matrix4::Identity())
	{
	}

	// 设置纹理
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::SetTexture(TexturePtr tex)
	{
		tex_ = tex;
	}

	// 获取纹理
	/////////////////////////////////////////////////////////////////////////////////
	TexturePtr Sampler::GetTexture() const
	{
		return tex_;
	}

	// 设置边界颜色
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::BorderColor(Color const & clr)
	{
		border_clr_ = clr;
	}
	
	// 获取边界颜色
	/////////////////////////////////////////////////////////////////////////////////
	Color const & Sampler::BorderColor() const
	{
		return border_clr_;
	}

	// 设置纹理寻址模式
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::AddressingMode(TexAddressingType type, TexAddressingMode tam)
	{
		switch (type)
		{
		case TAT_Addr_U:
			addr_mode_u_ = tam;
			break;

		case TAT_Addr_V:
			addr_mode_v_ = tam;
			break;

		case TAT_Addr_W:
			addr_mode_w_ = tam;
			break;
		}
	}

	// 获取纹理寻址模式
	/////////////////////////////////////////////////////////////////////////////////
	Sampler::TexAddressingMode Sampler::AddressingMode(TexAddressingType type) const
	{
		switch (type)
		{
		case TAT_Addr_U:
			return addr_mode_u_;

		case TAT_Addr_V:
			return addr_mode_v_;

		case TAT_Addr_W:
			return addr_mode_w_;

		default:
			BOOST_ASSERT(false);
			return addr_mode_u_;
		}
	}
	
	// 设置纹理过滤模式
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::Filtering(TexFilterOp op)
	{
		filter_ = op;
	}

	// 获取纹理过滤模式
	/////////////////////////////////////////////////////////////////////////////////
	Sampler::TexFilterOp Sampler::Filtering() const
	{
		return filter_;
	}

	// 设置纹理异性过滤
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::Anisotropy(uint32_t maxAnisotropy)
	{
		anisotropy_ = maxAnisotropy;
	}

	// 获取纹理异性过滤
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t Sampler::Anisotropy() const
	{
		return anisotropy_;
	}

	// 设置最大的mip等级
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::MaxMipLevel(uint32_t level)
	{
		max_mip_level_ = level;
	}

	// 获取最大的mip等级
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t Sampler::MaxMipLevel() const
	{
		return max_mip_level_;
	}

	// 设置mip map偏移量
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::MipMapLodBias(float bias)
	{
		mip_map_lod_bias_ = bias;
	}

	// 获取mip map偏移量
	/////////////////////////////////////////////////////////////////////////////////
	float Sampler::MipMapLodBias() const
	{
		return mip_map_lod_bias_;
	}

	// 设置纹理矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void Sampler::TextureMatrix(Matrix4 const & mat)
	{
		mat_ = mat;
	}
	
	// 获取纹理矩阵
	/////////////////////////////////////////////////////////////////////////////////
	Matrix4 const & Sampler::TextureMatrix() const
	{
		return mat_;
	}
}
