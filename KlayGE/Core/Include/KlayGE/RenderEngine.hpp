// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 增加了TexelToPixelOffset (2006.8.27)
// 去掉了ClearColor (2006.8.31)
//
// 3.3.0
// 统一了RenderState (2006.5.21)
//
// 3.2.0
// 暴露出了Clear (2005.12.31)
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
// 支持point sprite (2005.9.28)
// 支持scissor (2005.10.20)
//
// 2.8.0
// 简化了StencilBuffer相关操作 (2005.7.20)
// 简化了RenderTarget，支持MRT (2005.7.25)
// 去掉了纹理坐标生成 (2005.7.30)
//
// 2.7.1
// ViewMatrix和ProjectionMatrix改为const (2005.7.10)
//
// 2.7.0
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了NumFacesJustRendered和NumVerticesJustRendered (2005.3.21)
// 增加了PolygonMode (2005.3.20)
//
// 2.0.4
// 去掉了WorldMatrices (2004.4.3)
// 保存了三个矩阵 (2004.4.7)
//
// 2.0.3
// 去掉了SoftwareBlend (2004.3.10)
//
// 2.0.1
// 去掉了TexBlendMode (2003.10.16)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERENGINE_HPP
#define _RENDERENGINE_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>

#include <vector>
#include <list>
#include <boost/array.hpp>

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

	class RenderEngine
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

		enum ClearBufferMask
		{
			CBM_Color   = 1UL << 0,
			CBM_Depth   = 1UL << 1,
			CBM_Stencil = 1UL << 2
		};

		enum RenderStateType
		{
			RST_PolygonMode				= 0x00,
			RST_ShadeMode,
			RST_CullMode,
			RST_Clipping,

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

		enum ColorMask
		{
			CMASK_Red   = 1UL << 3,
			CMASK_Green = 1UL << 2,
			CMASK_Blue  = 1UL << 1,
			CMASK_Alpha = 1UL << 0,
			CMASK_All   = CMASK_Red | CMASK_Green | CMASK_Blue | CMASK_Alpha
		};

	public:
		RenderEngine();
		virtual ~RenderEngine();

		static RenderEnginePtr NullObject();

		virtual std::wstring const & Name() const = 0;

		virtual void StartRendering() = 0;

		void SetRenderTechnique(RenderTechniquePtr const & tech);
		RenderTechniquePtr GetRenderTechnique() const;

		virtual void BeginFrame() = 0;
		void Render(RenderLayout const & rl);
		virtual void EndFrame() = 0;

		size_t NumPrimitivesJustRendered();
		size_t NumVerticesJustRendered();

		virtual void Clear(uint32_t masks, Color const & clr, float depth, int32_t stencil) = 0;

		virtual RenderWindowPtr CreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;

		void SetRenderState(RenderStateType rst, uint32_t state);
		uint32_t GetRenderState(RenderStateType rst);

		void BindRenderTarget(RenderTargetPtr rt);
		RenderTargetPtr CurRenderTarget() const;
		RenderTargetPtr DefaultRenderTarget() const;

		// Set a sampler.
		virtual void SetSampler(uint32_t stage, SamplerPtr const & sampler) = 0;
		// Turns off a sampler.
		virtual void DisableSampler(uint32_t stage) = 0;

		// Determines the bit depth of the hardware accelerated stencil buffer, if supported.
		virtual uint16_t StencilBufferBitDepth() = 0;

		// Get render device capabilities
		RenderDeviceCaps const & DeviceCaps() const;

		// Scissor support
		virtual void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		// Return the appropiate offsets as full coordinates in the texture values.
		virtual float4 TexelToPixelOffset() const = 0;

	protected:
		virtual void DoBindRenderTarget(RenderTargetPtr rt) = 0;
		virtual void DoRender(RenderLayout const & rl) = 0;
		virtual void DoFlushRenderStates() = 0;

		virtual void InitRenderStates() = 0;
		virtual void FillRenderDeviceCaps() = 0;

	protected:
		RenderTargetPtr cur_render_target_;
		RenderTargetPtr default_render_target_;

		RenderTechniquePtr render_tech_;

		size_t numPrimitivesJustRendered_;
		size_t numVerticesJustRendered_;

		RenderDeviceCaps caps_;

		boost::array<uint32_t, RST_NUM_RENDER_STATES> render_states_;
		boost::array<bool, RST_NUM_RENDER_STATES> dirty_render_states_;
	};
}

#endif			// _RENDERENGINE_HPP
