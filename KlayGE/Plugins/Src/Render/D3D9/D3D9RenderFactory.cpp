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
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>

#include <boost/typeof/typeof.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9RenderLayout.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9OcclusionQuery.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>
#include <KlayGE/D3D9/D3D9ShaderObject.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

namespace KlayGE
{
	D3D9RenderFactory::D3D9RenderFactory()
	{
	}

	std::wstring const & D3D9RenderFactory::Name() const
	{
		static std::wstring const name(L"Direct3D9 Render Factory");
		return name;
	}

	RenderEngine& D3D9RenderFactory::RenderEngineInstance()
	{
		static D3D9RenderEngine renderEngine;
		return renderEngine;
	}

	TexturePtr D3D9RenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			ElementFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture1D(width, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			ElementFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture2D(width, height, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
			uint16_t numMipMaps, ElementFormat format)
	{
		D3D9TexturePtr ret(new D3D9Texture3D(width, height, depth, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}
	TexturePtr D3D9RenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
		ElementFormat format)
	{
		D3D9TexturePtr ret(new D3D9TextureCube(size, numMipMaps, format));
		resource_pool_.push_back(ret);
		return ret;
	}

	FrameBufferPtr D3D9RenderFactory::MakeFrameBuffer()
	{
		D3D9FrameBufferPtr ret(new D3D9FrameBuffer);
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderLayoutPtr D3D9RenderFactory::MakeRenderLayout()
	{
		return RenderLayoutPtr(new D3D9RenderLayout);
	}

	GraphicsBufferPtr D3D9RenderFactory::MakeVertexBuffer(BufferUsage usage)
	{
		D3D9VertexBufferPtr ret(new D3D9VertexBuffer(usage));
		resource_pool_.push_back(ret);
		return ret;
	}

	GraphicsBufferPtr D3D9RenderFactory::MakeIndexBuffer(BufferUsage usage)
	{
		D3D9IndexBufferPtr ret(new D3D9IndexBuffer(usage));
		resource_pool_.push_back(ret);
		return ret;
	}

	QueryPtr D3D9RenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr(new D3D9OcclusionQuery);
	}

	RenderViewPtr D3D9RenderFactory::Make1DRenderView(Texture& texture, int level)
	{
		D3D9RenderViewPtr ret(new D3D9Texture1DRenderView(texture, level));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderViewPtr D3D9RenderFactory::Make2DRenderView(Texture& texture, int level)
	{
		D3D9RenderViewPtr ret(new D3D9Texture2DRenderView(texture, level));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderViewPtr D3D9RenderFactory::Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level)
	{
		D3D9RenderViewPtr ret(new D3D9TextureCubeRenderView(texture, face, level));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderViewPtr D3D9RenderFactory::Make3DRenderView(Texture& texture, uint32_t slice, int level)
	{
		D3D9RenderViewPtr ret(new D3D9Texture3DRenderView(texture, slice, level));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderViewPtr D3D9RenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer,
		uint32_t width, uint32_t height, ElementFormat pf)
	{
		D3D9RenderViewPtr ret(new D3D9GraphicsBufferRenderView(gbuffer, width, height, pf));
		resource_pool_.push_back(ret);
		return ret;
	}

	RenderViewPtr D3D9RenderFactory::MakeDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t multi_sample)
	{
		D3D9RenderViewPtr ret(new D3D9DepthStencilRenderView(width, height, pf, multi_sample));
		resource_pool_.push_back(ret);
		return ret;
	}

	ShaderObjectPtr D3D9RenderFactory::MakeShaderObject()
	{
		D3D9ShaderObjectPtr ret(new D3D9ShaderObject);
		return ret;
	}

	void D3D9RenderFactory::OnLostDevice()
	{
		for (BOOST_AUTO(iter, resource_pool_.begin()); iter != resource_pool_.end();)
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

		D3D9RenderEngine& engine = *checked_cast<D3D9RenderEngine*>(&this->RenderEngineInstance());
		engine.OnLostDevice();
	}

	void D3D9RenderFactory::OnResetDevice()
	{
		D3D9RenderEngine& engine = *checked_cast<D3D9RenderEngine*>(&this->RenderEngineInstance());
		engine.OnResetDevice();

		for (BOOST_AUTO(iter, resource_pool_.begin()); iter != resource_pool_.end();)
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
