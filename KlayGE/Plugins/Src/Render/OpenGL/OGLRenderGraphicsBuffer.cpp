// OGLRenderGraphicsBuffer.hpp
// KlayGE OpenGL渲染图形缓冲区类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Util.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderGraphicsBuffer.hpp>

namespace KlayGE
{
	OGLRenderGraphicsBuffer::OGLRenderGraphicsBuffer(uint32_t width, uint32_t height)
	{
		left_ = 0;
		top_ = 0;
		width_ = width;
		height_ = height;

		viewport_.left		= left_;
		viewport_.top		= top_;
		viewport_.width		= width_;
		viewport_.height	= height_;

		if (glloader_GL_EXT_framebuffer_object())
		{
			if (!glloader_GL_ARB_pixel_buffer_object())
			{
				THR(E_FAIL);
			}

			glIsRenderbufferEXT_ = glIsRenderbufferEXT;
			glBindRenderbufferEXT_ = glBindRenderbufferEXT;
			glDeleteRenderbuffersEXT_ = glDeleteRenderbuffersEXT;
			glGenRenderbuffersEXT_ = glGenRenderbuffersEXT;
			glRenderbufferStorageEXT_ = glRenderbufferStorageEXT;
			glGetRenderbufferParameterivEXT_ = glGetRenderbufferParameterivEXT;
			glIsFramebufferEXT_ = glIsFramebufferEXT;
			glBindFramebufferEXT_ = glBindFramebufferEXT;
			glDeleteFramebuffersEXT_ = glDeleteFramebuffersEXT;
			glGenFramebuffersEXT_ = glGenFramebuffersEXT;
			glCheckFramebufferStatusEXT_ = glCheckFramebufferStatusEXT;
			glFramebufferTexture1DEXT_ = glFramebufferTexture1DEXT;
			glFramebufferTexture2DEXT_ = glFramebufferTexture2DEXT;
			glFramebufferTexture3DEXT_ = glFramebufferTexture3DEXT;
			glFramebufferRenderbufferEXT_ = glFramebufferRenderbufferEXT;
			glGetFramebufferAttachmentParameterivEXT_ = glGetFramebufferAttachmentParameterivEXT;
			glGenerateMipmapEXT_ = glGenerateMipmapEXT;

			glGenTextures(1, &texture_);
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture_);
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_FLOAT_RGBA_NV, width, height,
				0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glGenFramebuffersEXT_(1, &fbo_);
			glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, fbo_);
			glFramebufferTexture2DEXT_(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, texture_, 0);

			glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, 0);
		}
		else
		{
			THR(E_FAIL);
		}
	}

	void OGLRenderGraphicsBuffer::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderGraphicsBuffer::Attach(GraphicsBufferPtr vs)
	{
		BOOST_ASSERT(glIsFramebufferEXT_(fbo_));

		vs_ = vs;

		OGLGraphicsBuffer& ogl_vs = *checked_cast<OGLGraphicsBuffer*>(vs_.get());
		glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ogl_vs.OGLvbo());
		glBufferData(GL_PIXEL_PACK_BUFFER_ARB,
			reinterpret_cast<GLsizeiptr>(width_ * height_ * 4 * sizeof(float)), NULL, GL_DYNAMIC_DRAW);

		glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, fbo_);
	}

	void OGLRenderGraphicsBuffer::Detach()
	{
		BOOST_ASSERT(glIsFramebufferEXT_(fbo_));

		vs_->Resize(width_ * height_ * 4 * sizeof(GL_FLOAT));

		OGLGraphicsBuffer& ogl_vs = static_cast<OGLGraphicsBuffer&>(*vs_);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ogl_vs.OGLvbo());
		glReadPixels(0, 0, width_, height_, GL_RGBA, GL_FLOAT, 0);

		glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, 0);
	}
}
