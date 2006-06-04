// OGLMapping.hpp
// KlayGE RenderEngine和OpenGL本地之间的映射 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.19)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLMAPPING_HPP
#define _OGLMAPPING_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Sampler.hpp>

#define NOMINMAX
#include <windows.h>
#include <glloader/glloader.h>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLMapping
	{
	public:
		static void Mapping(GLfloat* clr4, Color const & clr);

		static GLenum Mapping(RenderEngine::CompareFunction func);

		static GLenum Mapping(RenderEngine::StencilOperation op);

		static GLenum Mapping(RenderEngine::AlphaBlendFactor factor);
		static GLenum Mapping(RenderEngine::PolygonMode mode);
		static GLenum Mapping(RenderEngine::ShadeMode mode);
		static GLenum Mapping(RenderEngine::BlendOperation bo);
		static GLint Mapping(Sampler::TexAddressingMode mode);

		static void Mapping(GLenum& primType, uint32_t& primCount, RenderLayout const & rl);

		static void MappingFormat(GLint& internalFormat, GLenum& glformat, GLenum& gltype, PixelFormat pf);
	};
}

#endif			// _OGLMAPPING_HPP
