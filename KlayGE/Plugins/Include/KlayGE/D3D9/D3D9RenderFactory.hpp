// D3D9RenderFactory.hpp
// KlayGE D3D9渲染引擎抽象工厂 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 增加了resource_pool_成员 (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERFACTORY_HPP
#define _D3D9RENDERFACTORY_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <boost/weak_ptr.hpp>

#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>

namespace KlayGE
{
	RenderFactory& D3D9RenderFactoryInstance();

	typedef ConcreteRenderFactory<D3D9RenderEngine, D3D9Texture, D3D9FrameBuffer,
			D3D9RenderEffect, D3D9RenderView> D3D9RenderFactoryBase;

	class D3D9RenderFactory : public D3D9RenderFactoryBase
	{
	public:
		D3D9RenderFactory();

		TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			ElementFormat format);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			ElementFormat format);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps,
			ElementFormat format);
		TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
			ElementFormat format);
		FrameBufferPtr MakeFrameBuffer();

		RenderLayoutPtr MakeRenderLayout(RenderLayout::buffer_type type);
		GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage);
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage);

		QueryPtr MakeOcclusionQuery();

		RenderViewPtr Make1DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level);
		RenderViewPtr Make3DRenderView(Texture& texture, uint32_t slice, int level);
		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf);
		RenderViewPtr MakeDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t multi_sample);

		void OnLostDevice();
		void OnResetDevice();

	private:
		RenderEffectPtr DoMakeRenderEffect(std::string const & srcData);

	private:
		std::vector<boost::weak_ptr<D3D9Resource> > resource_pool_;

	private:
		D3D9RenderFactory(D3D9RenderFactory const & rhs);
		D3D9RenderFactory& operator=(D3D9RenderFactory const & rhs);
	};
}

#endif			// _D3D9RENDERFACTORY_HPP
