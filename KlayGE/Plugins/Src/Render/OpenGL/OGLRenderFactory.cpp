#include <KlayGE/KlayGE.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderEffect.hpp>
#include <KlayGE/OpenGL/OGLFont.hpp>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLIndexStream.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

namespace KlayGE
{
	typedef ConcreteRenderFactory<OGLRenderEngine, OGLTexture, OGLRenderTexture,
				OGLFont, OGLRenderEffect> OGLRenderFactoryBase;
	class OGLRenderFactory : public OGLRenderFactoryBase
	{
	public:
		OGLRenderFactory()
			: OGLRenderFactoryBase(L"OpenGL Render Factory")
			{ }
			
		VertexStreamPtr MakeVertexStream(VertexStreamType type,
			uint8 sizeElement, uint8 numElement, bool staticStream = false)
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
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(uint32), 1));
				break;

			case VST_BlendWeights:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(float), 4));
				break;

			case VST_BlendIndices:
				stream = VertexStreamPtr(new OGLVertexStream(type, sizeof(uint8), 4));
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

		IndexStreamPtr MakeIndexStream(bool staticStream = false)
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
