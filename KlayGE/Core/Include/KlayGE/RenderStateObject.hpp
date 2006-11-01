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

	class RenderStateObject
	{
	public:
		enum ShadeMode
		{
			SM_Flat,
			SM_Gouraud
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

		enum CullMode
		{
			CM_None,
			CM_Clockwise,
			CM_AntiClockwise
		};

		enum PolygonMode
		{
			PM_Point,
			PM_Line,
			PM_Fill
		};

		// Type of alpha blend factor.
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

		enum BlendOperation
		{
			BOP_Add		= 1,
			BOP_Sub		= 2,
			BOP_Rev_Sub	= 3,
			BOP_Min		= 4,
			BOP_Max		= 5,
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

		enum RenderStateType
		{
			RST_PolygonMode				= 0x00,
			RST_ShadeMode,
			RST_CullMode,

			RST_AlphaToCoverageEnable,
			RST_BlendEnable,
			RST_BlendOp,
			RST_SrcBlend,
			RST_DestBlend,
			RST_BlendOpAlpha,
			RST_SrcBlendAlpha,
			RST_DestBlendAlpha,
			
			RST_DepthEnable,
			RST_DepthMask,
			RST_DepthFunc,
			RST_PolygonOffsetFactor,
			RST_PolygonOffsetUnits,

			// Turns stencil buffer checking on or off. 
			RST_FrontStencilEnable,
			// Stencil test function
			RST_FrontStencilFunc,
			// Stencil test reference value
			RST_FrontStencilRef,
			// Stencil test mask value
			RST_FrontStencilMask,
			// Sets the action to perform if the stencil test fails,
			RST_FrontStencilFail,
			// if the stencil test passes, but the depth buffer test fails
			RST_FrontStencilDepthFail,
			// if both the stencil test and the depth buffer test passes
			RST_FrontStencilPass,
			RST_FrontStencilWriteMask,
			RST_BackStencilEnable,
			RST_BackStencilFunc,
			RST_BackStencilRef,
			RST_BackStencilMask,
			RST_BackStencilFail,
			RST_BackStencilDepthFail,
			RST_BackStencilPass,
			RST_BackStencilWriteMask,

			RST_ScissorEnable,

			RST_ColorMask0,
			RST_ColorMask1,
			RST_ColorMask2,
			RST_ColorMask3,

			RST_NUM_RENDER_STATES
		};

	public:
		RenderStateObject();

		void SetRenderState(RenderStateType rst, uint32_t state);
		uint32_t GetRenderState(RenderStateType rst) const;

	private:
		boost::array<uint32_t, RST_NUM_RENDER_STATES> render_states_;
	};
}

#endif			// _RENDERSTATEOBJECT_HPP
