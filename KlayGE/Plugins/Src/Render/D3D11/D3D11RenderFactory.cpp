// D3D11RenderFactory.cpp
// KlayGE D3D11渲染引擎抽象工厂 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
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

	TexturePtr D3D11RenderFactory::MakeTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11Texture1D>(width, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11Texture2D>(width, height, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11Texture3D>(width, height, depth, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<D3D11TextureCube>(size, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}

	FrameBufferPtr D3D11RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<D3D11FrameBuffer>();
	}

	RenderLayoutPtr D3D11RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<D3D11RenderLayout>();
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, init_data, fmt);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, init_data, fmt);
	}

	QueryPtr D3D11RenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<D3D11OcclusionQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<D3D11ConditionalRender>();
	}

	RenderViewPtr D3D11RenderFactory::Make1DRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return this->Make3DRenderView(texture, array_index, slice, 1, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeCubeRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, level);
	}

	RenderViewPtr D3D11RenderFactory::Make3DRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, first_slice, num_slices, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer,
		uint32_t width, uint32_t height, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(gbuffer, width, height, pf);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr D3D11RenderFactory::Make1DDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, face, level);
	}
	
	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return this->Make3DDepthStencilRenderView(texture, array_index, slice, 1, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, level);
	}
	
	RenderViewPtr D3D11RenderFactory::Make3DDepthStencilRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, first_slice, num_slices, level);
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

void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr, KlayGE::XMLNodePtr const & /*extra_param*/)
{
	ptr = KlayGE::MakeSharedPtr<KlayGE::D3D11RenderFactory>();
}
