// OGLRenderFactory.cpp
// KlayGE OpenGL渲染工厂类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 可以建立静态OGLVertexStream和OGLIndexStream (2005.6.19)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLQuery.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactoryInternal.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "Cg.lib")
#pragma comment(lib, "CgGL.lib")
#endif

namespace KlayGE
{
	OGLRenderFactory::OGLRenderFactory()
	{
		context_ = cgCreateContext();
		cgSetParameterSettingMode(context_, CG_DEFERRED_PARAMETER_SETTING);
	}

	CGcontext OGLRenderFactory::CGContext() const
	{
		return context_;
	}

	std::wstring const & OGLRenderFactory::Name() const
	{
		static std::wstring const name(L"OpenGL Render Factory");
		return name;
	}

	TexturePtr OGLRenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps, ElementFormat format, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLTexture1D>(width, numMipMaps, format, access_hint, init_data);
	}

	TexturePtr OGLRenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				ElementFormat format, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLTexture2D>(width, height, numMipMaps, format, access_hint, init_data);
	}

	TexturePtr OGLRenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, ElementFormat format, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLTexture3D>(width, height, depth, numMipMaps, format, access_hint, init_data);
	}

	TexturePtr OGLRenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				ElementFormat format, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLTextureCube>(size, numMipMaps, format, access_hint, init_data);
	}

	FrameBufferPtr OGLRenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<OGLFrameBuffer>(true);
	}

	RenderLayoutPtr OGLRenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<OGLRenderLayout>();
	}

	GraphicsBufferPtr OGLRenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, init_data);
	}

	GraphicsBufferPtr OGLRenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData* init_data)
	{
		return MakeSharedPtr<OGLGraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, init_data);
	}

	QueryPtr OGLRenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<OGLOcclusionQuery>();
	}

	QueryPtr OGLRenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<OGLConditionalRender>();
	}

	RenderViewPtr OGLRenderFactory::Make1DRenderView(Texture& texture, int level)
	{
		return MakeSharedPtr<OGLTexture1DRenderView>(texture, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, int level)
	{
		return MakeSharedPtr<OGLTexture2DRenderView>(texture, level);
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<OGLTextureCubeRenderView>(texture, face, level);
	}

	RenderViewPtr OGLRenderFactory::Make3DRenderView(Texture& texture, uint32_t slice, int level)
	{
		return MakeSharedPtr<OGLTexture3DRenderView>(texture, slice, level);
	}

	RenderViewPtr OGLRenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf)
	{
		return MakeSharedPtr<OGLGraphicsBufferRenderView>(gbuffer, width, height, pf);
	}

	RenderViewPtr OGLRenderFactory::MakeDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t multi_sample)
	{
		return MakeSharedPtr<OGLDepthStencilRenderView>(width, height, pf, multi_sample);
	}

	ShaderObjectPtr OGLRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLShaderObject>();
	}

	RenderEnginePtr OGLRenderFactory::DoMakeRenderEngine()
	{
		return MakeSharedPtr<OGLRenderEngine>();
	}

	RasterizerStateObjectPtr OGLRenderFactory::DoMakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLRasterizerStateObject>(desc);
	}
	
	DepthStencilStateObjectPtr OGLRenderFactory::DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		return MakeSharedPtr<OGLDepthStencilStateObject>(desc);
	}

	BlendStateObjectPtr OGLRenderFactory::DoMakeBlendStateObject(BlendStateDesc const & desc)
	{
		return MakeSharedPtr<OGLBlendStateObject>(desc);
	}

	SamplerStateObjectPtr OGLRenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLSamplerStateObject>(desc);
	}
}

extern "C"
{
	void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr, boost::program_options::variables_map const & /*vm*/)
	{
		ptr = KlayGE::MakeSharedPtr<KlayGE::OGLRenderFactory>();
	}

	bool Match(char const * name, char const * compiler)
	{
		std::string cur_compiler_str = KLAYGE_COMPILER_TOOLSET;
#ifdef KLAYGE_DEBUG
		cur_compiler_str += "_d";
#endif

		if ((std::string("OpenGL") == name) && (cur_compiler_str == compiler))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
