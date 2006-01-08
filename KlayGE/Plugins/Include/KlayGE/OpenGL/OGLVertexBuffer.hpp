// OGLVertexBuffer.cpp
// KlayGE OpenGL顶点缓冲区类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLVERTEXBUFFER_HPP
#define _OGLVERTEXBUFFER_HPP

#include <KlayGE/VertexBuffer.hpp>

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

#endif			// _OGLVERTEXBUFFER_HPP
