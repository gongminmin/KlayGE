// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 增加了Dispatch (2009.12.22)
//
// 3.9.0
// 增加了BeginPass/EndPass (2009.4.9)
// 支持Stream Output (2009.5.14)
//
// 3.6.0
// 去掉了RenderTarget，直接使用FrameBuffer (2007.6.20)
//
// 3.5.0
// 支持Alpha to Coverage (2006.9.24)
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

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>
#include <KlayGE/Color.hpp>

#include <vector>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderEngine
	{
	public:
		RenderEngine();
		virtual ~RenderEngine();

		static RenderEnginePtr NullObject();

		virtual std::wstring const & Name() const = 0;

		virtual void StartRendering() = 0;

		virtual void BeginFrame() = 0;
		virtual void BeginPass() = 0;
		void Render(RenderTechnique const & tech, RenderLayout const & rl);
		void Dispatch(RenderTechnique const & tech, RenderLayout const & rl, uint32_t tgx, uint32_t tgy, uint32_t tgz);
		virtual void EndPass() = 0;
		virtual void EndFrame() = 0;

		size_t NumPrimitivesJustRendered();
		size_t NumVerticesJustRendered();

		virtual void CreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;

		void SetStateObjects(RasterizerStateObjectPtr const & rs_obj,
			DepthStencilStateObjectPtr const & dss_obj, uint16_t front_stencil_ref, uint16_t back_stencil_ref,
			BlendStateObjectPtr const & bs_obj, Color const & blend_factor, uint32_t sample_mask);

		void BindFrameBuffer(FrameBufferPtr const & fb);
		FrameBufferPtr const & CurFrameBuffer() const;
		FrameBufferPtr const & DefaultFrameBuffer() const;

		void BindSOBuffers(RenderLayoutPtr const & rl);
		void BindUABuffers(std::vector<GraphicsBufferPtr> const & uabs);

		// Determines the bit depth of the hardware accelerated stencil buffer, if supported.
		virtual uint16_t StencilBufferBitDepth() = 0;

		// Get render device capabilities
		RenderDeviceCaps const & DeviceCaps() const;

		// Scissor support
		virtual void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		// Return the appropiate offsets as full coordinates in the texture values.
		virtual float4 TexelToPixelOffset() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual bool FullScreen() const = 0;
		virtual void FullScreen(bool fs) = 0;

		RasterizerStateObjectPtr const & CurRSObj() const
		{
			return cur_rs_obj_;
		}
		DepthStencilStateObjectPtr const & CurDSSObj() const
		{
			return cur_dss_obj_;
		}
		uint16_t CurFrontStencilRef() const
		{
			return cur_front_stencil_ref_;
		}
		uint16_t CurBackStencilRef() const
		{
			return cur_back_stencil_ref_;
		}
		BlendStateObjectPtr const & CurBSObj() const
		{
			return cur_bs_obj_;
		}
		Color const & CurBlendFactor() const
		{
			return cur_blend_factor_;
		}
		uint32_t CurSampleMask() const
		{
			return cur_sample_mask_;
		}

	protected:
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) = 0;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) = 0;
		virtual void DoBindUABuffers(std::vector<GraphicsBufferPtr> const & uabs) = 0;
		virtual void DoRender(RenderTechnique const & tech, RenderLayout const & rl) = 0;
		virtual void DoDispatch(RenderTechnique const & tech, RenderLayout const & rl, uint32_t tgx, uint32_t tgy, uint32_t tgz) = 0;

	protected:
		FrameBufferPtr cur_frame_buffer_;
		FrameBufferPtr default_frame_buffer_;

		RenderLayoutPtr so_buffers_;

		size_t numPrimitivesJustRendered_;
		size_t numVerticesJustRendered_;

		RenderDeviceCaps caps_;

		RasterizerStateObjectPtr cur_rs_obj_;
		DepthStencilStateObjectPtr cur_dss_obj_;
		uint16_t cur_front_stencil_ref_;
		uint16_t cur_back_stencil_ref_;
		BlendStateObjectPtr cur_bs_obj_;
		Color cur_blend_factor_;
		uint32_t cur_sample_mask_;
	};
}

#endif			// _RENDERENGINE_HPP
