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

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

namespace KlayGE
{
	D3D9RenderFactory::D3D9RenderFactory()
		: D3D9RenderFactoryBase(L"Direct3D9 Render Factory")
		{ }

	TexturePtr D3D9RenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, numMipMaps, format, usage));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			PixelFormat format, Texture::TextureUsage usage)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, height, numMipMaps, format, usage));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, 
			uint16_t numMipMaps, PixelFormat format, Texture::TextureUsage usage)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, height, depth, numMipMaps, format, usage));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
		PixelFormat format, Texture::TextureUsage usage)
	{
		D3D9TexturePtr ret(new D3D9Texture(size, true, numMipMaps, format, usage));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderTexturePtr D3D9RenderFactory::MakeRenderTexture()
	{
		D3D9RenderTexturePtr ret(new D3D9RenderTexture);
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderEffectPtr D3D9RenderFactory::DoMakeRenderEffect(std::string const & srcData)
	{
		D3D9RenderEffectPtr ret(new D3D9RenderEffect(srcData));
		resource_pool_.push_back(ret);
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

		resource_pool_.push_back(ret);
		return ret;
	}

	IndexStreamPtr D3D9RenderFactory::MakeIndexStream(bool staticStream)
	{
		D3D9IndexStreamPtr ret(new D3D9IndexStream(staticStream));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderVertexStreamPtr D3D9RenderFactory::MakeRenderVertexStream(uint32_t width, uint32_t height)
	{
		D3D9RenderVertexStreamPtr ret(new D3D9RenderVertexStream(width, height));
		resource_pool_.push_back(ret);
		return ret;
	}

	void D3D9RenderFactory::OnLostDevice()
	{
		for (std::vector<boost::weak_ptr<D3D9Resource> >::iterator iter = resource_pool_.begin();
			iter != resource_pool_.end();)
		{
			D3D9ResourcePtr res = iter->lock();
			if (res)
			{
				res->OnLostDevice();
				++ iter;
			}
			else
			{
				iter = resource_pool_.erase(iter);
			}
		}

		D3D9RenderEngine& engine = static_cast<D3D9RenderEngine&>(this->RenderEngineInstance());
		engine.OnLostDevice();
	}

	void D3D9RenderFactory::OnResetDevice()
	{
		D3D9RenderEngine& engine = static_cast<D3D9RenderEngine&>(this->RenderEngineInstance());
		engine.OnResetDevice();

		for (std::vector<boost::weak_ptr<D3D9Resource> >::iterator iter = resource_pool_.begin();
			iter != resource_pool_.end();)
		{
			D3D9ResourcePtr res = iter->lock();
			if (res)
			{
				res->OnResetDevice();
				++ iter;
			}
			else
			{
				iter = resource_pool_.erase(iter);
			}
		}
	}

	RenderFactory& D3D9RenderFactoryInstance()
	{
		static D3D9RenderFactory renderFactory;
		return renderFactory;
	}
}
