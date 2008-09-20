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
#include <KlayGE/OpenGL/OGLOcclusionQuery.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "Cg.lib")
#pragma comment(lib, "CgGL.lib")
#endif

namespace KlayGE
{
	OGLRenderFactory::OGLRenderFactory()
	{
		context_ = cgCreateContext();
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

	RenderEngine& OGLRenderFactory::RenderEngineInstance()
	{
		static OGLRenderEngine renderEngine;
		return renderEngine;
	}

	TexturePtr OGLRenderFactory::MakeTexture1D(uint32_t width, uint16_t numMipMaps, ElementFormat format, uint32_t access_hint)
	{
		return TexturePtr(new OGLTexture1D(width, numMipMaps, format, access_hint));
	}

	TexturePtr OGLRenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				ElementFormat format, uint32_t access_hint)
	{
		return TexturePtr(new OGLTexture2D(width, height, numMipMaps, format, access_hint));
	}

	TexturePtr OGLRenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, ElementFormat format, uint32_t access_hint)
	{
		return TexturePtr(new OGLTexture3D(width, height, depth, numMipMaps, format, access_hint));
	}

	TexturePtr OGLRenderFactory::MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				ElementFormat format, uint32_t access_hint)
	{
		return TexturePtr(new OGLTextureCube(size, numMipMaps, format, access_hint));
	}

	FrameBufferPtr OGLRenderFactory::MakeFrameBuffer()
	{
		return FrameBufferPtr(new OGLFrameBuffer(true));
	}

	RenderLayoutPtr OGLRenderFactory::MakeRenderLayout()
	{
		return RenderLayoutPtr(new OGLRenderLayout);
	}

	GraphicsBufferPtr OGLRenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint)
	{
		return GraphicsBufferPtr(new OGLGraphicsBuffer(usage, access_hint, GL_ARRAY_BUFFER));
	}

	GraphicsBufferPtr OGLRenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint)
	{
		return GraphicsBufferPtr(new OGLGraphicsBuffer(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER));
	}

	QueryPtr OGLRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr(new OGLOcclusionQuery);
	}

	RenderViewPtr OGLRenderFactory::Make1DRenderView(Texture& texture, int level)
	{
		return RenderViewPtr(new OGLTexture1DRenderView(texture, level));
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, int level)
	{
		return RenderViewPtr(new OGLTexture2DRenderView(texture, level));
	}

	RenderViewPtr OGLRenderFactory::Make2DRenderView(Texture& texture, Texture::CubeFaces face, int level)
	{
		return RenderViewPtr(new OGLTextureCubeRenderView(texture, face, level));
	}

	RenderViewPtr OGLRenderFactory::Make3DRenderView(Texture& texture, uint32_t slice, int level)
	{
		return RenderViewPtr(new OGLTexture3DRenderView(texture, slice, level));
	}

	RenderViewPtr OGLRenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf)
	{
		return RenderViewPtr(new OGLGraphicsBufferRenderView(gbuffer, width, height, pf));
	}

	RenderViewPtr OGLRenderFactory::MakeDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t multi_sample)
	{
		return RenderViewPtr(new OGLDepthStencilRenderView(width, height, pf, multi_sample));
	}

	ShaderObjectPtr OGLRenderFactory::MakeShaderObject()
	{
		return ShaderObjectPtr(new OGLShaderObject);
	}

	RasterizerStateObjectPtr OGLRenderFactory::DoMakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		return RasterizerStateObjectPtr(new OGLRasterizerStateObject(desc));
	}
	
	DepthStencilStateObjectPtr OGLRenderFactory::DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		return DepthStencilStateObjectPtr(new OGLDepthStencilStateObject(desc));
	}

	BlendStateObjectPtr OGLRenderFactory::DoMakeBlendStateObject(BlendStateDesc const & desc)
	{
		return BlendStateObjectPtr(new OGLBlendStateObject(desc));
	}

	RenderFactory& OGLRenderFactoryInstance()
	{
		static OGLRenderFactory renderFactory;
		return renderFactory;
	}
}
