// OGLRenderFactory.h
// KlayGE OpenGL��Ⱦ������ ͷ�ļ�
// Ver 2.7.0
// ��Ȩ����(C) ������, 2004-2005
// Homepage: http://www.klayge.org
//
// 2.7.0
// ֧��vertex_buffer_object (2005.6.19)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERFACTORY_HPP
#define _OGLRENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_OGL_RE_SOURCE					// Build dll
	#define KLAYGE_OGL_RE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_OGL_RE_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_OGL_RE_API void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr);
}

#endif			// _OGLRENDERFACTORY_HPP
