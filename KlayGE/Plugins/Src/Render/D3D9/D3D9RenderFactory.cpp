// D3D9RenderFactory.cpp
// KlayGE D3D9渲染引擎抽象工厂 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderTexture.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9Font.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

namespace KlayGE
{
	class D3D9RenderFactory : public ConcreteRenderFactory<D3D9RenderEngine, D3D9Texture, D3D9RenderTexture,
			D3D9Font, D3D9RenderEffect>
	{
	public:
		D3D9RenderFactory()
			: ConcreteRenderFactory<D3D9RenderEngine, D3D9Texture, D3D9RenderTexture,
				D3D9Font, D3D9RenderEffect>(L"Direct3D9 Render Factory")
			{ }
			
		VertexStreamPtr MakeVertexStream(VertexStreamType type,
			U8 sizeElement, U8 numElement, bool staticStream = false)
		{
			VertexStreamPtr stream;

			switch (type)
			{
			case VST_Positions:
			case VST_Normals:
				stream = VertexStreamPtr(new D3D9VertexStream(type, sizeof(float), 3, staticStream));
				break;

			case VST_Diffuses:
			case VST_Speculars:
				stream = VertexStreamPtr(new D3D9VertexStream(type, sizeof(D3DCOLOR), 1, staticStream));
				break;

			case VST_BlendWeights:
				stream = VertexStreamPtr(new D3D9VertexStream(type, sizeof(float), 4, staticStream));
				break;

			case VST_BlendIndices:
				stream = VertexStreamPtr(new D3D9VertexStream(type, sizeof(U8), 4, staticStream));
				break;

			case VST_TextureCoords0:
			case VST_TextureCoords1:
			case VST_TextureCoords2:
			case VST_TextureCoords3:
			case VST_TextureCoords4:
			case VST_TextureCoords5:
			case VST_TextureCoords6:
			case VST_TextureCoords7:
				stream = VertexStreamPtr(new D3D9VertexStream(type, sizeof(float), numElement, staticStream));
				break;

			default:
				stream = VertexStreamPtr(new D3D9VertexStream(type, sizeElement, numElement, staticStream));
				break;
			}

			return stream;
		}

		IndexStreamPtr MakeIndexStream(bool staticStream = false)
		{
			return IndexStreamPtr(new D3D9IndexStream(staticStream));
		}
	};

	RenderFactory& D3D9RenderFactoryInstance()
	{
		static D3D9RenderFactory renderFactory;
		return renderFactory;
	}
}
