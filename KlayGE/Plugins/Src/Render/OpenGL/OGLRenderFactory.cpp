// OGLRenderFactory.cpp
// KlayGE OpenGL渲染工厂类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://www.klayge.org
//
// 2.7.0
// 可以建立静态OGLVertexStream和OGLIndexStream (2005.6.19)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLQuery.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>
#include <KlayGE/OpenGL/OGLFence.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

namespace KlayGE
{
	OGLRenderFactory::OGLRenderFactory()
	{
	}

	std::wstring const & OGLRenderFactory::Name() const
	{
		static std::wstring const name(L"OpenGL Render Factory");
		return name;
	}

	TexturePtr OGLRenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLTexture1D>(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLRenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLTexture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLRenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLTexture3D>(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLRenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLTextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr OGLRenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<OGLFrameBuffer>(true);
	}

	RenderLayoutPtr OGLRenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<OGLRenderLayout>();
	}

	GraphicsBufferPtr OGLRenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, size_in_byte, fmt);
	}

	GraphicsBufferPtr OGLRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, size_in_byte, fmt);
	}

	GraphicsBufferPtr OGLRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_UNIFORM_BUFFER, size_in_byte, fmt);
	}

	QueryPtr OGLRenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<OGLOcclusionQuery>();
	}

	QueryPtr OGLRenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<OGLConditionalRender>();
	}

	QueryPtr OGLRenderFactory::MakeTimerQuery()
	{
		return MakeSharedPtr<OGLTimerQuery>();
	}

	QueryPtr OGLRenderFactory::MakeSOStatisticsQuery()
	{
		return MakeSharedPtr<OGLSOStatisticsQuery>();
	}

	FencePtr OGLRenderFactory::MakeFence()
	{
		return MakeSharedPtr<OGLFence>();
	}

	RenderViewPtr OGLRenderFactory::Make1DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<OGLTexture1DRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<OGLTexture2DRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLTextureCubeRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return MakeSharedPtr<OGLTexture3DRenderView>(texture, array_index, slice, level);
	}

	RenderViewPtr OGLRenderFactory::MakeCubeRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<OGLTextureCubeRenderView>(texture, array_index, level);
	}

	RenderViewPtr OGLRenderFactory::Make3DRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLRenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf)
	{
		return MakeSharedPtr<OGLGraphicsBufferRenderView>(gbuffer, width, height, pf);
	}

	RenderViewPtr OGLRenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<OGLDepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr OGLRenderFactory::Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return this->Make2DDepthStencilRenderView(texture, first_array_index, array_size, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<OGLDepthStencilRenderView>(texture, first_array_index, array_size, level);
	}
	
	RenderViewPtr OGLRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLTextureCubeDepthStencilRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLRenderFactory::MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<OGLDepthStencilRenderView>(texture, array_index, 1, level);
	}

	RenderViewPtr OGLRenderFactory::Make3DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make1DUnorderedAccessView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make2DUnorderedAccessView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make2DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make2DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::MakeCubeUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make3DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& /*gbuffer*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	ShaderObjectPtr OGLRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLShaderObject>();
	}

	std::unique_ptr<RenderEngine> OGLRenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<OGLRenderEngine>();
	}

	RenderStateObjectPtr OGLRenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc,
		DepthStencilStateDesc const & dss_desc, BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<OGLRenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr OGLRenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLSamplerStateObject>(desc);
	}

	void OGLRenderFactory::DoSuspend()
	{
		// TODO
	}

	void OGLRenderFactory::DoResume()
	{
		// TODO
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::OGLRenderFactory>();
	}
}
