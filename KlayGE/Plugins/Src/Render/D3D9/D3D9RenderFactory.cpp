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
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

#include <boost/bind.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

namespace KlayGE
{
	D3D9RenderFactory::D3D9RenderFactory()
		: D3D9RenderFactoryBase(L"Direct3D9 Render Factory")
		{ }

	TexturePtr D3D9RenderFactory::MakeTexture(uint32_t width, uint32_t height, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, height, numMipMaps, format, usage));
		texture_pool_.push_back(ret);
		return ret;
	}

	RenderTexturePtr D3D9RenderFactory::MakeRenderTexture(uint32_t width, uint32_t height)
	{
		D3D9RenderTexturePtr ret(new D3D9RenderTexture(width, height));
		render_texture_pool_.push_back(ret);
		return ret;
	}

	RenderEffectPtr D3D9RenderFactory::MakeRenderEffect(std::string const & srcData, uint32_t flags)
	{
		D3D9RenderEffectPtr ret(new D3D9RenderEffect(srcData, flags));
		render_effect_pool_.push_back(ret);
		return ret;
	}
		
	VertexStreamPtr D3D9RenderFactory::MakeVertexStream(VertexStreamType type,
		uint8_t sizeElement, uint8_t numElement, bool staticStream)
	{
		D3D9VertexStreamPtr ret;

		switch (type)
		{
		case VST_Positions:
		case VST_Normals:
			ret.reset(new D3D9VertexStream(type, sizeof(float), 3, staticStream));
			break;

		case VST_Diffuses:
		case VST_Speculars:
			ret.reset(new D3D9VertexStream(type, sizeof(D3DCOLOR), 1, staticStream));
			break;

		case VST_BlendWeights:
			ret.reset(new D3D9VertexStream(type, sizeof(float), 4, staticStream));
			break;

		case VST_BlendIndices:
			ret.reset(new D3D9VertexStream(type, sizeof(uint8_t), 4, staticStream));
			break;

		case VST_TextureCoords0:
		case VST_TextureCoords1:
		case VST_TextureCoords2:
		case VST_TextureCoords3:
		case VST_TextureCoords4:
		case VST_TextureCoords5:
		case VST_TextureCoords6:
		case VST_TextureCoords7:
			ret.reset(new D3D9VertexStream(type, sizeof(float), numElement, staticStream));
			break;

		default:
			ret.reset(new D3D9VertexStream(type, sizeElement, numElement, staticStream));
			break;
		}

		vertex_stream_pool_.push_back(ret);
		return ret;
	}

	IndexStreamPtr D3D9RenderFactory::MakeIndexStream(bool staticStream)
	{
		D3D9IndexStreamPtr ret(new D3D9IndexStream(staticStream));
		index_stream_pool_.push_back(ret);
		return ret;
	}

	void D3D9RenderFactory::OnLostDevice()
	{
		for (std::vector<D3D9TexturePtr>::iterator iter = texture_pool_.begin();
			iter != texture_pool_.end(); ++ iter)
		{
			(*iter)->OnLostDevice();
		}
		for (std::vector<D3D9RenderTexturePtr>::iterator iter = render_texture_pool_.begin();
			iter != render_texture_pool_.end(); ++ iter)
		{
			(*iter)->OnLostDevice();
		}
		for (std::vector<D3D9RenderEffectPtr>::iterator iter = render_effect_pool_.begin();
			iter != render_effect_pool_.end(); ++ iter)
		{
			(*iter)->OnLostDevice();
		}
		for (std::vector<D3D9VertexStreamPtr>::iterator iter = vertex_stream_pool_.begin();
			iter != vertex_stream_pool_.end(); ++ iter)
		{
			(*iter)->OnLostDevice();
		}
		for (std::vector<D3D9IndexStreamPtr>::iterator iter = index_stream_pool_.begin();
			iter != index_stream_pool_.end(); ++ iter)
		{
			(*iter)->OnLostDevice();
		}
	}

	void D3D9RenderFactory::OnResetDevice()
	{
		for (std::vector<D3D9TexturePtr>::iterator iter = texture_pool_.begin();
			iter != texture_pool_.end(); ++ iter)
		{
			(*iter)->OnResetDevice();
		}
		for (std::vector<D3D9RenderTexturePtr>::iterator iter = render_texture_pool_.begin();
			iter != render_texture_pool_.end(); ++ iter)
		{
			(*iter)->OnResetDevice();
		}
		for (std::vector<D3D9RenderEffectPtr>::iterator iter = render_effect_pool_.begin();
			iter != render_effect_pool_.end(); ++ iter)
		{
			(*iter)->OnResetDevice();
		}
		for (std::vector<D3D9VertexStreamPtr>::iterator iter = vertex_stream_pool_.begin();
			iter != vertex_stream_pool_.end(); ++ iter)
		{
			(*iter)->OnResetDevice();
		}
		for (std::vector<D3D9IndexStreamPtr>::iterator iter = index_stream_pool_.begin();
			iter != index_stream_pool_.end(); ++ iter)
		{
			(*iter)->OnResetDevice();
		}
	}

	RenderFactory& D3D9RenderFactoryInstance()
	{
		static D3D9RenderFactory renderFactory;
		return renderFactory;
	}
}
