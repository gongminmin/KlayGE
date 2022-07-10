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
#include <KFL/ErrorHandling.hpp>
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
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr OGLRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr OGLRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_UNIFORM_BUFFER, size_in_byte, structure_byte_stride);
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

	ShaderResourceViewPtr OGLRenderFactory::MakeTextureSrv(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(first_level);
		KFL_UNUSED(num_levels);
		return MakeSharedPtr<OGLTextureShaderResourceView>(texture);
	}

	ShaderResourceViewPtr OGLRenderFactory::MakeTexture2DSrv(
		TexturePtr const& texture, ElementFormat pf, int array_index, Texture::CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(first_level);
		KFL_UNUSED(num_levels);
		return MakeSharedPtr<OGLTextureShaderResourceView>(texture);
	}

	ShaderResourceViewPtr OGLRenderFactory::MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		KFL_UNUSED(first_elem);
		KFL_UNUSED(num_elems);
		return MakeSharedPtr<OGLBufferShaderResourceView>(gbuffer, pf);
	}

	RenderTargetViewPtr OGLRenderFactory::Make1DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<OGLTexture1DRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr OGLRenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<OGLTexture2DRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr OGLRenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLTextureCubeRenderTargetView>(texture, pf, array_index, face, level);
	}

	RenderTargetViewPtr OGLRenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
		int level)
	{
		return MakeSharedPtr<OGLTexture3DRenderTargetView>(texture, pf, array_index, slice, level);
	}

	RenderTargetViewPtr OGLRenderFactory::Make3DRtv(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderTargetViewPtr();
	}

	RenderTargetViewPtr OGLRenderFactory::MakeCubeRtv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		return MakeSharedPtr<OGLTextureCubeRenderTargetView>(texture, pf, array_index, level);
	}

	RenderTargetViewPtr OGLRenderFactory::MakeBufferRtv(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		return MakeSharedPtr<OGLGraphicsBufferRenderTargetView>(gbuffer, pf, first_elem, num_elems);
	}

	DepthStencilViewPtr OGLRenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
		uint32_t sample_quality)
	{
		return MakeSharedPtr<OGLTextureDepthStencilView>(width, height, pf, sample_count, sample_quality);
	}

	DepthStencilViewPtr OGLRenderFactory::Make1DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return this->Make2DDsv(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr OGLRenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<OGLTextureDepthStencilView>(texture, pf, first_array_index, array_size, level);
	}
	
	DepthStencilViewPtr OGLRenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLTextureCubeFaceDepthStencilView>(texture, pf, array_index, face, level);
	}

	DepthStencilViewPtr OGLRenderFactory::Make2DDsv(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*slice*/, int /*level*/)
	{
		return DepthStencilViewPtr();
	}

	DepthStencilViewPtr OGLRenderFactory::Make3DDsv(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return DepthStencilViewPtr();
	}

	DepthStencilViewPtr OGLRenderFactory::MakeCubeDsv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		return MakeSharedPtr<OGLTextureDepthStencilView>(texture, pf, array_index, 1, level);
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make1DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/,
		int /*first_array_index*/, int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make2DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/,
		int /*first_array_index*/, int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make2DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/,
		int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make2DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/,
		int /*array_index*/, uint32_t /*slice*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::Make3DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/,
		int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::MakeCubeUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/,
		int /*array_index*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLRenderFactory::MakeBufferUav(GraphicsBufferPtr const & /*gbuffer*/,
		ElementFormat /*pf*/, uint32_t /*first_elem*/, uint32_t /*num_elems*/)
	{
		return UnorderedAccessViewPtr();
	}

	ShaderObjectPtr OGLRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLShaderObject>();
	}

	ShaderStageObjectPtr OGLRenderFactory::MakeShaderStageObject(ShaderStage stage)
	{
		std::shared_ptr<OGLShaderStageObject> ret;
		switch (stage)
		{
		case ShaderStage::Vertex:
			ret = MakeSharedPtr<OGLVertexShaderStageObject>();
			break;

		case ShaderStage::Pixel:
			ret = MakeSharedPtr<OGLPixelShaderStageObject>();
			break;

		case ShaderStage::Geometry:
			ret = MakeSharedPtr<OGLGeometryShaderStageObject>();
			break;

		case ShaderStage::Compute:
			ret = MakeSharedPtr<OGLComputeShaderStageObject>();
			break;

		case ShaderStage::Hull:
			ret = MakeSharedPtr<OGLHullShaderStageObject>();
			break;

		case ShaderStage::Domain:
			ret = MakeSharedPtr<OGLDomainShaderStageObject>();
			break;

		default:
			KFL_UNREACHABLE("Invalid shader stage");
		}
		return ret;
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
