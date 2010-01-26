// OGLES2Mapping.hpp
// KlayGE RenderEngine和OpenGL ES2 本地之间的映射 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2MAPPING_HPP
#define _OGLES2MAPPING_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <glloader/glloader.h>

namespace KlayGE
{
	class OGLES2Mapping
	{
	public:
		static void Mapping(GLfloat* clr4, Color const & clr);

		static GLenum Mapping(CompareFunction func);

		static GLenum Mapping(StencilOperation op);

		static GLenum Mapping(AlphaBlendFactor factor);
		static GLenum Mapping(BlendOperation bo);
		static GLint Mapping(TexAddressingMode mode);

		static void Mapping(GLenum& primType, uint32_t& primCount, RenderLayout const & rl);

		static void MappingFormat(GLint& internalFormat, GLenum& glformat, GLenum& gltype, ElementFormat ef);
	};
}

#endif			// _OGLES2MAPPING_HPP
