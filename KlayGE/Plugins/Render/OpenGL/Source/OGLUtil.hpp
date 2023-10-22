// OGLMapping.hpp
// KlayGE RenderEngine和OpenGL本地之间的映射 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
//
// 2.8.0
// 初次建立 (2005.7.19)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLMAPPING_HPP
#define _OGLMAPPING_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <glloader/glloader.h>

namespace KlayGE
{
	class OGLMapping final
	{
	public:
		static void Mapping(GLfloat* clr4, Color const & clr);

		static GLenum Mapping(CompareFunction func);
		static GLenum Mapping(StencilOperation op);
		static GLenum Mapping(AlphaBlendFactor factor);
		static GLenum Mapping(PolygonMode mode);
		static GLenum Mapping(ShadeMode mode);
		static GLenum Mapping(BlendOperation bo);
		static GLint Mapping(TexAddressingMode mode);
		static GLenum Mapping(LogicOperation lo);

		static void Mapping(GLenum& primType, uint32_t& primCount, RenderLayout const & rl);

		static void MappingFormat(GLint& internalFormat, GLenum& glformat, GLenum& gltype, ElementFormat ef);
		static void MappingVertexFormat(GLenum& gltype, GLboolean& normalized, ElementFormat ef);
	};
}

#endif			// _OGLMAPPING_HPP
