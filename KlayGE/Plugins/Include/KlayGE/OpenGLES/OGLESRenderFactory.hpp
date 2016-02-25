// OGLESRenderFactory.h
// KlayGE OpenGL ES��Ⱦ������ ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERFACTORY_HPP
#define _OGLESRENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_OGLES_RE_SOURCE				// Build dll
	#define KLAYGE_OGLES_RE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_OGLES_RE_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_OGLES_RE_API void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr);
}

#endif			// _OGLESRENDERFACTORY_HPP
