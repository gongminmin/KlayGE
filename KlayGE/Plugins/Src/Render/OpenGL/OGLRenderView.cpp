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
#include <KlayGE/Context.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>

namespace KlayGE
{
	OGLRenderView::~OGLRenderView()
	{
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

	void OGLTexture1DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_1d_.Usage())
		{
			texture_1d_.Usage(Texture::TU_RenderTarget);
		}

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

	void OGLTexture2DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_2d_.Usage())
		{
			texture_2d_.Usage(Texture::TU_RenderTarget);
		}

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

	void OGLTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_3d_.Usage())
		{
			texture_3d_.Usage(Texture::TU_RenderTarget);
		}

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

		if (0 == copy_to_tex_)
		{
			glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, tex_, level_, slice_);

			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				glGenTextures(1, &tex_2d_);
				glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_2d_);
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
		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

	void OGLTextureCubeRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		if (Texture::TU_RenderTarget != texture_cube_.Usage())
		{
			texture_cube_.Usage(Texture::TU_RenderTarget);
		}

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		
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
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
			0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	OGLGraphicsBufferRenderView::~OGLGraphicsBufferRenderView()
	{
		glDeleteTextures(1, &tex_);
	}

	void OGLGraphicsBufferRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
				GL_TEXTURE_RECTANGLE_ARB, tex_, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLGraphicsBufferRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

		this->CopyToGB(att);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
				GL_TEXTURE_RECTANGLE_ARB, 0, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLGraphicsBufferRenderView::OnUnbind(FrameBuffer& fb, uint32_t att)
	{
		gbuffer_.Resize(width_ * height_ * 4 * sizeof(GL_FLOAT));

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

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

	void OGLDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_DEPTH_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, rbo_);

		if (IsStencilFormat(pf_))
		{
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_STENCIL_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, rbo_);
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void OGLDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		GLuint fbo = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_DEPTH_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
								GL_STENCIL_ATTACHMENT_EXT,
								GL_RENDERBUFFER_EXT, 0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
}
