// OGLESRenderFactory.cpp
// KlayGE OpenGL ES 2��Ⱦ������ ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
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
#include <KlayGE/OpenGLES/OGLESRenderFactoryInternal.hpp>

namespace KlayGE
{
	OGLESRenderFactory::OGLESRenderFactory()
	{
	}

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
			uint32_t size_in_byte, ElementFormat /*fmt*/)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, size_in_byte);
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat /*fmt*/)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, size_in_byte);
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		KFL_UNUSED(fmt);

		GraphicsBufferPtr ret;
		if (glloader_GLES_VERSION_3_0())
		{
			ret = MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_UNIFORM_BUFFER, size_in_byte);
		}
		return ret;
	}

	QueryPtr OGLESRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr();
	}

	QueryPtr OGLESRenderFactory::MakeConditionalRender()
	{
		if (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_occlusion_query_boolean())
		{
			return MakeSharedPtr<OGLESConditionalRender>();
		}
		else
		{
			return QueryPtr();
		}
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

	FencePtr OGLESRenderFactory::MakeFence()
	{
		FencePtr ret;
		if (glloader_GLES_VERSION_3_0())
		{
			ret = MakeSharedPtr<OGLESFence>();
		}
		return ret;
	}

	RenderViewPtr OGLESRenderFactory::Make1DRenderView(Texture& texture, int first_array_index, int /*array_size*/, int level)
	{
		return MakeSharedPtr<OGLESTexture1DRenderView>(texture, first_array_index, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DRenderView(Texture& texture, int first_array_index, int /*array_size*/, int level)
	{
		return MakeSharedPtr<OGLESTexture2DRenderView>(texture, first_array_index, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLESTextureCubeRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return MakeSharedPtr<OGLESTexture3DRenderView>(texture, array_index, slice, level);
	}
	
	RenderViewPtr OGLESRenderFactory::MakeCubeRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::Make3DRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& /*gbuffer*/, uint32_t /*width*/, uint32_t /*height*/, ElementFormat /*pf*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<OGLESDepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr OGLESRenderFactory::Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return this->Make2DDepthStencilRenderView(texture, first_array_index, array_size, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int /*array_size*/, int level)
	{
		return MakeSharedPtr<OGLESDepthStencilRenderView>(texture, first_array_index, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLESTextureCubeDepthStencilRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::MakeCubeDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::Make3DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make1DUnorderedAccessView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUnorderedAccessView(Texture& /*texture*/, int /*first_array_index*/, int /*array_size*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::MakeCubeUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make3DUnorderedAccessView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& /*gbuffer*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	ShaderObjectPtr OGLESRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLESShaderObject>();
	}

	std::unique_ptr<RenderEngine> OGLESRenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<OGLESRenderEngine>();
	}

	RasterizerStateObjectPtr OGLESRenderFactory::DoMakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLESRasterizerStateObject>(desc);
	}

	DepthStencilStateObjectPtr OGLESRenderFactory::DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		return MakeSharedPtr<OGLESDepthStencilStateObject>(desc);
	}

	BlendStateObjectPtr OGLESRenderFactory::DoMakeBlendStateObject(BlendStateDesc const & desc)
	{
		return MakeSharedPtr<OGLESBlendStateObject>(desc);
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

void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
{
	ptr = KlayGE::MakeUniquePtr<KlayGE::OGLESRenderFactory>();
}
