// D3D10RenderStateObject.hpp
// KlayGE D3D10渲染状态对象类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10RENDERSTATEOBJECT_HPP
#define _D3D10RENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class D3D10RasterizerStateObject : public RasterizerStateObject
	{
	public:
		explicit D3D10RasterizerStateObject(RasterizerStateDesc const & desc);

		void Active();

		ID3D10RasterizerStatePtr const & D3DRasterizerState() const
		{
			return rasterizer_state_;
		}

	private:
		ID3D10RasterizerStatePtr rasterizer_state_;
	};

	class D3D10DepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit D3D10DepthStencilStateObject(DepthStencilStateDesc const & desc);

		void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref);

		ID3D10DepthStencilStatePtr const & D3DDepthStencilState() const
		{
			return depth_stencil_state_;
		}

	private:
		ID3D10DepthStencilStatePtr depth_stencil_state_;
	};

	class D3D10BlendStateObject : public BlendStateObject
	{
	public:
		explicit D3D10BlendStateObject(BlendStateDesc const & desc);

		void Active(Color const & blend_factor, uint32_t sample_mask);

		ID3D10BlendStatePtr const & D3DBlendState() const
		{
			return blend_state_;
		}

	private:
		ID3D10BlendStatePtr blend_state_;
	};

	class D3D10SamplerStateObject : public SamplerStateObject
	{
	public:
		explicit D3D10SamplerStateObject(SamplerStateDesc const & desc);

		ID3D10SamplerStatePtr const & D3DSamplerState() const
		{
			return sampler_state_;
		}

	private:
		ID3D10SamplerStatePtr sampler_state_;
	};
}

#endif			// _D3D10RENDERSTATEOBJECT_HPP
