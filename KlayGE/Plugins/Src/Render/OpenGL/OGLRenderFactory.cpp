#include <KlayGE/KlayGE.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderEffect.hpp>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLIndexStream.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

namespace KlayGE
{
	typedef ConcreteRenderFactory<OGLRenderEngine, OGLTexture, OGLRenderTexture,
				OGLRenderEffect> OGLRenderFactoryBase;

	class OGLRenderFactory : public OGLRenderFactoryBase
	{
	public:
		OGLRenderFactory()
			: OGLRenderFactoryBase(L"OpenGL Render Factory")
			{ }

		TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage)
		{
			return TexturePtr(new OGLTexture(width, numMipMaps, format, usage));
		}
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage)
		{
			return TexturePtr(new OGLTexture(width, height, numMipMaps, format, usage));
		}
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, PixelFormat format, Texture::TextureUsage usage)
		{
			return TexturePtr(new OGLTexture(width, height, depth, numMipMaps, format, usage));
		}
		TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage)
		{
			return TexturePtr(new OGLTexture(size, true, numMipMaps, format, usage));
		}

		RenderTexturePtr MakeRenderTexture(uint32_t width, uint32_t height)
		{
			return RenderTexturePtr(new OGLRenderTexture(width, height));
		}

		RenderEffectPtr MakeRenderEffect(std::string const & srcData)
		{
			return RenderEffectPtr(new OGLRenderEffect(srcData));
		}
			
		VertexStreamPtr MakeVertexStream(VertexStreamType type,
			uint8_t sizeElement, uint8_t numElement, bool staticStream)
		{
			VertexStreamPtr stream;

			switch (type)
			{
			case VST_Positions:
			case VST_Normals:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), 3));
				break;

			case VST_Diffuses:
			case VST_Speculars:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(uint32_t), 1));
				break;

			case VST_BlendWeights:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), 4));
				break;

			case VST_BlendIndices:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(uint8_t), 4));
				break;

			case VST_TextureCoords0:
			case VST_TextureCoords1:
			case VST_TextureCoords2:
			case VST_TextureCoords3:
			case VST_TextureCoords4:
			case VST_TextureCoords5:
			case VST_TextureCoords6:
			case VST_TextureCoords7:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), numElement));
				break;

			default:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeElement, numElement));
				break;
			}

			return stream;
		}

		IndexStreamPtr MakeIndexStream(bool staticStream)
		{
			return IndexStreamPtr(new OGLIndexStream);
		}
	};

	RenderFactory& OGLRenderFactoryInstance()
	{
		static OGLRenderFactory renderFactory;
		return renderFactory;
	}
}
