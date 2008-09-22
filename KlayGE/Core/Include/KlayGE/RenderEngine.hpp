// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://klayge.sourceforge.net
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

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>

namespace KlayGE
{
	class RenderEngine
	{
	public:
		RenderEngine();
		virtual ~RenderEngine();

		static RenderEnginePtr NullObject();

		virtual std::wstring const & Name() const = 0;

		virtual void StartRendering() = 0;

		virtual void BeginFrame() = 0;
		void Render(RenderTechnique const & tech, RenderLayout const & rl);
		virtual void EndFrame() = 0;

		size_t NumPrimitivesJustRendered();
		size_t NumVerticesJustRendered();

		virtual void CreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;

		void SetStateObjects(RasterizerStateObjectPtr const & rs_obj, DepthStencilStateObjectPtr const & dss_obj, uint16_t front_stencil_ref, uint16_t back_stencil_ref,
			BlendStateObjectPtr const & bs_obj, Color const & blend_factor, uint32_t sample_mask, ShaderObjectPtr const & shader_obj);

		void BindFrameBuffer(FrameBufferPtr fb);
		FrameBufferPtr CurFrameBuffer() const;
		FrameBufferPtr DefaultFrameBuffer() const;

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

	protected:
		virtual void DoBindFrameBuffer(FrameBufferPtr fb) = 0;
		virtual void DoRender(RenderTechnique const & tech, RenderLayout const & rl) = 0;

	protected:
		FrameBufferPtr cur_frame_buffer_;
		FrameBufferPtr default_frame_buffer_;

		size_t numPrimitivesJustRendered_;
		size_t numVerticesJustRendered_;

		RenderDeviceCaps caps_;

		RasterizerStateObjectPtr cur_rs_obj_;
		DepthStencilStateObjectPtr cur_dss_obj_;
		BlendStateObjectPtr cur_bs_obj_;
	};
}

#endif			// _RENDERENGINE_HPP
