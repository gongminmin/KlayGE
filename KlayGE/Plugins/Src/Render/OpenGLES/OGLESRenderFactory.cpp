// OGLESRenderFactory.cpp
// KlayGE OpenGL ES 2渲染工厂类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderLayout.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESQuery.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>
#include <KlayGE/OpenGLES/OGLESFence.hpp>

#include <KlayGE/OpenGLES/OGLESRenderFactory.hpp>

namespace KlayGE
{
	OGLESRenderFactory::OGLESRenderFactory() = default;

	std::wstring const & OGLESRenderFactory::Name() const
	{
		static std::wstring const name(L"OpenGL ES Render Factory");
		return name;
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTexture1D>(width, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTexture2D>(width, height, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTexture3D>(width, height, depth, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTextureCube>(size, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr OGLESRenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<OGLESFrameBuffer>(true);
	}

	RenderLayoutPtr OGLESRenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<OGLESRenderLayout>();
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, size_in_byte, structure_byte_stride);
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, uint32_t structure_byte_stride)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_UNIFORM_BUFFER, size_in_byte, structure_byte_stride);
	}

	QueryPtr OGLESRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr();
	}

	QueryPtr OGLESRenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<OGLESConditionalRender>();
	}

	QueryPtr OGLESRenderFactory::MakeTimerQuery()
	{
		if (glloader_GLES_EXT_disjoint_timer_query())
		{
			return MakeSharedPtr<OGLESTimerQuery>();
		}
		else
		{
			return QueryPtr();
		}
	}

	QueryPtr OGLESRenderFactory::MakeSOStatisticsQuery()
	{
		return MakeSharedPtr<OGLESSOStatisticsQuery>();
	}

	FencePtr OGLESRenderFactory::MakeFence()
	{
		return MakeSharedPtr<OGLESFence>();
	}

	ShaderResourceViewPtr OGLESRenderFactory::MakeTextureSrv(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(first_level);
		KFL_UNUSED(num_levels);
		return MakeSharedPtr<OGLESTextureShaderResourceView>(texture);
	}

	ShaderResourceViewPtr OGLESRenderFactory::MakeTexture2DSrv(
		TexturePtr const& texture, ElementFormat pf, int array_index, Texture::CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(first_level);
		KFL_UNUSED(num_levels);
		return MakeSharedPtr<OGLESTextureShaderResourceView>(texture);
	}

	ShaderResourceViewPtr OGLESRenderFactory::MakeBufferSrv(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		KFL_UNUSED(first_elem);
		KFL_UNUSED(num_elems);
		return MakeSharedPtr<OGLESBufferShaderResourceView>(gbuffer, pf);
	}

	RenderTargetViewPtr OGLESRenderFactory::Make1DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<OGLESTexture1DRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr OGLESRenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<OGLESTexture2DRenderTargetView>(texture, pf, first_array_index, array_size, level);
	}

	RenderTargetViewPtr OGLESRenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLESTextureCubeRenderTargetView>(texture, pf, array_index, face, level);
	}

	RenderTargetViewPtr OGLESRenderFactory::Make2DRtv(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
		int level)
	{
		return MakeSharedPtr<OGLESTexture3DRenderTargetView>(texture, pf, array_index, slice, level);
	}

	RenderTargetViewPtr OGLESRenderFactory::Make3DRtv(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderTargetViewPtr();
	}

	RenderTargetViewPtr OGLESRenderFactory::MakeCubeRtv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		return MakeSharedPtr<OGLESTextureCubeRenderTargetView>(texture, pf, array_index, level);
	}

	RenderTargetViewPtr OGLESRenderFactory::MakeBufferRtv(GraphicsBufferPtr const & /*gbuffer*/, ElementFormat /*pf*/,
		uint32_t /*first_elem*/, uint32_t /*num_elems*/)
	{
		return RenderTargetViewPtr();
	}

	DepthStencilViewPtr OGLESRenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
		uint32_t sample_quality)
	{
		return MakeSharedPtr<OGLESTextureDepthStencilView>(width, height, pf, sample_count, sample_quality);
	}

	DepthStencilViewPtr OGLESRenderFactory::Make1DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return this->Make2DDsv(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr OGLESRenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
	{
		return MakeSharedPtr<OGLESTextureDepthStencilView>(texture, pf, first_array_index, array_size, level);
	}

	DepthStencilViewPtr OGLESRenderFactory::Make2DDsv(TexturePtr const & texture, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLESTextureCubeFaceDepthStencilView>(texture, pf, array_index, face, level);
	}

	DepthStencilViewPtr OGLESRenderFactory::Make2DDsv(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*slice*/, int /*level*/)
	{
		return DepthStencilViewPtr();
	}

	DepthStencilViewPtr OGLESRenderFactory::Make3DDsv(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return DepthStencilViewPtr();
	}

	DepthStencilViewPtr OGLESRenderFactory::MakeCubeDsv(TexturePtr const & texture, ElementFormat pf, int array_index, int level)
	{
		return MakeSharedPtr<OGLESTextureDepthStencilView>(texture, pf, array_index, 1, level);
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make1DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*first_array_index*/,
		int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*first_array_index*/,
		int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		Texture::CubeFaces /*face*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*slice*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make3DUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::MakeCubeUav(TexturePtr const & /*texture*/, ElementFormat /*pf*/, int /*array_index*/,
		int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::MakeBufferUav(GraphicsBufferPtr const & /*gbuffer*/,
		ElementFormat /*pf*/, uint32_t /*first_elem*/, uint32_t /*num_elems*/)
	{
		return UnorderedAccessViewPtr();
	}

	ShaderObjectPtr OGLESRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLESShaderObject>();
	}

	ShaderStageObjectPtr OGLESRenderFactory::MakeShaderStageObject(ShaderStage stage)
	{
		std::shared_ptr<OGLESShaderStageObject> ret;
		switch (stage)
		{
		case ShaderStage::Vertex:
			ret = MakeSharedPtr<OGLESVertexShaderStageObject>();
			break;

		case ShaderStage::Pixel:
			ret = MakeSharedPtr<OGLESPixelShaderStageObject>();
			break;

		case ShaderStage::Geometry:
			ret = MakeSharedPtr<OGLESGeometryShaderStageObject>();
			break;

		case ShaderStage::Compute:
			ret = MakeSharedPtr<OGLESComputeShaderStageObject>();
			break;

		case ShaderStage::Hull:
			ret = MakeSharedPtr<OGLESHullShaderStageObject>();
			break;

		case ShaderStage::Domain:
			ret = MakeSharedPtr<OGLESDomainShaderStageObject>();
			break;

		default:
			KFL_UNREACHABLE("Invalid shader stage");
		}
		return ret;
	}

	std::unique_ptr<RenderEngine> OGLESRenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<OGLESRenderEngine>();
	}

	RenderStateObjectPtr OGLESRenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
		BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<OGLESRenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr OGLESRenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLESSamplerStateObject>(desc);
	}

	void OGLESRenderFactory::DoSuspend()
	{
		// TODO
	}

	void OGLESRenderFactory::DoResume()
	{
		// TODO
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::OGLESRenderFactory>();
	}
}
