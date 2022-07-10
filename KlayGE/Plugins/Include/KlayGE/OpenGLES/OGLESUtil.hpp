// OGLESMapping.hpp
// KlayGE RenderEngine��OpenGL ES����֮���ӳ�� ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESMAPPING_HPP
#define _OGLESMAPPING_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <glloader/glloader.h>

namespace KlayGE
{
	class OGLESMapping final
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
		static void MappingVertexFormat(GLenum& gltype, GLboolean& normalized, ElementFormat ef);
	};
}

#endif			// _OGLESMAPPING_HPP
