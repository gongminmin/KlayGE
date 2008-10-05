// D3D10RenderFactory.hpp
// KlayGE D3D10渲染引擎抽象工厂 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10RENDERFACTORY_HPP
#define _D3D10RENDERFACTORY_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D10
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

namespace KlayGE
{
	RenderFactory& D3D10RenderFactoryInstance();

	class D3D10RenderFactory : public RenderFactory
	{
	public:
		D3D10RenderFactory();

		std::wstring const & Name() const;

		RenderEngine& RenderEngineInstance();

		TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			ElementFormat format, uint32_t access_hint, ElementInitData* init_data);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			ElementFormat format, uint32_t access_hint, ElementInitData* init_data);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps,
			ElementFormat format, uint32_t access_hint, ElementInitData* init_data);
		TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
			ElementFormat format, uint32_t access_hint, ElementInitData* init_data);
		FrameBufferPtr MakeFrameBuffer();

		RenderLayoutPtr MakeRenderLayout();
		GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data);
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data);

		QueryPtr MakeOcclusionQuery();

		RenderViewPtr Make1DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level);
		RenderViewPtr Make3DRenderView(Texture& texture, uint32_t slice, int level);
		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf);
		RenderViewPtr MakeDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t multi_sample);

		ShaderObjectPtr MakeShaderObject();

	private:
		RasterizerStateObjectPtr DoMakeRasterizerStateObject(RasterizerStateDesc const & desc);
		DepthStencilStateObjectPtr DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc);
		BlendStateObjectPtr DoMakeBlendStateObject(BlendStateDesc const & desc);
		SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc);

	private:
		D3D10RenderFactory(D3D10RenderFactory const & rhs);
		D3D10RenderFactory& operator=(D3D10RenderFactory const & rhs);
	};
}

#endif			// _D3D10RENDERFACTORY_HPP
