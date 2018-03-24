// OGLFrameBuffer.cpp
// KlayGE OpenGL渲染到纹理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://www.klayge.org
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
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KFL/Util.hpp>
#include <KFL/Color.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>

namespace KlayGE
{
	OGLFrameBuffer::OGLFrameBuffer(bool off_screen)
	{
		if (off_screen)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glCreateFramebuffers(1, &fbo_);
			}
			else
			{
				glGenFramebuffers(1, &fbo_);
			}
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
			if (Context::Instance().RenderFactoryValid())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteFramebuffers(1, &fbo_);
			}
			else
			{
				glDeleteFramebuffers(1, &fbo_);
			}
		}
	}

	std::wstring const & OGLFrameBuffer::Description() const
	{
		static std::wstring const desc(L"OpenGL Frame Buffer Object");
		return desc;
	}

	void OGLFrameBuffer::OnBind()
	{
		if (views_dirty_)
		{
			if (fbo_ != 0)
			{
				gl_targets_.resize(clr_views_.size());
				for (size_t i = 0; i < clr_views_.size(); ++ i)
				{
					gl_targets_[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);
				}
			}
			else
			{
				gl_targets_.resize(1);
				gl_targets_[0] = GL_BACK_LEFT;
			}

			views_dirty_ = false;
		}

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			if (fbo_ != 0)
			{
				BOOST_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER));
			}
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			if (fbo_ != 0)
			{
				BOOST_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckNamedFramebufferStatusEXT(fbo_, GL_FRAMEBUFFER));
			}
		}
		else
		{
			BOOST_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		if (fbo_ != 0)
		{
			re.EnableFramebufferSRGB(IsSRGB(clr_views_[0]->Format()));
		}
		else
		{
			re.EnableFramebufferSRGB(false);
		}

		glDrawBuffers(static_cast<GLsizei>(gl_targets_.size()), &gl_targets_[0]);
	}

	void OGLFrameBuffer::OnUnbind()
	{
	}

	void OGLFrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(fbo_);

		DepthStencilStateDesc const & ds_desc = re.CurRenderStateObject()->GetDepthStencilStateDesc();
		BlendStateDesc const & blend_desc = re.CurRenderStateObject()->GetBlendStateDesc();

		if (flags & CBM_Color)
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (blend_desc.color_write_mask[i] != CMASK_All)
				{
					glColorMaski(i, true, true, true, true);
				}
			}
		}
		if (flags & CBM_Depth)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_TRUE);
			}
		}
		if (flags & CBM_Stencil)
		{
			if (ds_desc.front_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_FRONT, 0xFF);
			}
			if (ds_desc.back_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_BACK, 0xFF);
			}
		}

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

		if (flags & CBM_Color)
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (blend_desc.color_write_mask[i] != CMASK_All)
				{
					glColorMaski(i, (blend_desc.color_write_mask[i] & CMASK_Red) != 0,
						(blend_desc.color_write_mask[i] & CMASK_Green) != 0,
						(blend_desc.color_write_mask[i] & CMASK_Blue) != 0,
						(blend_desc.color_write_mask[i] & CMASK_Alpha) != 0);
				}
			}
		}
		if (flags & CBM_Depth)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_FALSE);
			}
		}
		if (flags & CBM_Stencil)
		{
			if (ds_desc.front_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_FRONT, ds_desc.front_stencil_write_mask);
			}
			if (ds_desc.back_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_BACK, ds_desc.back_stencil_write_mask);
			}
		}

		re.BindFramebuffer(old_fbo);
	}

	void OGLFrameBuffer::Discard(uint32_t flags)
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
		{
			std::vector<GLenum> attachments;
			if (fbo_ != 0)
			{
				if (flags & CBM_Color)
				{
					for (size_t i = 0; i < clr_views_.size(); ++ i)
					{
						if (clr_views_[i])
						{
							attachments.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
						}
					}
				}
				if (flags & CBM_Depth)
				{
					if (ds_view_)
					{
						attachments.push_back(GL_DEPTH_ATTACHMENT);
					}
				}
				if (flags & CBM_Stencil)
				{
					if (ds_view_)
					{
						attachments.push_back(GL_STENCIL_ATTACHMENT);
					}
				}
			}
			else
			{
				if (flags & CBM_Color)
				{
					attachments.push_back(GL_COLOR);
				}
				if (flags & CBM_Depth)
				{
					attachments.push_back(GL_DEPTH);
				}
				if (flags & CBM_Stencil)
				{
					attachments.push_back(GL_STENCIL);
				}
			}

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glInvalidateNamedFramebufferData(fbo_, static_cast<GLsizei>(attachments.size()), &attachments[0]);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint old_fbo = re.BindFramebuffer();
				re.BindFramebuffer(fbo_);

				glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(attachments.size()), &attachments[0]);

				re.BindFramebuffer(old_fbo);
			}
		}
		else
		{
			this->Clear(flags, Color(0, 0, 0, 0), 1, 0);
		}
	}
}
