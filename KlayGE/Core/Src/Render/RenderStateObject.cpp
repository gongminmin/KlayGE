// RenderStateObject.cpp
// KlayGE 渲染状态对象类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <cstring>
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
			scissor_enable(false),
			multisample_enable(false)
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
			front_stencil_read_mask(0xFFFF),
			front_stencil_write_mask(0xFFFF),
			front_stencil_fail(SOP_Keep),
			front_stencil_depth_fail(SOP_Keep),
			front_stencil_pass(SOP_Keep),
			back_stencil_enable(false),
			back_stencil_func(CF_AlwaysPass),
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
		: alpha_to_coverage_enable(false),
			independent_blend_enable(false)
	{
		blend_enable.assign(false);
		blend_op.assign(BOP_Add);
		src_blend.assign(ABF_One);
		dest_blend.assign(ABF_Zero);
		blend_op_alpha.assign(BOP_Add);
		src_blend_alpha.assign(ABF_One);
		dest_blend_alpha.assign(ABF_Zero);
		color_write_mask.assign(CMASK_All);
	}

	bool operator<(BlendStateDesc const & lhs, BlendStateDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	SamplerStateDesc::SamplerStateDesc()
		: border_clr(0, 0, 0, 0),
			addr_mode_u(TAM_Wrap), addr_mode_v(TAM_Wrap), addr_mode_w(TAM_Wrap),
			filter(TFO_Min_Mag_Mip_Point),
			anisotropy(1),
			max_mip_level(1),
			mip_map_lod_bias(0)
	{
	}

	bool operator<(SamplerStateDesc const & lhs, SamplerStateDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}


	RasterizerStateObjectPtr RasterizerStateObject::NullObject()
	{
		static RasterizerStateObjectPtr obj(Context::Instance().RenderFactoryInstance().MakeRasterizerStateObject(RasterizerStateDesc()));
		return obj;
	}

	DepthStencilStateObjectPtr DepthStencilStateObject::NullObject()
	{
		static DepthStencilStateObjectPtr obj(Context::Instance().RenderFactoryInstance().MakeDepthStencilStateObject(DepthStencilStateDesc()));
		return obj;
	}

	BlendStateObjectPtr BlendStateObject::NullObject()
	{
		static BlendStateObjectPtr obj(Context::Instance().RenderFactoryInstance().MakeBlendStateObject(BlendStateDesc()));
		return obj;
	}

	SamplerStateObjectPtr SamplerStateObject::NullObject()
	{
		static SamplerStateObjectPtr obj(Context::Instance().RenderFactoryInstance().MakeSamplerStateObject(SamplerStateDesc()));
		return obj;
	}
}
