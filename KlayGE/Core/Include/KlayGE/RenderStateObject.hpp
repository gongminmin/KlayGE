// RenderStateObject.hpp
// KlayGE 渲染状态对象类 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 初次建立 (2006.11.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERSTATEOBJECT_HPP
#define _RENDERSTATEOBJECT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Math.hpp>

#include <vector>

namespace KlayGE
{
	inline uint32_t
	float_to_uint32(float v)
	{
		return *reinterpret_cast<uint32_t*>(&v);
	}
	inline float
	uint32_to_float(uint32_t v)
	{
		return *reinterpret_cast<float*>(&v);
	}

#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif
	struct RenderStateObject
	{
		enum PolygonMode
		{
			PM_Point,
			PM_Line,
			PM_Fill
		};

		enum ShadeMode
		{
			SM_Flat,
			SM_Gouraud
		};

		enum CullMode
		{
			CM_None,
			CM_Clockwise,
			CM_AntiClockwise
		};

		enum BlendOperation
		{
			BOP_Add		= 1,
			BOP_Sub		= 2,
			BOP_Rev_Sub	= 3,
			BOP_Min		= 4,
			BOP_Max		= 5,
		};

		enum AlphaBlendFactor
		{
			ABF_Zero,
			ABF_One,
			ABF_Src_Alpha,
			ABF_Dst_Alpha,
			ABF_Inv_Src_Alpha,
			ABF_Inv_Dst_Alpha,
			ABF_Src_Color,
			ABF_Dst_Color,
			ABF_Inv_Src_Color,
			ABF_Inv_Dst_Color,
			ABF_Src_Alpha_Sat
		};

		enum CompareFunction
		{
			CF_AlwaysFail,
			CF_AlwaysPass,
			CF_Less,
			CF_LessEqual,
			CF_Equal,
			CF_NotEqual,
			CF_GreaterEqual,
			CF_Greater
		};

		// Enum describing the various actions which can be taken onthe stencil buffer
		enum StencilOperation
		{
			// Leave the stencil buffer unchanged
			SOP_Keep,
			// Set the stencil value to zero
			SOP_Zero,
			// Set the stencil value to the reference value
			SOP_Replace,
			// Increase the stencil value by 1, clamping at the maximum value
			SOP_Increment,
			// Decrease the stencil value by 1, clamping at 0
			SOP_Decrement,
			// Invert the bits of the stencil buffer
			SOP_Invert
		};

		enum ColorMask
		{
			CMASK_Red   = 1UL << 3,
			CMASK_Green = 1UL << 2,
			CMASK_Blue  = 1UL << 1,
			CMASK_Alpha = 1UL << 0,
			CMASK_All   = CMASK_Red | CMASK_Green | CMASK_Blue | CMASK_Alpha
		};

		
		PolygonMode			polygon_mode;
		ShadeMode			shade_mode;
		CullMode			cull_mode;

		bool				alpha_to_coverage_enable;
		bool				blend_enable;
		BlendOperation		blend_op;
		AlphaBlendFactor	src_blend;
		AlphaBlendFactor	dest_blend;
		BlendOperation		blend_op_alpha;
		AlphaBlendFactor	src_blend_alpha;
		AlphaBlendFactor	dest_blend_alpha;

		bool				depth_enable;
		bool				depth_mask;
		CompareFunction		depth_func;
		float				polygon_offset_factor;
		float				polygon_offset_units;

		bool				front_stencil_enable;
		CompareFunction		front_stencil_func;
		uint16_t			front_stencil_ref;
		uint16_t			front_stencil_mask;
		StencilOperation	front_stencil_fail;
		StencilOperation	front_stencil_depth_fail;
		StencilOperation	front_stencil_pass;
		uint16_t			front_stencil_write_mask;

		bool				back_stencil_enable;
		CompareFunction		back_stencil_func;
		uint16_t			back_stencil_ref;
		uint16_t			back_stencil_mask;
		StencilOperation	back_stencil_fail;
		StencilOperation	back_stencil_depth_fail;
		StencilOperation	back_stencil_pass;
		uint16_t			back_stencil_write_mask;

		bool				scissor_enable;

		uint8_t				color_mask_0 : 4;
		uint8_t				color_mask_1 : 4;
		uint8_t				color_mask_2 : 4;
		uint8_t				color_mask_3 : 4;

		RenderStateObject();
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif
}

#endif			// _RENDERSTATEOBJECT_HPP
