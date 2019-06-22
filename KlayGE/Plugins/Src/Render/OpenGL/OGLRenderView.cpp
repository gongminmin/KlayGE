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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
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
	OGLTextureShaderResourceView::OGLTextureShaderResourceView(TexturePtr const & texture)
	{
		BOOST_ASSERT(texture->AccessHint() & EAH_GPU_Read);

		tex_ = texture;
		pf_ = texture->Format();

		first_array_index_ = 0;
		array_size_ = texture->ArraySize();
		first_level_ = 0;
		num_levels_ = texture->NumMipMaps();
		first_elem_ = 0;
		num_elems_ = 0;

		gl_target_ = GL_TEXTURE_2D;
		gl_tex_ = 0;
	}

	void OGLTextureShaderResourceView::RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const
	{
		if ((gl_tex_ == 0) && tex_ && tex_->HWResourceReady())
		{
			gl_target_ = checked_cast<OGLTexture&>(*tex_).GLType();
			gl_tex_ = checked_cast<OGLTexture&>(*tex_).GLTexture();
		}
		target = gl_target_;
		tex = gl_tex_;
	}


	OGLBufferShaderResourceView::OGLBufferShaderResourceView(GraphicsBufferPtr const & gbuffer, ElementFormat pf)
	{
		BOOST_ASSERT(gbuffer->AccessHint() & EAH_GPU_Read);

		buff_ = gbuffer;
		pf_ = pf;

		first_array_index_ = 0;
		array_size_ = 0;
		first_level_ = 0;
		num_levels_ = 0;
		first_elem_ = 0;
		num_elems_ = gbuffer->Size() / NumFormatBytes(pf_);

		gl_target_ = GL_TEXTURE_BUFFER;
		gl_tex_ = 0;
	}

	void OGLBufferShaderResourceView::RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const
	{
		if ((gl_tex_ == 0) && buff_ && buff_->HWResourceReady())
		{
			gl_tex_ = checked_cast<OGLGraphicsBuffer&>(*buff_).RetrieveGLTexture(pf_);
		}
		target = gl_target_;
		tex = gl_tex_;
	}


	GLuint OGLRenderTargetView::RetrieveGLTexture() const
	{
		if ((gl_tex_ == 0) && (tex_->HWResourceReady()))
		{
			gl_tex_ = checked_cast<OGLTexture&>(*tex_).GLTexture();
		}
		return gl_tex_;
	}

	void OGLRenderTargetView::DoClearColor(Color const & clr)
	{
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		BlendStateDesc const & blend_desc = re.CurRenderStateObject()->GetBlendStateDesc();

		for (int i = 0; i < 8; ++i)
		{
			if (blend_desc.color_write_mask[i] != CMASK_All)
			{
				glColorMaski(i, true, true, true, true);
			}
		}

		glClearBufferfv(GL_COLOR, index_, &clr[0]);

		for (int i = 0; i < 8; ++i)
		{
			if (blend_desc.color_write_mask[i] != CMASK_All)
			{
				glColorMaski(i, (blend_desc.color_write_mask[i] & CMASK_Red) != 0,
					(blend_desc.color_write_mask[i] & CMASK_Green) != 0,
					(blend_desc.color_write_mask[i] & CMASK_Blue) != 0,
					(blend_desc.color_write_mask[i] & CMASK_Alpha) != 0);
			}
		}

		re.BindFramebuffer(old_fbo);
	}

	void OGLRenderTargetView::DoDiscardColor()
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
		{
			GLenum attachment;
			if (gl_fbo_ != 0)
			{
				attachment = GL_COLOR_ATTACHMENT0 + index_;
			}
			else
			{
				attachment = GL_COLOR;
			}

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glInvalidateNamedFramebufferData(gl_fbo_, 1, &attachment);
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint old_fbo = re.BindFramebuffer();
				re.BindFramebuffer(gl_fbo_);

				glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

				re.BindFramebuffer(old_fbo);
			}
		}
		else
		{
			this->ClearColor(Color(0, 0, 0, 0));
		}
	}


	void OGLDepthStencilView::ClearDepth(float depth)
	{
		this->DoClearDepthStencil(GL_DEPTH_BUFFER_BIT, depth, 0);
	}

	void OGLDepthStencilView::ClearStencil(int32_t stencil)
	{
		this->DoClearDepthStencil(GL_STENCIL_BUFFER_BIT, 0, stencil);
	}

	void OGLDepthStencilView::ClearDepthStencil(float depth, int32_t stencil)
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

		this->DoClearDepthStencil(flags, depth, stencil);
	}

	GLuint OGLDepthStencilView::RetrieveGLTexture() const
	{
		if ((gl_tex_ == 0) && (tex_->HWResourceReady()))
		{
			gl_tex_ = checked_cast<OGLTexture&>(*tex_).GLTexture();
		}
		return gl_tex_;
	}

	void OGLDepthStencilView::DoClearDepthStencil(uint32_t flags, float depth, int32_t stencil)
	{
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		DepthStencilStateDesc const & ds_desc = re.CurRenderStateObject()->GetDepthStencilStateDesc();

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

	void OGLDepthStencilView::DoDiscardDepthStencil()
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
		{
			GLenum attachments[2];
			if (gl_fbo_ != 0)
			{
				attachments[0] = GL_DEPTH_ATTACHMENT;
				attachments[1] = GL_STENCIL_ATTACHMENT;
			}
			else
			{
				attachments[0] = GL_DEPTH;
				attachments[1] = GL_STENCIL;
			}

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glInvalidateNamedFramebufferData(gl_fbo_, 2, attachments);
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint old_fbo = re.BindFramebuffer();
				re.BindFramebuffer(gl_fbo_);

				glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

				re.BindFramebuffer(old_fbo);
			}
		}
		else
		{
			this->ClearDepthStencil(1, 0);
		}
	}


	OGLScreenRenderTargetView::OGLScreenRenderTargetView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLScreenRenderTargetView::ClearColor(Color const & clr)
	{
		this->DoClearColor(clr);
	}

	void OGLScreenRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLScreenRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer&>(fb).OGLFbo());

		index_ = static_cast<uint32_t>(att);

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer&>(fb).OGLFbo());

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	GLuint OGLScreenRenderTargetView::RetrieveGLTexture() const
	{
		return gl_tex_;
	}


	OGLScreenDepthStencilView::OGLScreenDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLScreenDepthStencilView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLScreenDepthStencilView::OnAttached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer&>(fb).OGLFbo());

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenDepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer&>(fb).OGLFbo());

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	GLuint OGLScreenDepthStencilView::RetrieveGLTexture() const
	{
		return gl_tex_;
	}


	OGLTexture1DRenderTargetView::OGLTexture1DRenderTargetView(TexturePtr const & texture_1d, ElementFormat pf, int array_index,
		int array_size, int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_1d->ArraySize())));

		tex_ = texture_1d;

		width_ = texture_1d->Width(level);
		height_ = 1;
		pf_ = pf == EF_Unknown ? texture_1d->Format() : pf;
		sample_count_ = texture_1d->SampleCount();
		sample_quality_ = texture_1d->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLTexture1DRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLTexture&>(*tex_).GLType();

			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage1D(gl_target, level_, 0, width_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture1DRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture1DRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);
		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		GLenum const gl_target = checked_cast<OGLTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_1D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture1DEXT(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + index_, gl_target, gl_tex_, level_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture1D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + index_, gl_target, gl_tex_, level_);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index_,
											GL_RENDERBUFFER, gl_tex_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index_,
											GL_RENDERBUFFER, gl_tex_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + index_,
											GL_RENDERBUFFER, gl_tex_);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_,
						gl_tex_, level_, array_index_);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_,
						gl_tex_, level_, array_index_);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_,
						gl_tex_, level_, array_index_);
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTexture1DRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		uint32_t const index = static_cast<uint32_t>(att);
		GLenum const gl_target = checked_cast<OGLTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_1D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture1DEXT(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + index, gl_target, 0, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture1D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + index, gl_target, 0, 0);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index,
											GL_RENDERBUFFER, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index,
											GL_RENDERBUFFER, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + index,
											GL_RENDERBUFFER, 0);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLTexture2DRenderTargetView::OGLTexture2DRenderTargetView(TexturePtr const & texture_2d, ElementFormat pf, int array_index,
		int array_size, int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_2d->ArraySize())));

		tex_ = texture_2d;

		width_ = texture_2d->Width(level);
		height_ = texture_2d->Height(level);
		pf_ = pf == EF_Unknown ? texture_2d->Format() : pf;
		sample_count_ = texture_2d->SampleCount();
		sample_quality_ = texture_2d->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLTexture2DRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLTexture&>(*tex_).GLType();

			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(gl_target, level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture2DRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture2DRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);
		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		GLenum const gl_target = checked_cast<OGLTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_2D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture2DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index_, gl_target, gl_tex_, level_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + index_, gl_target, gl_tex_, level_);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index_,
											GL_RENDERBUFFER, gl_tex_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index_,
											GL_RENDERBUFFER, gl_tex_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + index_,
											GL_RENDERBUFFER, gl_tex_);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_,
						gl_tex_, level_, array_index_);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index_,
						gl_tex_, level_, array_index_);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_,
						gl_tex_, level_, array_index_);
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTexture2DRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		uint32_t const index = static_cast<uint32_t>(att);
		GLenum const gl_target = checked_cast<OGLTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_2D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + index, gl_target, 0, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + index, gl_target, 0, 0);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index,
											GL_RENDERBUFFER, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + index,
											GL_RENDERBUFFER, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + index,
											GL_RENDERBUFFER, 0);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, 0, 0);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLTexture3DRenderTargetView::OGLTexture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t slice, int level)
		: slice_(slice), level_(level), copy_to_tex_(0)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_3D == texture_3d->Type());
		BOOST_ASSERT(texture_3d->Depth(level) > slice);
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_3d;

		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();

		this->RetrieveGLTexture();
	}

	OGLTexture3DRenderTargetView::~OGLTexture3DRenderTargetView()
	{
		if (2 == copy_to_tex_)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteTextures(1, &gl_tex_2d_);
			}
			else
			{
				glDeleteTextures(1, &gl_tex_2d_);
			}
		}
	}

	void OGLTexture3DRenderTargetView::ClearColor(Color const & clr)
	{
		BOOST_ASSERT(gl_fbo_ != 0);

		this->DoClearColor(clr);
	}

	void OGLTexture3DRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture3DRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			if (0 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, gl_tex_, level_, slice_);

				GLenum status = glCheckNamedFramebufferStatusEXT(gl_fbo_, GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE)
				{
					glGenTextures(1, &gl_tex_2d_);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTextureImage2DEXT(gl_tex_2d_, GL_TEXTURE_RECTANGLE_ARB,
							   0, GL_RGBA32F_ARB,
							   width_, height_, 0,
							   GL_RGBA, GL_FLOAT, nullptr);

					copy_to_tex_ = 2;
				}
				else
				{
					copy_to_tex_ = 1;
				}
			}

			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index_,
						GL_TEXTURE_3D, gl_tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index_,
						GL_TEXTURE_RECTANGLE_ARB, gl_tex_2d_, 0);
			}
		}
		else
		{
			re.BindFramebuffer(gl_fbo_);

			if (0 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, gl_tex_, level_, slice_);

				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE)
				{
					glGenTextures(1, &gl_tex_2d_);
					glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gl_tex_2d_);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
						0, GL_RGBA, GL_FLOAT, nullptr);

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
						GL_COLOR_ATTACHMENT0 + index_,
						GL_TEXTURE_3D, gl_tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + index_,
						GL_TEXTURE_RECTANGLE_ARB, gl_tex_2d_, 0);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		uint32_t const index = static_cast<uint32_t>(att);

		if (glloader_GL_EXT_direct_state_access())
		{
			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index,
						GL_TEXTURE_RECTANGLE_ARB, 0, 0);

				this->CopyToSlice(att);
			}
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + index,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + index,
						GL_TEXTURE_RECTANGLE_ARB, 0, 0);

				this->CopyToSlice(att);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderTargetView::OnUnbind(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(copy_to_tex_ != 0);
		if (2 == copy_to_tex_)
		{
			this->CopyToSlice(att);
		}
	}

	void OGLTexture3DRenderTargetView::CopyToSlice(FrameBuffer::Attachment att)
	{
		uint32_t const index = static_cast<uint32_t>(att);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, GL_TEXTURE_3D, gl_tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLTextureCubeRenderTargetView::OGLTextureCubeRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_cube;

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		this->RetrieveGLTexture();
	}

	OGLTextureCubeRenderTargetView::OGLTextureCubeRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		int level)
		: face_(static_cast<Texture::CubeFaces>(-1)), level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_cube;

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLTextureCubeRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_CUBE_MAP, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTextureCubeRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTextureCubeRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);

		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		if (face_ >= 0)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (!re.HackForIntel() && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
			{
				glNamedFramebufferTextureLayer(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_, face_);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_, face);
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + index_,
						face, gl_tex_, level_);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTexture(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_);
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_);

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTextureCubeRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		uint32_t const index = static_cast<uint32_t>(att);

		if (face_ >= 0)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTextureLayer(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + index,
						0, 0, 0);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTexture(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + index,
					0, 0);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + index,
					0, 0);
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index,
					0, 0);

				re.BindFramebuffer(0);
			}
		}
	}


	OGLGraphicsBufferRenderTargetView::OGLGraphicsBufferRenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		buff_ = gb;
		width_ = num_elems;
		height_ = 1;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		glGenTextures(1, &gl_tex_);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gl_tex_);
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
			0, GL_RGBA, GL_FLOAT, nullptr);
	}

	OGLGraphicsBufferRenderTargetView::~OGLGraphicsBufferRenderTargetView()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeleteTextures(1, &gl_tex_);
		}
		else
		{
			glDeleteTextures(1, &gl_tex_);
		}
	}

	void OGLGraphicsBufferRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_RECTANGLE_ARB, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLGraphicsBufferRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLGraphicsBufferRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);

		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture2DEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + index_,
					GL_TEXTURE_RECTANGLE_ARB, gl_tex_, 0);
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_,
					GL_TEXTURE_RECTANGLE_ARB, gl_tex_, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		uint32_t const index = static_cast<uint32_t>(att);

		if (glloader_GL_EXT_direct_state_access())
		{
			this->CopyToGB(att);

			glNamedFramebufferTexture2DEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + index,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			this->CopyToGB(att);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderTargetView::OnUnbind(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		this->CopyToGB(att);

		re.BindFramebuffer(0);
	}

	void OGLGraphicsBufferRenderTargetView::CopyToGB(FrameBuffer::Attachment att)
	{
		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + static_cast<uint32_t>(att));

		OGLGraphicsBuffer& ogl_gb = checked_cast<OGLGraphicsBuffer&>(*buff_);
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(GL_PIXEL_PACK_BUFFER, ogl_gb.GLvbo());
		glReadPixels(0, 0, width_, height_, glformat, gltype, nullptr);
	}

	GLuint OGLGraphicsBufferRenderTargetView::RetrieveGLTexture() const
	{
		return gl_tex_;
	}


	OGLTextureDepthStencilView::OGLTextureDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf,
		uint32_t sample_count, uint32_t sample_quality)
		: target_type_(0), array_index_(0), level_(-1), sample_count_(sample_count), sample_quality_(sample_quality)
	{
		BOOST_ASSERT(IsDepthFormat(pf));
		KFL_UNUSED(sample_count_);
		KFL_UNUSED(sample_quality_);

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glCreateRenderbuffers(1, &gl_rbo_);
			if (sample_count <= 1)
			{
				glNamedRenderbufferStorage(gl_rbo_, internalFormat, width_, height_);
			}
			else
			{
				glNamedRenderbufferStorageMultisample(gl_rbo_, sample_count, internalFormat, width_, height_);
			}
		}
		else
		{
			glGenRenderbuffers(1, &gl_rbo_);
			glBindRenderbuffer(GL_RENDERBUFFER, gl_rbo_);
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
	}

	OGLTextureDepthStencilView::OGLTextureDepthStencilView(TexturePtr const & texture, ElementFormat pf, int array_index,
		int array_size, int level)
		: target_type_(checked_cast<OGLTexture&>(*texture).GLType()),
			array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT((Texture::TT_2D == texture->Type()) || (Texture::TT_Cube == texture->Type()));
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture->ArraySize())));
		BOOST_ASSERT(IsDepthFormat(texture->Format()));

		tex_ = texture;

		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();

		this->RetrieveGLTexture();
	}

	OGLTextureDepthStencilView::~OGLTextureDepthStencilView()
	{
		glDeleteRenderbuffers(1, &gl_rbo_);
	}

	void OGLTextureDepthStencilView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLTextureDepthStencilView::OnAttached(FrameBuffer& fb)
	{
		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		if (level_ < 0)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				}
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				if (IsStencilFormat(pf_))
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				}

				re.BindFramebuffer(0);
			}
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_DEPTH_ATTACHMENT, target_type_, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_STENCIL_ATTACHMENT, target_type_, gl_tex_, level_);
					}
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_DEPTH_ATTACHMENT, target_type_, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_STENCIL_ATTACHMENT, target_type_, gl_tex_, level_);
					}

					re.BindFramebuffer(0);
				}
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (array_size_ > 1)
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
						}

						re.BindFramebuffer(0);
					}
				}
				else
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
						}

						re.BindFramebuffer(0);
					}
				}
			}
		}
	}

	void OGLTextureDepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		if (level_ < 0)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

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
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_DEPTH_ATTACHMENT, target_type_, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_STENCIL_ATTACHMENT, target_type_, 0, 0);
					}
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

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
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

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
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

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
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

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


	OGLTextureCubeFaceDepthStencilView::OGLTextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(IsDepthFormat(texture_cube->Format()));
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		tex_ = texture_cube;

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLTextureCubeFaceDepthStencilView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLTextureCubeFaceDepthStencilView::OnAttached(FrameBuffer& fb)
	{
		gl_fbo_ = checked_cast<OGLFrameBuffer&>(fb).OGLFbo();
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, face_);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, face_);
			}
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, face);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, face);
			}
		}
		else
		{
			re.BindFramebuffer(gl_fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT, face, gl_tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_STENCIL_ATTACHMENT, face, gl_tex_, level_);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTextureCubeFaceDepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
			}
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
			}
		}
		else
		{
			re.BindFramebuffer(gl_fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
			}

			re.BindFramebuffer(0);
		}
	}
}
