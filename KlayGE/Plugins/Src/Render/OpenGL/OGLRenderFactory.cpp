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
		return TexturePtr(new OGLTexture(width, numMipMaps, format));
	}
	
	TexturePtr OGLRenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				PixelFormat format)
	{
		return TexturePtr(new OGLTexture(width, height, numMipMaps, format));
	}

	TexturePtr OGLRenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, PixelFormat format)
	{
		return TexturePtr(new OGLTexture(width, height, depth, numMipMaps, format));
	}
	
	TexturePtr OGLRenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				PixelFormat format)
	{
		return TexturePtr(new OGLTexture(size, true, numMipMaps, format));
	}

	RenderTexturePtr OGLRenderFactory::MakeRenderTexture()
	{
		return RenderTexturePtr(new OGLRenderTexture);
	}

	RenderEffectPtr OGLRenderFactory::DoMakeRenderEffect(std::string const & srcData)
	{
		return RenderEffectPtr(new OGLRenderEffect(srcData));
	}

	VertexBufferPtr OGLRenderFactory::MakeVertexBuffer(VertexBuffer::BufferType type)
	{
		return VertexBufferPtr(new OGLVertexBuffer(type));
	}

	VertexStreamPtr OGLRenderFactory::MakeVertexStream(BufferUsage usage)
	{
		return VertexStreamPtr(new OGLVertexStream(usage));
	}

	IndexStreamPtr OGLRenderFactory::MakeIndexStream(BufferUsage usage)
	{
		return IndexStreamPtr(new OGLIndexStream(usage));
	}

	RenderVertexStreamPtr OGLRenderFactory::MakeRenderVertexStream(uint32_t width, uint32_t height)
	{
		return RenderVertexStreamPtr(new OGLRenderVertexStream(width, height));
	}

	QueryPtr OGLRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr(new OGLOcclusionQuery);
	}

	RenderFactory& OGLRenderFactoryInstance()
	{
		static OGLRenderFactory renderFactory;
		return renderFactory;
	}
}
