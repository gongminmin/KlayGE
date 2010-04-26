// OGLES2RenderFactory.cpp
// KlayGE OpenGL ES 2渲染工厂类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/OpenGLES2/OGLES2RenderEngine.hpp>
#include <KlayGE/OpenGLES2/OGLES2Texture.hpp>
#include <KlayGE/OpenGLES2/OGLES2FrameBuffer.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderLayout.hpp>
#include <KlayGE/OpenGLES2/OGLES2GraphicsBuffer.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderView.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderStateObject.hpp>
#include <KlayGE/OpenGLES2/OGLES2ShaderObject.hpp>

#include <KlayGE/OpenGLES2/OGLES2RenderFactory.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderFactoryInternal.hpp>

namespace KlayGE
{
	OGLES2RenderFactory::OGLES2RenderFactory()
	{
	}

	std::wstring const & OGLES2RenderFactory::Name() const
	{
		static std::wstring const name(L"OpenGL ES 2 Render Factory");
		return name;
	}

	TexturePtr OGLES2RenderFactory::MakeTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLES2Texture1D>(width, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}

	TexturePtr OGLES2RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLES2Texture2D>(width, height, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}

	TexturePtr OGLES2RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLES2Texture3D>(width, height, depth, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}

	TexturePtr OGLES2RenderFactory::MakeTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLES2TextureCube>(size, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}

	FrameBufferPtr OGLES2RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<OGLES2FrameBuffer>(true);
	}

	RenderLayoutPtr OGLES2RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<OGLES2RenderLayout>();
	}

	GraphicsBufferPtr OGLES2RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data, ElementFormat /*fmt*/)
	{
		return MakeSharedPtr<OGLES2GraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, init_data);
	}

	GraphicsBufferPtr OGLES2RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data, ElementFormat /*fmt*/)
	{
		return MakeSharedPtr<OGLES2GraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, init_data);
	}

	QueryPtr OGLES2RenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr();
	}

	QueryPtr OGLES2RenderFactory::MakeConditionalRender()
	{
		return QueryPtr();
	}

	RenderViewPtr OGLES2RenderFactory::Make1DRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<OGLES2Texture1DRenderView>(texture, array_index, level);
	}

	RenderViewPtr OGLES2RenderFactory::Make2DRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<OGLES2Texture2DRenderView>(texture, array_index, level);
	}

	RenderViewPtr OGLES2RenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLES2TextureCubeRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr OGLES2RenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return MakeSharedPtr<OGLES2Texture3DRenderView>(texture, array_index, slice, level);
	}
	
	RenderViewPtr OGLES2RenderFactory::MakeCubeRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLES2RenderFactory::Make3DRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLES2RenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& /*gbuffer*/, uint32_t /*width*/, uint32_t /*height*/, ElementFormat /*pf*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLES2RenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<OGLES2DepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr OGLES2RenderFactory::Make1DDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		return this->Make2DDepthStencilRenderView(texture, array_index, level);
	}

	RenderViewPtr OGLES2RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		return MakeSharedPtr<OGLES2DepthStencilRenderView>(texture, array_index, level);
	}

	RenderViewPtr OGLES2RenderFactory::Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, Texture::CubeFaces /*face*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLES2RenderFactory::Make2DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*slice*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLES2RenderFactory::MakeCubeDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLES2RenderFactory::Make3DDepthStencilRenderView(Texture& /*texture*/, int /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/)
	{
		return RenderViewPtr();
	}

	ShaderObjectPtr OGLES2RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLES2ShaderObject>();
	}

	RenderEnginePtr OGLES2RenderFactory::DoMakeRenderEngine()
	{
		return MakeSharedPtr<OGLES2RenderEngine>();
	}

	RasterizerStateObjectPtr OGLES2RenderFactory::DoMakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLES2RasterizerStateObject>(desc);
	}

	DepthStencilStateObjectPtr OGLES2RenderFactory::DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		return MakeSharedPtr<OGLES2DepthStencilStateObject>(desc);
	}

	BlendStateObjectPtr OGLES2RenderFactory::DoMakeBlendStateObject(BlendStateDesc const & desc)
	{
		return MakeSharedPtr<OGLES2BlendStateObject>(desc);
	}

	SamplerStateObjectPtr OGLES2RenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLES2SamplerStateObject>(desc);
	}
}

void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr, KlayGE::XMLNodePtr const & /*extra_param*/)
{
	ptr = KlayGE::MakeSharedPtr<KlayGE::OGLES2RenderFactory>();
}
