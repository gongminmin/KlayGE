// OGLFrameBuffer.cpp
// KlayGE OpenGL渲染到纹理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2005-2006
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

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Util.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>

namespace KlayGE
{
	OGLFrameBuffer::OGLFrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		glGenFramebuffersEXT(1, &fbo_);
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		glDeleteFramebuffersEXT(1, &fbo_);
	}
}
