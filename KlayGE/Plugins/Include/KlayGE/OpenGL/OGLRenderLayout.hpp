// OGLRenderLayout.cpp
// KlayGE OpenGL渲染分布类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2005.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERLAYOUT_HPP
#define _OGLRENDERLAYOUT_HPP

#include <KlayGE/RenderLayout.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderLayout : public RenderLayout
	{
	public:
		explicit OGLRenderLayout(buffer_type type)
			: RenderLayout(type)
		{
		}

		~OGLRenderLayout()
		{
		}
	};
}

#endif			// _OGLRENDERLAYOUT_HPP
