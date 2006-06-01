// OGLRenderFactory.cpp
// KlayGE OpenGL渲染工厂类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 可以建立静态OGLVertexStream和OGLIndexStream (2005.6.19)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#pragma comment(lib, "Cg.lib")
#pragma comment(lib, "CgGL.lib")

namespace KlayGE
{
	OGLRenderFactory::OGLRenderFactory()
			: OGLRenderFactoryBase(L"OpenGL Render Factory")
	{
		context_ = cgCreateContext();
		cgGLRegisterStates(context_);
	}
	
	CGcontext OGLRenderFactory::CGContext() const
	{
		return context_;
	}

	TexturePtr OGLRenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps, PixelFormat format)
	{
		return TexturePtr(new OGLTexture1D(width, numMipMaps, format));
	}
	
	TexturePtr OGLRenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				PixelFormat format)
	{
		return TexturePtr(new OGLTexture2D(width, height, numMipMaps, format));
	}

	TexturePtr OGLRenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, PixelFormat format)
	{
		return TexturePtr(new OGLTexture3D(width, height, depth, numMipMaps, format));
	}
	
	TexturePtr OGLRenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				PixelFormat format)
	{
		return TexturePtr(new OGLTextureCube(size, numMipMaps, format));
	}

	FrameBufferPtr OGLRenderFactory::MakeFrameBuffer()
	{
		return FrameBufferPtr(new OGLFrameBuffer);
	}

	RenderEffectPtr OGLRenderFactory::DoMakeRenderEffect(std::string const & srcData)
	{
		return RenderEffectPtr(new OGLRenderEffect(srcData));
	}

	RenderLayoutPtr OGLRenderFactory::MakeRenderLayout(RenderLayout::buffer_type type)
	{
		return RenderLayoutPtr(new OGLRenderLayout(type));
	}

	GraphicsBufferPtr OGLRenderFactory::MakeVertexBuffer(BufferUsage usage)
	{
		return GraphicsBufferPtr(new OGLGraphicsBuffer(usage, GL_ARRAY_BUFFER));
	}

	GraphicsBufferPtr OGLRenderFactory::MakeIndexBuffer(BufferUsage usage)
	{
		return GraphicsBufferPtr(new OGLGraphicsBuffer(usage, GL_ELEMENT_ARRAY_BUFFER));
	}

	QueryPtr OGLRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr(new OGLOcclusionQuery);
	}

	RenderViewPtr OGLRenderFactory::Make1DRenderView(Texture& texture, int level)
	{
		return checked_cast<OGLTexture1D*>(&texture)->CreateRenderView(level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, int level)
	{
		return checked_cast<OGLTexture2D*>(&texture)->CreateRenderView(level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level)
	{
		return checked_cast<OGLTextureCube*>(&texture)->CreateRenderView(face, level);
	}

	RenderViewPtr OGLRenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height)
	{
		return checked_cast<OGLGraphicsBuffer*>(&gbuffer)->CreateRenderView(width, height);
	}

	RenderFactory& OGLRenderFactoryInstance()
	{
		static OGLRenderFactory renderFactory;
		return renderFactory;
	}
}
