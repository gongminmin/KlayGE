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
	{
		render_states_[RST_PolygonMode]		= PM_Fill;
		render_states_[RST_ShadeMode]		= SM_Gouraud;
		render_states_[RST_CullMode]		= CM_AntiClockwise;

		render_states_[RST_AlphaToCoverageEnable] = false;
		render_states_[RST_BlendEnable]		= false;
		render_states_[RST_BlendOp]			= BOP_Add;
		render_states_[RST_SrcBlend]		= ABF_One;
		render_states_[RST_DestBlend]		= ABF_Zero;
		render_states_[RST_BlendOpAlpha]	= BOP_Add;
		render_states_[RST_SrcBlendAlpha]	= ABF_One;
		render_states_[RST_DestBlendAlpha]	= ABF_Zero;

		render_states_[RST_DepthEnable]			= true;
		render_states_[RST_DepthMask]			= true;
		render_states_[RST_DepthFunc]			= CF_LessEqual;
		render_states_[RST_PolygonOffsetFactor]	= 0;
		render_states_[RST_PolygonOffsetUnits]	= 0;

		render_states_[RST_FrontStencilEnable]		= false;
		render_states_[RST_FrontStencilFunc]		= CF_AlwaysPass;
		render_states_[RST_FrontStencilRef]			= 0;
		render_states_[RST_FrontStencilMask]		= 0xFFFFFFFF;
		render_states_[RST_FrontStencilFail]		= SOP_Keep;
		render_states_[RST_FrontStencilDepthFail]	= SOP_Keep;
		render_states_[RST_FrontStencilPass]		= SOP_Keep;
		render_states_[RST_FrontStencilWriteMask]	= 0xFFFFFFFF;
		render_states_[RST_BackStencilEnable]		= false;
		render_states_[RST_BackStencilFunc]			= CF_AlwaysPass;
		render_states_[RST_BackStencilRef]			= 0;
		render_states_[RST_BackStencilMask]			= 0xFFFFFFFF;
		render_states_[RST_BackStencilFail]			= SOP_Keep;
		render_states_[RST_BackStencilDepthFail]	= SOP_Keep;
		render_states_[RST_BackStencilPass]			= SOP_Keep;
		render_states_[RST_BackStencilWriteMask]	= 0xFFFFFFFF;

		render_states_[RST_ColorMask0] = 0xF;
		render_states_[RST_ColorMask1] = 0xF;
		render_states_[RST_ColorMask2] = 0xF;
		render_states_[RST_ColorMask3] = 0xF;
	}

	void RenderStateObject::SetRenderState(RenderStateType rst, uint32_t state)
	{
		BOOST_ASSERT(static_cast<size_t>(rst) < render_states_.size());

		render_states_[rst] = state;
	}

	uint32_t RenderStateObject::GetRenderState(RenderStateType rst) const
	{
		BOOST_ASSERT(static_cast<size_t>(rst) < render_states_.size());

		return render_states_[rst];
	}
}
