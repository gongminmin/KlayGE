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

	TexturePtr OGLRenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage)
	{
		return TexturePtr(new OGLTexture(width, numMipMaps, format, usage));
	}
	
	TexturePtr OGLRenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage)
	{
		return TexturePtr(new OGLTexture(width, height, numMipMaps, format, usage));
	}

	TexturePtr OGLRenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, PixelFormat format, Texture::TextureUsage usage)
	{
		return TexturePtr(new OGLTexture(width, height, depth, numMipMaps, format, usage));
	}
	
	TexturePtr OGLRenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage)
	{
		return TexturePtr(new OGLTexture(size, true, numMipMaps, format, usage));
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

	VertexStreamPtr OGLRenderFactory::MakeVertexStream(VertexStreamType type,
			uint8_t sizeElement, uint8_t numElement, bool staticStream)
	{
		VertexStreamPtr stream;

		switch (type)
		{
		case VST_Positions:
		case VST_Normals:
			stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), 3, staticStream));
			break;

		case VST_Diffuses:
		case VST_Speculars:
			stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(uint32_t), 1, staticStream));
			break;

		case VST_BlendWeights:
			stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), 4, staticStream));
			break;

		case VST_BlendIndices:
			stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(uint8_t), 4, staticStream));
			break;

		case VST_TextureCoords0:
		case VST_TextureCoords1:
		case VST_TextureCoords2:
		case VST_TextureCoords3:
		case VST_TextureCoords4:
		case VST_TextureCoords5:
		case VST_TextureCoords6:
		case VST_TextureCoords7:
			stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), numElement, staticStream));
			break;

		default:
			stream = VertexStreamPtr(new OGLVertexStream(type, sizeElement, numElement, staticStream));
			break;
		}

		return stream;
	}

	IndexStreamPtr OGLRenderFactory::MakeIndexStream(bool staticStream)
	{
		return IndexStreamPtr(new OGLIndexStream(staticStream));
	}

	RenderVertexStreamPtr OGLRenderFactory::MakeRenderVertexStream(uint32_t width, uint32_t height)
	{
		return RenderVertexStreamPtr(new OGLRenderVertexStream(width, height));
	}

	OcclusionQueryPtr OGLRenderFactory::MakeOcclusionQuery()
	{
		return OcclusionQueryPtr(new OGLOcclusionQuery);
	}

	RenderFactory& OGLRenderFactoryInstance()
	{
		static OGLRenderFactory renderFactory;
		return renderFactory;
	}
}
