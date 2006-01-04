// OGLRenderFactory.h
// KlayGE OpenGL渲染工厂类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERFACTORY_HPP
#define _OGLRENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderEffect.hpp>
#include <KlayGE/OpenGL/OGLVertexBuffer.hpp>
#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLIndexStream.hpp>
#include <KlayGE/OpenGL/OGLRenderVertexStream.hpp>
#include <KlayGE/OpenGL/OGLOcclusionQuery.hpp>

#define NOMINMAX
#include <windows.h>
#include <glloader/glloader.h>
#include <gl/glu.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	RenderFactory& OGLRenderFactoryInstance();

	typedef ConcreteRenderFactory<OGLRenderEngine, OGLTexture, OGLRenderTexture,
				OGLRenderEffect, OGLRenderVertexStream> OGLRenderFactoryBase;

	class OGLRenderFactory : public OGLRenderFactoryBase
	{
	public:
		OGLRenderFactory();

		CGcontext CGContext() const;

		TexturePtr MakeTexture1D(uint32_t width, uint16_t numMipMaps,
				PixelFormat format);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps,
				PixelFormat format);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth,
				uint16_t numMipMaps, PixelFormat format);
		TexturePtr MakeTextureCube(uint32_t size, uint16_t numMipMaps,
				PixelFormat format);

		RenderTexturePtr MakeRenderTexture();

		RenderEffectPtr DoMakeRenderEffect(std::string const & srcData);

		VertexBufferPtr MakeVertexBuffer(VertexBuffer::BufferType type);
		VertexStreamPtr MakeVertexStream(vertex_elements_type const & vertex_elems, bool staticStream);
		IndexStreamPtr MakeIndexStream(IndexFormat format, bool staticStream);

		RenderVertexStreamPtr MakeRenderVertexStream(uint32_t width, uint32_t height);

		QueryPtr MakeOcclusionQuery();

	private:
		CGcontext context_;

	private:
		OGLRenderFactory(OGLRenderFactory const &);
		OGLRenderFactory& operator=(OGLRenderFactory const &);
	};
}

#endif			// _OGLRENDERFACTORY_HPP
