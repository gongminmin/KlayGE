// OGLRenderFactory.h
// KlayGE OpenGL渲染工厂类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERFACTORY_HPP
#define _OGLRENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderEffect.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLOcclusionQuery.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>

#define NOMINMAX
#include <windows.h>
#include <glloader/glloader.h>
#include <gl/glu.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	RenderFactory& OGLRenderFactoryInstance();

	typedef ConcreteRenderFactory<OGLRenderEngine, OGLTexture, OGLFrameBuffer,
				OGLRenderEffect, OGLRenderView> OGLRenderFactoryBase;

	class OGLRenderFactory : public OGLRenderFactoryBase
	{
	public:
		OGLRenderFactory();

		CGcontext CGContext() const;

		TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
				ElementFormat format);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				ElementFormat format);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, ElementFormat format);
		TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				ElementFormat format);

		FrameBufferPtr MakeFrameBuffer();

		RenderEffectPtr DoMakeRenderEffect(std::string const & srcData);

		RenderLayoutPtr MakeRenderLayout(RenderLayout::buffer_type type);
		GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage);
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage);

		QueryPtr MakeOcclusionQuery();

		RenderViewPtr Make1DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level);
		RenderViewPtr Make3DRenderView(Texture& texture, uint32_t slice, int level);
		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf);
		RenderViewPtr MakeDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t multi_sample);

	private:
		CGcontext context_;

	private:
		OGLRenderFactory(OGLRenderFactory const &);
		OGLRenderFactory& operator=(OGLRenderFactory const &);
	};
}

#endif			// _OGLRENDERFACTORY_HPP
