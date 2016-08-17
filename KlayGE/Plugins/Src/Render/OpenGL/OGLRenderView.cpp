// OGLRenderView.cpp
// KlayGE OGL渲染视图类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持Texture Array (2009.8.5)
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>

namespace KlayGE
{
	OGLRenderView::OGLRenderView()
		: tex_(0), fbo_(0)
	{
	}

	OGLRenderView::~OGLRenderView()
	{
	}

	void OGLRenderView::ClearDepth(float depth)
	{
		this->DoClear(GL_DEPTH_BUFFER_BIT, Color(), depth, 0);
	}

	void OGLRenderView::ClearStencil(int32_t stencil)
	{
		this->DoClear(GL_STENCIL_BUFFER_BIT, Color(), 0, stencil);
	}

	void OGLRenderView::ClearDepthStencil(float depth, int32_t stencil)
	{
		uint32_t flags = 0;
		if (IsDepthFormat(pf_))
		{
			flags |= GL_DEPTH_BUFFER_BIT;
		}
		if (IsStencilFormat(pf_))
		{
			flags |= GL_STENCIL_BUFFER_BIT;
		}

		this->DoClear(flags, Color(), depth, stencil);
	}

	void OGLRenderView::DoClear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(fbo_);

		DepthStencilStateDesc const & ds_desc = re.CurDSSObj()->GetDesc();
		BlendStateDesc const & blend_desc = re.CurBSObj()->GetDesc();

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			if (glloader_GL_VERSION_3_0())
			{
				for (int i = 0; i < 8; ++ i)
				{
					if (blend_desc.color_write_mask[i] != CMASK_All)
					{
						glColorMaski(i, true, true, true, true);
					}
				}
			}
			else if (glloader_GL_EXT_draw_buffers2())
			{
				for (int i = 0; i < 8; ++ i)
				{
					if (blend_desc.color_write_mask[i] != CMASK_All)
					{
						glColorMaskIndexedEXT(i, true, true, true, true);
					}
				}
			}
			else
			{
				if (blend_desc.color_write_mask[0] != CMASK_All)
				{
					glColorMask(true, true, true, true);
				}
			}
		}
		if (flags & GL_DEPTH_BUFFER_BIT)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_TRUE);
			}
		}
		if (flags & GL_STENCIL_BUFFER_BIT)
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

		if (glloader_GL_VERSION_3_0())
		{
			if (flags & GL_COLOR_BUFFER_BIT)
			{
				glClearBufferfv(GL_COLOR, index_, &clr[0]);
			}

			if ((flags & GL_DEPTH_BUFFER_BIT) && (flags & GL_STENCIL_BUFFER_BIT))
			{
				glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
			}
			else
			{
				if (flags & GL_DEPTH_BUFFER_BIT)
				{
					glClearBufferfv(GL_DEPTH, 0, &depth);
				}
				else
				{
					if (flags & GL_STENCIL_BUFFER_BIT)
					{
						GLint s = stencil;
						glClearBufferiv(GL_STENCIL, 0, &s);
					}
				}
			}
		}
		else
		{
			if (flags & GL_COLOR_BUFFER_BIT)
			{
				re.ClearColor(clr.r(), clr.g(), clr.b(), clr.a());
			}
			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				re.ClearDepth(depth);
			}
			if (flags & GL_STENCIL_BUFFER_BIT)
			{
				re.ClearStencil(stencil);
			}
			if (flags != 0)
			{
				glClear(flags);
			}
		}

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			if (glloader_GL_VERSION_3_0())
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
			else if (glloader_GL_EXT_draw_buffers2())
			{
				for (int i = 0; i < 8; ++ i)
				{
					if (blend_desc.color_write_mask[i] != CMASK_All)
					{
						glColorMaskIndexedEXT(i, (blend_desc.color_write_mask[i] & CMASK_Red) != 0,
							(blend_desc.color_write_mask[i] & CMASK_Green) != 0,
							(blend_desc.color_write_mask[i] & CMASK_Blue) != 0,
							(blend_desc.color_write_mask[i] & CMASK_Alpha) != 0);
					}
				}
			}
			else
			{
				if (blend_desc.color_write_mask[0] != CMASK_All)
				{
					glColorMask((blend_desc.color_write_mask[0] & CMASK_Red) != 0,
							(blend_desc.color_write_mask[0] & CMASK_Green) != 0,
							(blend_desc.color_write_mask[0] & CMASK_Blue) != 0,
							(blend_desc.color_write_mask[0] & CMASK_Alpha) != 0);
				}
			}
		}
		if (flags & GL_DEPTH_BUFFER_BIT)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_FALSE);
			}
		}
		if (flags & GL_STENCIL_BUFFER_BIT)
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

	void OGLRenderView::DoDiscardColor()
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
		{
			GLenum attachment;
			if (fbo_ != 0)
			{
				attachment = GL_COLOR_ATTACHMENT0 + index_;
			}
			else
			{
				attachment = GL_COLOR;
			}

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			GLuint old_fbo = re.BindFramebuffer();
			re.BindFramebuffer(fbo_);

			glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

			re.BindFramebuffer(old_fbo);
		}
		else
		{
			this->ClearColor(Color(0, 0, 0, 0));
		}
	}

	void OGLRenderView::DoDiscardDepthStencil()
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
		{
			GLenum attachments[2];
			if (fbo_ != 0)
			{
				attachments[0] = GL_DEPTH_ATTACHMENT;
				attachments[1] = GL_STENCIL_ATTACHMENT;
			}
			else
			{
				attachments[0] = GL_DEPTH;
				attachments[1] = GL_STENCIL;
			}

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			GLuint old_fbo = re.BindFramebuffer();
			re.BindFramebuffer(fbo_);

			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

			re.BindFramebuffer(old_fbo);
		}
		else
		{
			this->ClearDepthStencil(1, 0);
		}
	}


	OGLScreenColorRenderView::OGLScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void OGLScreenColorRenderView::ClearColor(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLScreenColorRenderView::ClearDepth(float /*depth*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenColorRenderView::ClearStencil(int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenColorRenderView::ClearDepthStencil(float /*depth*/, int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenColorRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLScreenColorRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		index_ = att - FrameBuffer::ATT_Color0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenColorRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLScreenDepthStencilRenderView::OGLScreenDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void OGLScreenDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLScreenDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		index_ = 0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLTexture1DRenderView::OGLTexture1DRenderView(Texture& texture_1d, int array_index, int array_size, int level)
		: texture_1d_(*checked_cast<OGLTexture1D*>(&texture_1d)),
			array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d.Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_1d_.ArraySize())));

		if ((array_index > 0) && (!(glloader_GL_VERSION_3_0() || glloader_GL_EXT_texture_array())))
		{
			THR(errc::function_not_supported);
		}

		uint32_t const channels = NumComponents(texture_1d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_1d_.GLTexture();

		width_ = texture_1d_.Width(level);
		height_ = 1;
		pf_ = texture_1d_.Format();
	}

	void OGLTexture1DRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, texture_1d_.GLType(), tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage1D(texture_1d_.GLType(), level_, 0, width_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture1DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture1DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_1D == texture_1d_.GLType())
		{
			if (texture_1d_.SampleCount() <= 1)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture1DEXT(fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_1d_.GLType(), tex_, level_);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferTexture1D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_1d_.GLType(), tex_, level_);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, tex_);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, tex_);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						tex_, level_, array_index_);
				}
			}
			else
			{
				re.BindFramebuffer(fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, tex_, level_);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						tex_, level_, array_index_);
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_1D == texture_1d_.GLType())
		{
			if (texture_1d_.SampleCount() <= 1)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture1DEXT(fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_1d_.GLType(), 0, 0);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferTexture1D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_1d_.GLType(), 0, 0);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}
			}
			else
			{
				re.BindFramebuffer(fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLTexture2DRenderView::OGLTexture2DRenderView(Texture& texture_2d, int array_index, int array_size, int level)
		: texture_2d_(*checked_cast<OGLTexture2D*>(&texture_2d)),
			array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d.Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_2d_.ArraySize())));

		if ((array_index > 0) && (!(glloader_GL_VERSION_3_0() || glloader_GL_EXT_texture_array())))
		{
			THR(errc::function_not_supported);
		}

		uint32_t const channels = NumComponents(texture_2d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_2d_.GLTexture();

		width_ = texture_2d_.Width(level);
		height_ = texture_2d_.Height(level);
		pf_ = texture_2d_.Format();
	}

	void OGLTexture2DRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, texture_2d_.GLType(), tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(texture_2d_.GLType(), level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture2DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture2DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_2D == texture_2d_.GLType())
		{
			if (texture_2d_.SampleCount() <= 1)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_2d_.GLType(), tex_, level_);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_2d_.GLType(), tex_, level_);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, tex_);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, tex_);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						tex_, level_, array_index_);
				}
			}
			else
			{
				re.BindFramebuffer(fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, tex_, level_);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						tex_, level_, array_index_);
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_2D == texture_2d_.GLType())
		{
			if (texture_2d_.SampleCount() <= 1)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture2DEXT(fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_2d_.GLType(), 0, 0);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_2d_.GLType(), 0, 0);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);
				}
				else
				{
					re.BindFramebuffer(fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}
			}
			else
			{
				re.BindFramebuffer(fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLTexture3DRenderView::OGLTexture3DRenderView(Texture& texture_3d, int array_index, uint32_t slice, int level)
		: texture_3d_(*checked_cast<OGLTexture3D*>(&texture_3d)),
			slice_(slice), level_(level), copy_to_tex_(0)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_3D == texture_3d.Type());
		BOOST_ASSERT(texture_3d_.Depth(level) > slice);
		BOOST_ASSERT(0 == array_index);

		uint32_t const channels = NumComponents(texture_3d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_3d_.GLTexture();

		width_ = texture_3d_.Width(level);
		height_ = texture_3d_.Height(level);
		pf_ = texture_3d_.Format();
	}

	OGLTexture3DRenderView::~OGLTexture3DRenderView()
	{
		if (2 == copy_to_tex_)
		{
			glDeleteTextures(1, &tex_2d_);
		}
	}

	void OGLTexture3DRenderView::ClearColor(Color const & clr)
	{
		BOOST_ASSERT(fbo_ != 0);

		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLTexture3DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			if (0 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(fbo_,
					GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tex_, level_, slice_);

				GLenum status = glCheckNamedFramebufferStatusEXT(fbo_, GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE)
				{
					glGenTextures(1, &tex_2d_);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTextureImage2DEXT(tex_2d_, GL_TEXTURE_RECTANGLE_ARB,
							   0, GL_RGBA32F_ARB,
							   width_, height_, 0,
							   GL_RGBA, GL_FLOAT, nullptr);
					glTextureParameteriEXT(tex_2d_, GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTextureParameteriEXT(tex_2d_, GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTextureParameteriEXT(tex_2d_, GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTextureParameteriEXT(tex_2d_, GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

					copy_to_tex_ = 2;
				}
				else
				{
					copy_to_tex_ = 1;
				}
			}

			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
			}
		}
		else
		{
			re.BindFramebuffer(fbo_);

			if (0 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tex_, level_, slice_);

				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE)
				{
					glGenTextures(1, &tex_2d_);
					glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_2d_);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
						0, GL_RGBA, GL_FLOAT, nullptr);
					glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

					copy_to_tex_ = 2;
				}
				else
				{
					copy_to_tex_ = 1;
				}
			}

			if (1 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		if (glloader_GL_EXT_direct_state_access())
		{
			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, 0, 0);

				this->CopyToSlice(att);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, 0, 0);

				this->CopyToSlice(att);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t att)
	{
		BOOST_ASSERT(copy_to_tex_ != 0);
		if (2 == copy_to_tex_)
		{
			this->CopyToSlice(att);
		}
	}

	void OGLTexture3DRenderView::CopyToSlice(uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, GL_TEXTURE_3D, tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLTextureCubeRenderView::OGLTextureCubeRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<OGLTextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(0 == array_index);

		uint32_t const channels = NumComponents(texture_cube.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_cube_.GLTexture();

		width_ = texture_cube_.Width(level);
		height_ = texture_cube_.Height(level);
		pf_ = texture_cube_.Format();
	}

	OGLTextureCubeRenderView::OGLTextureCubeRenderView(Texture& texture_cube, int array_index, int level)
		: texture_cube_(*checked_cast<OGLTextureCube*>(&texture_cube)),
			face_(static_cast<Texture::CubeFaces>(-1)),
			level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(0 == array_index);
		
		uint32_t const channels = NumComponents(texture_cube.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_cube_.GLTexture();

		width_ = texture_cube_.Width(level);
		height_ = texture_cube_.Height(level);
		pf_ = texture_cube_.Format();
	}

	void OGLTextureCubeRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_CUBE_MAP, tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTextureCubeRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTextureCubeRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (face_ >= 0)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						face, tex_, level_);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						face, tex_, level_);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureEXT(fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					tex_, level_);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferTexture(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					tex_, level_);

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		if (face_ >= 0)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						face, 0, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						face, 0, 0);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureEXT(fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferTexture(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0);

				re.BindFramebuffer(0);
			}
		}
	}


	OGLGraphicsBufferRenderView::OGLGraphicsBufferRenderView(GraphicsBuffer& gb,
									uint32_t width, uint32_t height, ElementFormat pf)
		: gbuffer_(gb)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;

		glGenTextures(1, &tex_);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_);
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
			0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	OGLGraphicsBufferRenderView::~OGLGraphicsBufferRenderView()
	{
		glDeleteTextures(1, &tex_);
	}

	void OGLGraphicsBufferRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_RECTANGLE_ARB, tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLGraphicsBufferRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLGraphicsBufferRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, tex_, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, tex_, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		if (glloader_GL_EXT_direct_state_access())
		{
			this->CopyToGB(att);

			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			this->CopyToGB(att);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderView::OnUnbind(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		this->CopyToGB(att);

		re.BindFramebuffer(0);
	}

	void OGLGraphicsBufferRenderView::CopyToGB(uint32_t att)
	{
		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0);

		OGLGraphicsBuffer* ogl_gb = checked_cast<OGLGraphicsBuffer*>(&gbuffer_);
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(GL_PIXEL_PACK_BUFFER, ogl_gb->GLvbo());
		glReadPixels(0, 0, width_, height_, glformat, gltype, nullptr);
	}


	OGLDepthStencilRenderView::OGLDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: target_type_(0), array_index_(0), level_(-1),
			sample_count_(sample_count), sample_quality_(sample_quality)
	{
		BOOST_ASSERT(IsDepthFormat(pf));
		KFL_UNUSED(sample_count_);
		KFL_UNUSED(sample_quality_);

		width_ = width;
		height_ = height;
		pf_ = pf;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		glGenRenderbuffers(1, &rbo_);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
		if (sample_count <= 1)
		{
			glRenderbufferStorage(GL_RENDERBUFFER,
								internalFormat, width_, height_);
		}
		else
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, sample_count,
								internalFormat, width_, height_);
		}
	}

	OGLDepthStencilRenderView::OGLDepthStencilRenderView(Texture& texture, int array_index, int array_size, int level)
		: target_type_(checked_cast<OGLTexture*>(&texture)->GLType()),
			array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT((Texture::TT_2D == texture.Type()) || (Texture::TT_Cube == texture.Type()));
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture.ArraySize())));
		BOOST_ASSERT(IsDepthFormat(texture.Format()));

		if ((array_index > 0) && (!(glloader_GL_VERSION_3_0() || glloader_GL_EXT_texture_array())))
		{
			THR(errc::function_not_supported);
		}

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();

		tex_ = checked_cast<OGLTexture*>(&texture)->GLTexture();
	}

	OGLDepthStencilRenderView::~OGLDepthStencilRenderView()
	{
		glDeleteRenderbuffers(1, &rbo_);
	}

	void OGLDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (level_ < 0)
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, rbo_);
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferRenderbufferEXT(fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, rbo_);
				}
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, rbo_);
				if (IsStencilFormat(pf_))
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, rbo_);
				}

				re.BindFramebuffer(0);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(fbo_,
							GL_DEPTH_ATTACHMENT, target_type_, tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(fbo_,
							GL_STENCIL_ATTACHMENT, target_type_, tex_, level_);
					}
				}
				else
				{
					re.BindFramebuffer(fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_DEPTH_ATTACHMENT, target_type_, tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_STENCIL_ATTACHMENT, target_type_, tex_, level_);
					}

					re.BindFramebuffer(0);
				}
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTextureEXT(fbo_, GL_DEPTH_ATTACHMENT, tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTextureEXT(fbo_, GL_STENCIL_ATTACHMENT, tex_, level_);
					}
				}
				else
				{
					re.BindFramebuffer(fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, tex_, level_);
					}

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (array_size_ > 1)
				{
					if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureEXT(fbo_, GL_DEPTH_ATTACHMENT, tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureEXT(fbo_, GL_STENCIL_ATTACHMENT, tex_, level_);
						}
					}
					else
					{
						re.BindFramebuffer(fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, tex_, level_);
						}

						re.BindFramebuffer(0);
					}
				}
				else
				{
					if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(fbo_, GL_DEPTH_ATTACHMENT, tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(fbo_, GL_STENCIL_ATTACHMENT, tex_, level_, array_index_);
						}
					}
					else
					{
						re.BindFramebuffer(fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, tex_, level_, array_index_);
						}

						re.BindFramebuffer(0);
					}
				}
			}
		}
	}

	void OGLDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		if (level_ < 0)
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glNamedFramebufferRenderbufferEXT(fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(fbo_,
							GL_DEPTH_ATTACHMENT, target_type_, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(fbo_,
							GL_STENCIL_ATTACHMENT, target_type_, 0, 0);
					}
				}
				else
				{
					re.BindFramebuffer(fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_DEPTH_ATTACHMENT, target_type_, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_STENCIL_ATTACHMENT, target_type_, 0, 0);
					}

					re.BindFramebuffer(0);
				}
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTextureEXT(fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTextureEXT(fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else
				{
					re.BindFramebuffer(fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
					}

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (array_size_ > 1)
				{
					if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTexture(fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTexture(fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
						}
					}
					else
					{
						re.BindFramebuffer(fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}

						re.BindFramebuffer(0);
					}
				}
				else
				{
					if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayer(fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayer(fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}
					}
					else
					{
						re.BindFramebuffer(fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}

						re.BindFramebuffer(0);
					}
				}
			}
		}
	}


	OGLTextureCubeDepthStencilRenderView::OGLTextureCubeDepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<OGLTextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(IsDepthFormat(texture_cube.Format()));

		if ((array_index > 0) && (!(glloader_GL_VERSION_3_0() || glloader_GL_EXT_texture_array())))
		{
			THR(errc::function_not_supported);
		}

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Height(level);
		pf_ = texture_cube.Format();

		tex_ = checked_cast<OGLTextureCube*>(&texture_cube)->GLTexture();
	}

	void OGLTextureCubeDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTextureCubeDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLTextureCubeDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_EXT_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTexture2DEXT(fbo_,
					GL_DEPTH_ATTACHMENT, face, tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTexture2DEXT(fbo_,
					GL_STENCIL_ATTACHMENT, face, tex_, level_);
			}
		}
		else
		{
			re.BindFramebuffer(fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT, face, tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_STENCIL_ATTACHMENT, face, tex_, level_);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTextureCubeDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_EXT_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTexture2DEXT(fbo_,
					GL_DEPTH_ATTACHMENT, face, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTexture2DEXT(fbo_,
					GL_STENCIL_ATTACHMENT, face, 0, 0);
			}
		}
		else
		{
			re.BindFramebuffer(fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT, face, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_STENCIL_ATTACHMENT, face, 0, 0);
			}

			re.BindFramebuffer(0);
		}
	}
}
