// D3D11RenderFactory.cpp
// KlayGE D3D11渲染引擎抽象工厂 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Query.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

#include <KlayGE/D3D11/D3D11RenderFactory.hpp>
#include <KlayGE/D3D11/D3D11RenderFactoryInternal.hpp>

namespace KlayGE
{
	D3D11RenderFactory::D3D11RenderFactory()
	{
	}

	std::wstring const & D3D11RenderFactory::Name() const
	{
		static std::wstring const name(L"Direct3D11 Render Factory");
		return name;
	}

	TexturePtr D3D11RenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11Texture1D>(width, numMipMaps, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11Texture2D>(width, height, numMipMaps, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
			uint16_t numMipMaps, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11Texture3D>(width, height, depth, numMipMaps, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11TextureCube>(size, numMipMaps, format, sample_count, sample_quality, access_hint, init_data);
	}

	FrameBufferPtr D3D11RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<D3D11FrameBuffer>();
	}

	RenderLayoutPtr D3D11RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<D3D11RenderLayout>();
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, init_data);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, init_data);
	}

	QueryPtr D3D11RenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<D3D11OcclusionQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<D3D11ConditionalRender>();
	}

	RenderViewPtr D3D11RenderFactory::Make1DRenderView(Texture& texture, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, face, level);
	}

	RenderViewPtr D3D11RenderFactory::Make3DRenderView(Texture& texture, uint32_t slice, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, slice, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer,
		uint32_t width, uint32_t height, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(gbuffer, width, height, pf);
	}

	RenderViewPtr D3D11RenderFactory::MakeDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr D3D11RenderFactory::MakeDepthStencilRenderView(Texture& texture, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, level);
	}

	ShaderObjectPtr D3D11RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<D3D11ShaderObject>();
	}

	RenderEnginePtr D3D11RenderFactory::DoMakeRenderEngine()
	{
		return MakeSharedPtr<D3D11RenderEngine>();
	}

	RasterizerStateObjectPtr D3D11RenderFactory::DoMakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11RasterizerStateObject>(desc);
	}
	
	DepthStencilStateObjectPtr D3D11RenderFactory::DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11DepthStencilStateObject>(desc);
	}

	BlendStateObjectPtr D3D11RenderFactory::DoMakeBlendStateObject(BlendStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11BlendStateObject>(desc);
	}

	SamplerStateObjectPtr D3D11RenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11SamplerStateObject>(desc);
	}
}

void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr, void* /*extra_param*/)
{
	ptr = KlayGE::MakeSharedPtr<KlayGE::D3D11RenderFactory>();
}
