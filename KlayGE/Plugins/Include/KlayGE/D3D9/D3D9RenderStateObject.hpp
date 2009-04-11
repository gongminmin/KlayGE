// D3D9RenderStateObject.hpp
// KlayGE 渲染状态对象类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.7.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERSTATEOBJECT_HPP
#define _D3D9RENDERSTATEOBJECT_HPP

#pragma KLAYGE_ONCE

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class D3D9RasterizerStateObject : public RasterizerStateObject
	{
	public:
		explicit D3D9RasterizerStateObject(RasterizerStateDesc const & desc);

		void Active();

	private:
		uint32_t d3d9_polygon_mode_;
		uint32_t d3d9_shade_mode_;
		uint32_t d3d9_cull_mode_;
		uint32_t d3d9_polygon_offset_factor_;
		uint32_t d3d9_polygon_offset_units_;
	};

	class D3D9DepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit D3D9DepthStencilStateObject(DepthStencilStateDesc const & desc);

		void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref);

	private:
		uint32_t d3d9_depth_func_;
		uint32_t d3d9_front_stencil_func_;
		uint32_t d3d9_front_stencil_fail_;
		uint32_t d3d9_front_stencil_depth_fail_;
		uint32_t d3d9_front_stencil_pass_;
		uint32_t d3d9_back_stencil_func_;
		uint32_t d3d9_back_stencil_fail_;
		uint32_t d3d9_back_stencil_depth_fail_;
		uint32_t d3d9_back_stencil_pass_;
	};

	class D3D9BlendStateObject : public BlendStateObject
	{
	public:
		explicit D3D9BlendStateObject(BlendStateDesc const & desc);

		void Active(Color const & blend_factor, uint32_t sample_mask);

	private:
		uint32_t d3d9_blend_op_;
		uint32_t d3d9_src_blend_;
		uint32_t d3d9_dest_blend_;
		uint32_t d3d9_blend_op_alpha_;
		uint32_t d3d9_src_blend_alpha_;
		uint32_t d3d9_dest_blend_alpha_;
		uint32_t d3d9_color_write_mask_0_;
		uint32_t d3d9_color_write_mask_1_;
		uint32_t d3d9_color_write_mask_2_;
		uint32_t d3d9_color_write_mask_3_;
	};

	class D3D9SamplerStateObject : public SamplerStateObject
	{
	public:
		explicit D3D9SamplerStateObject(SamplerStateDesc const & desc);

		void Active(uint32_t stage, TexturePtr const & texture);

	private:
		uint32_t d3d9_border_clr_;
		uint32_t d3d9_addr_mode_u_;
		uint32_t d3d9_addr_mode_v_;
		uint32_t d3d9_addr_mode_w_;
		uint32_t d3d9_min_filter_;
		uint32_t d3d9_mag_filter_;
		uint32_t d3d9_mip_filter_;
	};
}

#endif			// _D3D9RENDERSTATEOBJECT_HPP
