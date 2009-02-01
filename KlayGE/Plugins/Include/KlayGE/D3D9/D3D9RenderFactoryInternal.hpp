// D3D9RenderFactoryInternal.hpp
// KlayGE D3D9渲染引擎抽象工厂 内部头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2008.10.12)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERFACTORYINTERNAL_HPP
#define _D3D9RENDERFACTORYINTERNAL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D9/D3D9Resource.hpp>

#include <boost/weak_ptr.hpp>

namespace KlayGE
{
	class D3D9RenderFactory : public RenderFactory
	{
	public:
		D3D9RenderFactory();

		std::wstring const & Name() const;

		TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);
		TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);
		FrameBufferPtr MakeFrameBuffer();

		RenderLayoutPtr MakeRenderLayout();
		GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data);
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data);

		QueryPtr MakeOcclusionQuery();
		QueryPtr MakeConditionalRender();

		RenderViewPtr Make1DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, int level);
		RenderViewPtr Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level);
		RenderViewPtr Make3DRenderView(Texture& texture, uint32_t slice, int level);
		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf);
		RenderViewPtr MakeDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
		RenderViewPtr MakeDepthStencilRenderView(Texture& texture, int level);

		ShaderObjectPtr MakeShaderObject();

		void OnLostDevice();
		void OnResetDevice();

	private:
		RenderEnginePtr DoMakeRenderEngine();

		RasterizerStateObjectPtr DoMakeRasterizerStateObject(RasterizerStateDesc const & desc);
		DepthStencilStateObjectPtr DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc);
		BlendStateObjectPtr DoMakeBlendStateObject(BlendStateDesc const & desc);
		SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc);

	private:
		std::vector<boost::weak_ptr<D3D9Resource> > resource_pool_;

	private:
		D3D9RenderFactory(D3D9RenderFactory const & rhs);
		D3D9RenderFactory& operator=(D3D9RenderFactory const & rhs);
	};
}

#endif			// _D3D9RENDERFACTORYINTERNAL_HPP
