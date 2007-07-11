// RenderStateObject.cpp
// KlayGE 渲染状态对象类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 初次建立 (2006.11.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/assert.hpp>

#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	RenderStateObject::RenderStateObject()
		: polygon_mode(PM_Fill),
			shade_mode(SM_Gouraud),
			cull_mode(CM_AntiClockwise),
			alpha_to_coverage_enable(false),
			blend_enable(false),
			blend_op(BOP_Add),
			src_blend(ABF_One),
			dest_blend(ABF_Zero),
			blend_op_alpha(BOP_Add),
			src_blend_alpha(ABF_One),
			dest_blend_alpha(ABF_Zero),
			depth_enable(true),
			depth_mask(true),
			depth_func(CF_Less),
			polygon_offset_factor(0),
			polygon_offset_units(0),
			front_stencil_enable(false),
			front_stencil_func(CF_AlwaysPass),
			front_stencil_ref(0),
			front_stencil_mask(0xFFFF),
			front_stencil_fail(SOP_Keep),
			front_stencil_depth_fail(SOP_Keep),
			front_stencil_pass(SOP_Keep),
			front_stencil_write_mask(0xFFFF),
			back_stencil_enable(false),
			back_stencil_func(CF_AlwaysPass),
			back_stencil_ref(0),
			back_stencil_mask(0xFFFF),
			back_stencil_fail(SOP_Keep),
			back_stencil_depth_fail(SOP_Keep),
			back_stencil_pass(SOP_Keep),
			back_stencil_write_mask(0xFFFF),
			scissor_enable(false),
			color_mask_0(CMASK_All),
			color_mask_1(CMASK_All),
			color_mask_2(CMASK_All),
			color_mask_3(CMASK_All)
	{
	}
}
