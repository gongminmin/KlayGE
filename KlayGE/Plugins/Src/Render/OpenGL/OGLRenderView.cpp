// OGLRenderView.cpp
// KlayGE OGL渲染视图类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>

namespace
{
	bool CheckFrameBufferStatus()
	{
		GLenum status;
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return true;

		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			// choose different formats
			return false;

		default:
			BOOST_ASSERT(false);
			return false;
		}
	}
}

namespace KlayGE
{
	OGLRenderView::OGLRenderView()
		: tex_(0), fbo_(0)
	{
	}

	OGLRenderView::~OGLRenderView()
	{
	}

	void OGLRenderView::Clear(float depth)
	{
		this->DoClear(GL_DEPTH_BUFFER_BIT, Color(), depth, 0);
	}

	void OGLRenderView::Clear(int32_t stencil)
	{
		this->DoClear(GL_STENCIL_BUFFER_BIT, Color(), 0, stencil);
	}

	void OGLRenderView::Clear(float depth, int32_t stencil)
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
		GLint old_fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &old_fbo);

		if (static_cast<GLuint>(old_fbo) != fbo_)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
		}

		glClearColor(clr.r(), clr.g(), clr.b(), clr.a());
		glClearDepth(depth);
		glClearStencil(stencil);
		glClear(flags);

		if (static_cast<GLuint>(old_fbo) != fbo_)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, old_fbo);
		}
	}


	OGLScreenColorRenderView::OGLScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void OGLScreenColorRenderView::Clear(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLScreenColorRenderView::Clear(float /*depth*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenColorRenderView::Clear(int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenColorRenderView::Clear(float /*depth*/, int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenColorRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLScreenColorRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}


	OGLScreenDepthStencilRenderView::OGLScreenDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void OGLScreenDepthStencilRenderView::Clear(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLScreenDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}


	OGLTexture1DRenderView::OGLTexture1DRenderView(Texture& texture_1d, int level)
		: texture_1d_(*checked_cast<OGLTexture1D*>(&texture_1d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d.Type());

		tex_ = texture_1d_.GLTexture();

		width_ = texture_1d_.Width(level);
		height_ = 1;
		pf_ = texture_1d_.Format();
	}

	void OGLTexture1DRenderView::Clear(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_1D, tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage1D(GL_TEXTURE_1D, level_, 0, width_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture1DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_1d_.Usage())
		{
			texture_1d_.Usage(Texture::TU_RenderTarget);
		}

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_1D, tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_1D, tex_, level_);
			}
			break;

		default:
			glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_1D, tex_, level_);
			break;
		}
		BOOST_ASSERT(CheckFrameBufferStatus());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_1D, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_1D, 0, 0);
			}
			break;

		default:
			glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_1D, 0, 0);
			break;
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}


	OGLTexture2DRenderView::OGLTexture2DRenderView(Texture& texture_2d, int level)
		: texture_2d_(*checked_cast<OGLTexture2D*>(&texture_2d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d.Type());

		tex_ = texture_2d_.GLTexture();

		width_ = texture_2d_.Width(level);
		height_ = texture_2d_.Height(level);
		pf_ = texture_2d_.Format();
	}

	void OGLTexture2DRenderView::Clear(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_2D, level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture2DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_2d_.Usage())
		{
			texture_2d_.Usage(Texture::TU_RenderTarget);
		}

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, tex_, level_);
			}
			break;

		default:
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_2D, tex_, level_);
			break;
		}
		BOOST_ASSERT(CheckFrameBufferStatus());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
			}
			break;

		default:
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_2D, 0, 0);
			break;
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}


	OGLTexture3DRenderView::OGLTexture3DRenderView(Texture& texture_3d, uint32_t slice, int level)
		: texture_3d_(*checked_cast<OGLTexture3D*>(&texture_3d)),
			slice_(slice), level_(level), copy_to_tex_(0)
	{
		BOOST_ASSERT(Texture::TT_3D == texture_3d.Type());
		BOOST_ASSERT(texture_3d_.Depth(level) > slice);

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

	void OGLTexture3DRenderView::Clear(Color const & clr)
	{
		BOOST_ASSERT(fbo_ != 0);

		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_3d_.Usage())
		{
			texture_3d_.Usage(Texture::TU_RenderTarget);
		}

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		if (0 == copy_to_tex_)
		{
			glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, tex_, level_, slice_);

			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				glGenTextures(1, &tex_2d_);
				glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_2d_);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
					0, GL_RGBA, GL_FLOAT, NULL);
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
			switch (att)
			{
			case FrameBuffer::ATT_DepthStencil:
				if (IsDepthFormat(pf_))
				{
					glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
						GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_3D, tex_, level_, slice_);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
						GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_3D, tex_, level_, slice_);
				}
				break;

			default:
				glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_3D, tex_, level_, slice_);
				break;
			}
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			switch (att)
			{
			case FrameBuffer::ATT_DepthStencil:
				if (IsDepthFormat(pf_))
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
				}
				break;

			default:
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
				break;
			}
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTexture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		BOOST_ASSERT(copy_to_tex_ != 0);
		if (1 == copy_to_tex_)
		{
			switch (att)
			{
			case FrameBuffer::ATT_DepthStencil:
				if (IsDepthFormat(pf_))
				{
					glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
						GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_3D, 0, 0, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
						GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_3D, 0, 0, 0);
				}
				break;

			default:
				glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_3D, 0, 0, 0);
				break;
			}
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			switch (att)
			{
			case FrameBuffer::ATT_DepthStencil:
				if (IsDepthFormat(pf_))
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, 0, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, 0, 0);
				}
				break;

			default:
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);
				break;
			}

			this->CopyToSlice(att);
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			glReadBuffer(GL_DEPTH_ATTACHMENT_EXT);
			break;

		default:
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0);
			break;
		}

		glBindTexture(GL_TEXTURE_3D, tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLTextureCubeRenderView::OGLTextureCubeRenderView(Texture& texture_cube, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<OGLTextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());

		tex_ = texture_cube_.GLTexture();

		width_ = texture_cube_.Width(level);
		height_ = texture_cube_.Height(level);
		pf_ = texture_cube_.Format();
	}

	void OGLTextureCubeRenderView::Clear(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTextureCubeRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_cube_.Usage())
		{
			texture_cube_.Usage(Texture::TU_RenderTarget);
		}

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, face, tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, face, tex_, level_);
			}
			break;

		default:
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
				face, tex_, level_);
			break;
		}
		BOOST_ASSERT(CheckFrameBufferStatus());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		switch (att)
		{
		case FrameBuffer::ATT_DepthStencil:
			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, face, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, face, 0, 0);
			}
			break;

		default:
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
				face, 0, 0);
			break;
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
			0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	OGLGraphicsBufferRenderView::~OGLGraphicsBufferRenderView()
	{
		glDeleteTextures(1, &tex_);
	}

	void OGLGraphicsBufferRenderView::Clear(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLGraphicsBufferRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
				GL_TEXTURE_RECTANGLE_ARB, tex_, 0);
		BOOST_ASSERT(CheckFrameBufferStatus());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLGraphicsBufferRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		this->CopyToGB(att);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
				GL_TEXTURE_RECTANGLE_ARB, 0, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLGraphicsBufferRenderView::OnUnbind(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		gbuffer_.Resize(width_ * height_ * 4 * sizeof(GL_FLOAT));

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		this->CopyToGB(att);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLGraphicsBufferRenderView::CopyToGB(uint32_t att)
	{
		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0);

		OGLGraphicsBuffer* ogl_gb = checked_cast<OGLGraphicsBuffer*>(&gbuffer_);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ogl_gb->OGLvbo());
		glReadPixels(0, 0, width_, height_, glformat, gltype, 0);
	}


	OGLDepthStencilRenderView::OGLDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t multi_sample)
		: multi_sample_(multi_sample)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		glGenRenderbuffersEXT(1, &rbo_);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo_);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
								glformat, width_, height_);
	}

	OGLDepthStencilRenderView::~OGLDepthStencilRenderView()
	{
		glDeleteRenderbuffersEXT(1, &rbo_);
	}

	void OGLDepthStencilRenderView::Clear(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_DEPTH_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, rbo_);
		if (IsStencilFormat(pf_))
		{
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_STENCIL_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, rbo_);
		}
		BOOST_ASSERT(CheckFrameBufferStatus());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_DEPTH_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_STENCIL_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
}
