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
				targets[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0_EXT + i);
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

		DepthStencilStateDesc const & cur_desc = re.CurDSSObj()->GetDesc();

		if (flags & CBM_Depth)
		{
			if (!cur_desc.depth_write_mask)
			{
				glDepthMask(GL_TRUE);
			}
		}
		if (flags & CBM_Stencil)
		{
			if (!cur_desc.front_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_FRONT, GL_TRUE);
			}
			if (!cur_desc.back_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_BACK, GL_TRUE);
			}
		}

		if (glloader_GL_VERSION_3_0())
		{
			if (flags & CBM_Color)
			{
				if (fbo_ != 0)
				{
					for (size_t i = 0; i < clr_views_.size(); ++ i)
					{
						if (clr_views_[i])
						{
							glClearBufferfv(GL_COLOR, static_cast<GLint>(i), &clr[0]);
						}
					}
				}
				else
				{
					glClearBufferfv(GL_COLOR, 0, &clr[0]);
				}
			}

			if ((flags & CBM_Depth) && (flags & CBM_Stencil))
			{
				glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
			}
			else
			{
				if (flags & CBM_Depth)
				{
					glClearBufferfv(GL_DEPTH, 0, &depth);
				}
				else
				{
					if (flags & CBM_Stencil)
					{
						GLint s = stencil;
						glClearBufferiv(GL_STENCIL, 0, &s);
					}
				}
			}

			glGetError();
		}
		else
		{
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
			}
			if (flags & CBM_Stencil)
			{
				ogl_flags |= GL_STENCIL_BUFFER_BIT;
				re.ClearStencil(stencil);
			}

			glClear(ogl_flags);
		}

		if (flags & CBM_Depth)
		{
			if (!cur_desc.depth_write_mask)
			{
				glDepthMask(GL_FALSE);
			}
		}
		if (flags & CBM_Stencil)
		{
			if (!cur_desc.front_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_FRONT, GL_FALSE);
			}
			if (!cur_desc.back_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_BACK, GL_FALSE);
			}
		}

		re.BindFramebuffer(old_fbo);
	}
}
