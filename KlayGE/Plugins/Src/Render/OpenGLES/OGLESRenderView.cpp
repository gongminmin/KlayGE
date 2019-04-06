// OGLESRenderView.cpp
// KlayGE OpenGL ES 2渲染视图类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>

namespace KlayGE
{
	OGLESTextureShaderResourceView::OGLESTextureShaderResourceView(TexturePtr const & texture)
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

	void OGLESTextureShaderResourceView::RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const
	{
		if ((gl_tex_ == 0) && tex_ && tex_->HWResourceReady())
		{
			gl_target_ = checked_cast<OGLESTexture&>(*tex_).GLType();
			gl_tex_ = checked_cast<OGLESTexture&>(*tex_).GLTexture();
		}
		target = gl_target_;
		tex = gl_tex_;
	}


	OGLESBufferShaderResourceView::OGLESBufferShaderResourceView(GraphicsBufferPtr const & gbuffer, ElementFormat pf)
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

		gl_target_ = GL_TEXTURE_BUFFER_OES;
		gl_tex_ = 0;
	}

	void OGLESBufferShaderResourceView::RetrieveGLTargetTexture(GLuint& target, GLuint& tex) const
	{
		if ((gl_tex_ == 0) && buff_ && buff_->HWResourceReady())
		{
			gl_tex_ = checked_cast<OGLESGraphicsBuffer&>(*buff_).RetrieveGLTexture(pf_);
		}
		target = gl_target_;
		tex = gl_tex_;
	}


	GLuint OGLESRenderTargetView::RetrieveGLTexture() const
	{
		if ((gl_tex_ == 0) && (tex_->HWResourceReady()))
		{
			gl_tex_ = checked_cast<OGLESTexture&>(*tex_).GLTexture();
		}
		return gl_tex_;
	}

	void OGLESRenderTargetView::DoClearColor(Color const & clr)
	{
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		BlendStateDesc const & blend_desc = re.CurRenderStateObject()->GetBlendStateDesc();

		if (blend_desc.color_write_mask[0] != CMASK_All)
		{
			glColorMask(true, true, true, true);
		}

		glClearBufferfv(GL_COLOR, index_, &clr[0]);

		if (blend_desc.color_write_mask[0] != CMASK_All)
		{
			glColorMask((blend_desc.color_write_mask[0] & CMASK_Red) != 0,
				(blend_desc.color_write_mask[0] & CMASK_Green) != 0,
				(blend_desc.color_write_mask[0] & CMASK_Blue) != 0,
				(blend_desc.color_write_mask[0] & CMASK_Alpha) != 0);
		}

		re.BindFramebuffer(old_fbo);
	}

	void OGLESRenderTargetView::DoDiscardColor()
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

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

		re.BindFramebuffer(old_fbo);
	}


	void OGLESDepthStencilView::ClearDepth(float depth)
	{
		this->DoClearDepthStencil(GL_DEPTH_BUFFER_BIT, depth, 0);
	}

	void OGLESDepthStencilView::ClearStencil(int32_t stencil)
	{
		this->DoClearDepthStencil(GL_STENCIL_BUFFER_BIT, 0, stencil);
	}

	void OGLESDepthStencilView::ClearDepthStencil(float depth, int32_t stencil)
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
	
	GLuint OGLESDepthStencilView::RetrieveGLTexture() const
	{
		if ((gl_tex_ == 0) && (tex_->HWResourceReady()))
		{
			gl_tex_ = checked_cast<OGLESTexture&>(*tex_).GLTexture();
		}
		return gl_tex_;
	}

	void OGLESDepthStencilView::DoClearDepthStencil(uint32_t flags, float depth, int32_t stencil)
	{
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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

	void OGLESDepthStencilView::DoDiscardDepthStencil()
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

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

		re.BindFramebuffer(old_fbo);
	}


	OGLESScreenRenderTargetView::OGLESScreenRenderTargetView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLESScreenRenderTargetView::ClearColor(Color const & clr)
	{
		this->DoClearColor(clr);
	}

	void OGLESScreenRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESScreenRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		index_ = static_cast<uint32_t>(att);

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLESScreenRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}
	
	GLuint OGLESScreenRenderTargetView::RetrieveGLTexture() const
	{
		return gl_tex_;
	}


	OGLESScreenDepthStencilView::OGLESScreenDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLESScreenDepthStencilView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESScreenDepthStencilView::OnAttached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLESScreenDepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}
	
	GLuint OGLESScreenDepthStencilView::RetrieveGLTexture() const
	{
		return gl_tex_;
	}


	OGLESTexture1DRenderTargetView::OGLESTexture1DRenderTargetView(TexturePtr const & texture_1d, ElementFormat pf, int array_index,
		int array_size, int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_1d->ArraySize())));

		if (array_index > 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		tex_ = texture_1d;

		width_ = texture_1d->Width(level);
		height_ = 1;
		pf_ = pf == EF_Unknown ? texture_1d->Format() : pf;
		sample_count_ = texture_1d->SampleCount();
		sample_quality_ = texture_1d->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLESTexture1DRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLESTexture&>(*tex_).GLType();

			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage2D(gl_target, level_, 0, 0, width_, 1, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTexture1DRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture1DRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);
		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		GLenum const gl_target = checked_cast<OGLESTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_, gl_target, gl_tex_, level_);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_, array_index_);
			}
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture1DRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		uint32_t const index = static_cast<uint32_t>(att);
		GLenum const gl_target = checked_cast<OGLESTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index, gl_target, 0, 0);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, 0, 0);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
					0, 0, 0);
			}
		}

		re.BindFramebuffer(0);
	}


	OGLESTexture2DRenderTargetView::OGLESTexture2DRenderTargetView(TexturePtr const & texture_2d, ElementFormat pf, int array_index,
		int array_size, int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_2d->ArraySize())));

		if (array_index > 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		tex_ = texture_2d;

		width_ = texture_2d->Width(level);
		height_ = texture_2d->Height(level);
		pf_ = pf == EF_Unknown ? texture_2d->Format() : pf;
		sample_count_ = texture_2d->SampleCount();
		sample_quality_ = texture_2d->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLESTexture2DRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLESTexture&>(*tex_).GLType();

			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(gl_target, level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTexture2DRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture2DRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);
		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		GLenum const gl_target = checked_cast<OGLESTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_, gl_target, gl_tex_, level_);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_, gl_tex_, level_);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index_,
					gl_tex_, level_, array_index_);
			}
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture2DRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		uint32_t const index = static_cast<uint32_t>(att);
		GLenum const gl_target = checked_cast<OGLESTexture&>(*tex_).GLType();

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index, gl_target, 0, 0);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, 0, 0);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
					0, 0, 0);
			}
		}

		re.BindFramebuffer(0);
	}


	OGLESTexture3DRenderTargetView::OGLESTexture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
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

	OGLESTexture3DRenderTargetView::~OGLESTexture3DRenderTargetView()
	{
		if (2 == copy_to_tex_)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteTextures(1, &gl_tex_2d_);
			}
			else
			{
				glDeleteTextures(1, &gl_tex_2d_);
			}
		}
	}

	void OGLESTexture3DRenderTargetView::ClearColor(Color const & clr)
	{
		BOOST_ASSERT(gl_fbo_ != 0);

		this->DoClearColor(clr);
	}

	void OGLESTexture3DRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture3DRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);

		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (0 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D_OES, gl_tex_, level_, slice_);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (GL_FRAMEBUFFER_COMPLETE == status)
			{
				glGenTextures(1, &gl_tex_2d_);
				glBindTexture(GL_TEXTURE_2D, gl_tex_2d_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_,
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
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_,
					GL_TEXTURE_3D_OES, gl_tex_, level_, slice_);
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_,
					GL_TEXTURE_2D, gl_tex_2d_, 0);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture3DRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		uint32_t const index = static_cast<uint32_t>(att);

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		BOOST_ASSERT(copy_to_tex_ != 0);
		if (1 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index,
					GL_TEXTURE_3D_OES, 0, 0, 0);
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index,
					GL_TEXTURE_2D, 0, 0);

			this->CopyToSlice(att);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture3DRenderTargetView::OnUnbind(FrameBuffer& /*fb*/, FrameBuffer::Attachment att)
	{
		BOOST_ASSERT(copy_to_tex_ != 0);
		if (2 == copy_to_tex_)
		{
			this->CopyToSlice(att);
		}
	}

	void OGLESTexture3DRenderTargetView::CopyToSlice(FrameBuffer::Attachment att)
	{
		KFL_UNUSED(att);

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, GL_TEXTURE_3D, gl_tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLESTextureCubeRenderTargetView::OGLESTextureCubeRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
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

	OGLESTextureCubeRenderTargetView::OGLESTextureCubeRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
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

	void OGLESTextureCubeRenderTargetView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClearColor(clr);
		}
		else
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_CUBE_MAP, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTextureCubeRenderTargetView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTextureCubeRenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);

		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);
		if (face_ >= 0)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index_,
					face, gl_tex_, level_);
		}
		else
		{
			glFramebufferTextureEXT(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + index_,
				gl_tex_, level_);
		}
		re.BindFramebuffer(0);
	}

	void OGLESTextureCubeRenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());

		uint32_t const index = static_cast<uint32_t>(att);

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		if (face_ >= 0)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index,
					face, 0, 0);
		}
		else
		{
			glFramebufferTextureEXT(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + index,
					0, 0);
		}

		re.BindFramebuffer(0);
	}


	OGLESTextureDepthStencilView::OGLESTextureDepthStencilView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: target_type_(0), array_index_(0), level_(-1),
			sample_count_(sample_count), sample_quality_(sample_quality)
	{
		KFL_UNUSED(array_index_);
		KFL_UNUSED(level_);
		KFL_UNUSED(sample_count_);
		KFL_UNUSED(sample_quality_);
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLESMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		switch (internalFormat)
		{
		case GL_DEPTH_COMPONENT:
			internalFormat = GL_DEPTH_COMPONENT16;
			break;

		case GL_DEPTH_STENCIL_OES:
			internalFormat = GL_DEPTH24_STENCIL8_OES;
			break;

		default:
			break;
		}

		glGenRenderbuffers(1, gl_rbos_);
		glBindRenderbuffer(GL_RENDERBUFFER, gl_rbos_[0]);
		glRenderbufferStorage(GL_RENDERBUFFER,
								internalFormat, width_, height_);
		gl_rbos_[1] = gl_rbos_[0];
	}

	OGLESTextureDepthStencilView::OGLESTextureDepthStencilView(TexturePtr const & texture, ElementFormat pf, int array_index, int array_size,
		int level)
		: target_type_(checked_cast<OGLESTexture&>(*texture).GLType()),
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

	OGLESTextureDepthStencilView::~OGLESTextureDepthStencilView()
	{
		if (gl_rbos_[0] == gl_rbos_[1])
		{
			glDeleteRenderbuffers(1, gl_rbos_);
		}
		else
		{
			glDeleteRenderbuffers(2, gl_rbos_);
		}
	}

	void OGLESTextureDepthStencilView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESTextureDepthStencilView::OnAttached(FrameBuffer& fb)
	{
		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		if (level_ < 0)
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_DEPTH_ATTACHMENT,
									GL_RENDERBUFFER, gl_rbos_[0]);
			if (IsStencilFormat(pf_))
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_STENCIL_ATTACHMENT,
									GL_RENDERBUFFER, gl_rbos_[1]);
			}

			re.BindFramebuffer(0);
		}
		else
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
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
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				re.BindFramebuffer(gl_fbo_);

				if (IsDepthFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
				}

				re.BindFramebuffer(0);
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
					}
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLESTextureDepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());
		if (level_ < 0)
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_DEPTH_ATTACHMENT,
									GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_STENCIL_ATTACHMENT,
									GL_RENDERBUFFER, 0);

			re.BindFramebuffer(0);
		}
		else
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
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
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				re.BindFramebuffer(gl_fbo_);

				if (IsDepthFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
				}

				re.BindFramebuffer(0);
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
					}
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLESTextureCubeFaceDepthStencilView::OGLESTextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(IsDepthFormat(texture_cube->Format()));

		if (array_index > 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		tex_ = texture_cube;

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		this->RetrieveGLTexture();
	}

	void OGLESTextureCubeFaceDepthStencilView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTextureCubeFaceDepthStencilView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESTextureCubeFaceDepthStencilView::OnAttached(FrameBuffer& fb)
	{
		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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

	void OGLESTextureCubeFaceDepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

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

#if defined(KLAYGE_PLATFORM_IOS)
	OGLESEAGLRenderView::OGLESEAGLRenderView(ElementFormat pf)
	{
		glGenRenderbuffers(1, &gl_rf_);
		glBindRenderbuffer(GL_RENDERBUFFER, gl_rf_);
		
		WindowPtr const & app_window = KlayGE::Context::Instance().AppInstance().MainWnd();
		app_window->CreateColorRenderBuffer(pf);
		
		pf_ = pf;
		GLint param;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &param);
		width_ = param;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &param);
		height_ = param;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &param);
		sample_count_ = param;
		sample_quality_ = 0;
	}
	
	void OGLESEAGLRenderView::ClearColor(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}
	
	void OGLESEAGLRenderView::Discard()
	{
		this->DoDiscardColor();
	}
	
	void OGLESEAGLRenderView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		index_ = static_cast<uint32_t>(att);
		gl_fbo_ = checked_cast<OGLESFrameBuffer&>(fb).OGLFbo();
		
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER, gl_rf_);
		
		re.BindFramebuffer(0);
	}

	void OGLESEAGLRenderView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer&>(fb).OGLFbo());
		
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER, gl_rf_);
		
		re.BindFramebuffer(0);
	}

	void OGLESEAGLRenderView::BindRenderBuffer()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, gl_rf_);
	}

	GLuint OGLESEAGLRenderView::RetrieveGLTexture() const
	{
		return gl_tex_;
	}
#endif
}
