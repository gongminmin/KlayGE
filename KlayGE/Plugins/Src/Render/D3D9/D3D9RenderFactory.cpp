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
#include <KlayGE/D3D9/D3D9RenderLayout.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>
#include <KlayGE/D3D9/D3D9OcclusionQuery.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

namespace KlayGE
{
	D3D9RenderFactory::D3D9RenderFactory()
		: D3D9RenderFactoryBase(L"Direct3D9 Render Factory")
	{
	}

	TexturePtr D3D9RenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			PixelFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			PixelFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, height, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, 
			uint16_t numMipMaps, PixelFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture(width, height, depth, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
		PixelFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture(size, true, numMipMaps, format));
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

	RenderLayoutPtr D3D9RenderFactory::MakeRenderLayout(RenderLayout::buffer_type type)
	{
		return RenderLayoutPtr(new D3D9RenderLayout(type));
	}

	GraphicsBufferPtr D3D9RenderFactory::MakeVertexBuffer(BufferUsage usage)
	{
		D3D9VertexStreamPtr ret(new D3D9VertexStream(usage));
		resource_pool_.push_back(ret);
		return ret;
	}

	GraphicsBufferPtr D3D9RenderFactory::MakeIndexBuffer(BufferUsage usage)
	{
		D3D9IndexStreamPtr ret(new D3D9IndexStream(usage));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderVertexStreamPtr D3D9RenderFactory::MakeRenderVertexStream(uint32_t width, uint32_t height)
	{
		D3D9RenderVertexStreamPtr ret(new D3D9RenderVertexStream(width, height));
		resource_pool_.push_back(ret);
		return ret;
	}

	QueryPtr D3D9RenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr(new D3D9OcclusionQuery);
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
