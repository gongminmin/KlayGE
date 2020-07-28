// RenderStateObject.cpp
// KlayGE 渲染状态对象类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 增加了SamplerStateObject (2008.9.21)
//
// 3.7.0
// 把RenderStateObject拆成三部分 (2008.6.29)
//
// 3.5.0
// 初次建立 (2006.11.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <cstring>
#include <limits>
#include <boost/assert.hpp>

#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	RasterizerStateDesc::RasterizerStateDesc()
		: polygon_mode(PM_Fill),
			shade_mode(SM_Gouraud),
			cull_mode(CM_Back),
			front_face_ccw(false),
			polygon_offset_factor(0),
			polygon_offset_units(0),
			depth_clip_enable(true),
			scissor_enable(false),
			multisample_enable(true)
	{
	}

	bool operator<(RasterizerStateDesc const & lhs, RasterizerStateDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	DepthStencilStateDesc::DepthStencilStateDesc()
		: depth_enable(true),
			depth_write_mask(true),
			depth_func(CF_Less),
			front_stencil_enable(false),
			front_stencil_func(CF_AlwaysPass),
			front_stencil_ref(0),
			front_stencil_read_mask(0xFFFF),
			front_stencil_write_mask(0xFFFF),
			front_stencil_fail(SOP_Keep),
			front_stencil_depth_fail(SOP_Keep),
			front_stencil_pass(SOP_Keep),
			back_stencil_enable(false),
			back_stencil_func(CF_AlwaysPass),
			back_stencil_ref(0),
			back_stencil_read_mask(0xFFFF),
			back_stencil_write_mask(0xFFFF),
			back_stencil_fail(SOP_Keep),
			back_stencil_depth_fail(SOP_Keep),
			back_stencil_pass(SOP_Keep)
	{
	}

	bool operator<(DepthStencilStateDesc const & lhs, DepthStencilStateDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	BlendStateDesc::BlendStateDesc()
		: blend_factor(1, 1, 1, 1), sample_mask(0xFFFFFFFF),
			alpha_to_coverage_enable(false), independent_blend_enable(false)
	{
		blend_enable.fill(false);
		logic_op_enable.fill(false);
		blend_op.fill(BOP_Add);
		src_blend.fill(ABF_One);
		dest_blend.fill(ABF_Zero);
		blend_op_alpha.fill(BOP_Add);
		src_blend_alpha.fill(ABF_One);
		dest_blend_alpha.fill(ABF_Zero);
		color_write_mask.fill(CMASK_All);
		logic_op.fill(LOP_Noop);
	}

	bool operator<(BlendStateDesc const & lhs, BlendStateDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	SamplerStateDesc::SamplerStateDesc()
		: border_clr(0, 0, 0, 0),
			addr_mode_u(TAM_Wrap), addr_mode_v(TAM_Wrap), addr_mode_w(TAM_Wrap),
			filter(TFO_Min_Mag_Mip_Point),
			max_anisotropy(16),
			min_lod(0), max_lod(std::numeric_limits<float>::max()),
			mip_map_lod_bias(0),
			cmp_func(CF_AlwaysFail)
	{
	}

	bool operator<(SamplerStateDesc const & lhs, SamplerStateDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}


	RenderStateObject::RenderStateObject(
		RasterizerStateDesc const& rs_desc, DepthStencilStateDesc const& dss_desc, BlendStateDesc const& bs_desc)
		: rs_desc_(rs_desc), dss_desc_(dss_desc), bs_desc_(bs_desc)
	{
	}

	RenderStateObject::~RenderStateObject() noexcept = default;
}
