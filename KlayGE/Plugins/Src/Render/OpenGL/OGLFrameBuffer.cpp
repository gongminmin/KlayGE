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
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Color.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>

namespace KlayGE
{
	OGLFrameBuffer::OGLFrameBuffer(bool off_screen)
	{
		left_ = 0;
		top_ = 0;

		if (off_screen)
		{
			glGenFramebuffersEXT(1, &fbo_);
		}
		else
		{
			fbo_ = 0;
		}
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		if (fbo_ != 0)
		{
			glDeleteFramebuffersEXT(1, &fbo_);
		}
	}

	std::wstring const & OGLFrameBuffer::Description() const
	{
		static std::wstring const desc(L"OpenGL Frame Buffer Object");
		return desc;
	}

	void OGLFrameBuffer::OnBind()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			THR(boost::system::posix_error::not_supported);
		}

		if (fbo_ != 0)
		{
			std::vector<GLenum> targets(clr_views_.size());
			for (size_t i = 0; i < clr_views_.size(); ++ i)
			{
				targets[i] = GL_COLOR_ATTACHMENT0_EXT + i;
			}
			glDrawBuffers(static_cast<GLsizei>(targets.size()), &targets[0]);
		}
		else
		{
			GLenum targets[] = { GL_BACK_LEFT };
			glDrawBuffers(1, &targets[0]);
		}
	}

	void OGLFrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(fbo_);

		if (glloader_GL_VERSION_3_0())
		{
			if (flags & CBM_Color)
			{
				glClearBufferfv(GL_COLOR_BUFFER_BIT, &clr[0]);
			}
			GLbitfield ogl_flags = 0;
			if (flags & CBM_Depth)
			{
				ogl_flags |= GL_DEPTH_BUFFER_BIT;
			}
			if (flags & CBM_Stencil)
			{
				ogl_flags |= GL_STENCIL_BUFFER_BIT;
			}
			glClearBufferfi(ogl_flags, depth, stencil);
		}
		else
		{
			glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			GLbitfield ogl_flags = 0;
			if (flags & CBM_Color)
			{
				ogl_flags |= GL_COLOR_BUFFER_BIT;
				re.ClearColor(clr.r(), clr.g(), clr.b(), clr.a());
			}
			if (flags & CBM_Depth)
			{
				ogl_flags |= GL_DEPTH_BUFFER_BIT;
				re.ClearDepth(depth);

				glDepthMask(GL_TRUE);
			}
			if (flags & CBM_Stencil)
			{
				ogl_flags |= GL_STENCIL_BUFFER_BIT;
				re.ClearStencil(stencil);

				glStencilMaskSeparate(GL_FRONT, GL_TRUE);
				glStencilMaskSeparate(GL_BACK, GL_TRUE);
			}

			glClear(ogl_flags);

			glPopAttrib();
		}

		re.BindFramebuffer(old_fbo);
	}
}
