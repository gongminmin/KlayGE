// OGLFrameBuffer.hpp
// KlayGE OpenGL渲染到纹理类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 2.8.0
// 初次建立 (2005.8.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLFRAMEBUFFER_HPP
#define _OGLFRAMEBUFFER_HPP

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderView.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLFrameBuffer : public FrameBuffer
	{
	public:
		OGLFrameBuffer();
		~OGLFrameBuffer();

		void OnBind();

		GLuint OGLFbo() const
		{
			return fbo_;
		}

	private:
		GLuint fbo_;
	};

	typedef boost::shared_ptr<OGLFrameBuffer> OGLFrameBufferPtr;
}

#endif			// _OGLFRAMEBUFFER_HPP
