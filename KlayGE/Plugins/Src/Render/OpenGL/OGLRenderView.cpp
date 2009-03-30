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
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(fbo_);

		DepthStencilStateDesc const & cur_desc = re.CurDSSObj()->GetDesc();

		if (glloader_GL_VERSION_3_0())
		{
			if (flags & GL_COLOR_BUFFER_BIT)
			{
				glClearBufferfv(GL_COLOR, index_, &clr[0]);
			}

			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				if (!cur_desc.depth_write_mask)
				{
					glDepthMask(GL_TRUE);
				}
			}
			if (flags & GL_STENCIL_BUFFER_BIT)
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

			/*GLenum ogl_buff = 0;
			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				if (flags & GL_STENCIL_BUFFER_BIT)
				{
					ogl_buff = GL_DEPTH_STENCIL;
				}
				else
				{
					ogl_buff = GL_DEPTH;
				}
			}
			else
			{
				if (flags & GL_STENCIL_BUFFER_BIT)
				{
					ogl_buff = GL_STENCIL;
				}
			}
			if (ogl_buff != 0)
			{
				glClearBufferfi(ogl_buff, 0, depth, stencil);
			}*/

			flags &= ~GL_COLOR_BUFFER_BIT;
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

			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				if (!cur_desc.depth_write_mask)
				{
					glDepthMask(GL_FALSE);
				}
			}
			if (flags & GL_STENCIL_BUFFER_BIT)
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
		}
		else
		{
			glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			if (flags & GL_COLOR_BUFFER_BIT)
			{
				re.ClearColor(clr.r(), clr.g(), clr.b(), clr.a());
			}
			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				re.ClearDepth(depth);

				if (!cur_desc.depth_write_mask)
				{
					glDepthMask(GL_TRUE);
				}
			}
			if (flags & GL_STENCIL_BUFFER_BIT)
			{
				re.ClearStencil(stencil);

				if (!cur_desc.front_stencil_write_mask)
				{
					glStencilMaskSeparate(GL_FRONT, GL_TRUE);
				}
				if (!cur_desc.back_stencil_write_mask)
				{
					glStencilMaskSeparate(GL_BACK, GL_TRUE);
				}
			}

			glClear(flags);

			glPopAttrib();
		}

		re.BindFramebuffer(old_fbo);
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

	void OGLScreenColorRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		index_ = att - FrameBuffer::ATT_Color0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenColorRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

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

	void OGLScreenDepthStencilRenderView::Clear(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLScreenDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		index_ = 0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLTexture1DRenderView::OGLTexture1DRenderView(Texture& texture_1d, int level)
		: texture_1d_(*checked_cast<OGLTexture1D*>(&texture_1d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d.Type());

		uint32_t const channels = NumComponents(texture_1d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(boost::system::posix_error::not_supported);
		}

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
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture1DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_1D, tex_, level_);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_1D, tex_, level_);

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture1DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_1D, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_1D, 0, 0);

			re.BindFramebuffer(0);
		}
	}


	OGLTexture2DRenderView::OGLTexture2DRenderView(Texture& texture_2d, int level)
		: texture_2d_(*checked_cast<OGLTexture2D*>(&texture_2d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d.Type());

		uint32_t const channels = NumComponents(texture_2d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(boost::system::posix_error::not_supported);
		}

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
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_2D, tex_, level_);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_2D, tex_, level_);

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_2D, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0, GL_TEXTURE_2D, 0, 0);

			re.BindFramebuffer(0);
		}
	}


	OGLTexture3DRenderView::OGLTexture3DRenderView(Texture& texture_3d, uint32_t slice, int level)
		: texture_3d_(*checked_cast<OGLTexture3D*>(&texture_3d)),
			slice_(slice), level_(level), copy_to_tex_(0)
	{
		BOOST_ASSERT(Texture::TT_3D == texture_3d.Type());
		BOOST_ASSERT(texture_3d_.Depth(level) > slice);

		uint32_t const channels = NumComponents(texture_3d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(boost::system::posix_error::not_supported);
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

	void OGLTexture3DRenderView::Clear(Color const & clr)
	{
		BOOST_ASSERT(fbo_ != 0);

		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			if (0 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, tex_, level_, slice_);

				GLenum status = glCheckNamedFramebufferStatusEXT(fbo_, GL_FRAMEBUFFER_EXT);
				if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
				{
					glGenTextures(1, &tex_2d_);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTextureImage2DEXT(tex_2d_, GL_TEXTURE_RECTANGLE_ARB,
                               0, GL_RGBA32F_ARB,
                               width_, height_, 0,
                               GL_RGBA, GL_FLOAT, NULL);
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
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

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
				glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, tex_2d_, 0);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		if (glloader_GL_EXT_direct_state_access())
		{
			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(fbo_,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(fbo_,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
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
				glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
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

		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0);

		glBindTexture(GL_TEXTURE_3D, tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLTextureCubeRenderView::OGLTextureCubeRenderView(Texture& texture_cube, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<OGLTextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());

		uint32_t const channels = NumComponents(texture_cube.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())))
		{
			THR(boost::system::posix_error::not_supported);
		}

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
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					face, tex_, level_);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					face, tex_, level_);

			re.BindFramebuffer(0);
		}
	}

	void OGLTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		if (glloader_GL_EXT_direct_state_access())
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					face, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					face, 0, 0);

			re.BindFramebuffer(0);
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

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, tex_, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, tex_, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		if (glloader_GL_EXT_direct_state_access())
		{
			this->CopyToGB(att);

			glNamedFramebufferTexture2DEXT(fbo_,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			this->CopyToGB(att);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderView::OnUnbind(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		gbuffer_.Resize(width_ * height_ * 4 * sizeof(GL_FLOAT));

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
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

		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + att - FrameBuffer::ATT_Color0);

		OGLGraphicsBuffer* ogl_gb = checked_cast<OGLGraphicsBuffer*>(&gbuffer_);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ogl_gb->OGLvbo());
		glReadPixels(0, 0, width_, height_, glformat, gltype, 0);
	}


	OGLDepthStencilRenderView::OGLDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: level_(-1),
			sample_count_(sample_count), sample_quality_(sample_quality)
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

	OGLDepthStencilRenderView::OGLDepthStencilRenderView(Texture& texture, int level)
		: level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture.Type());
		BOOST_ASSERT(IsDepthFormat(texture.Format()));

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();

		tex_ = checked_cast<OGLTexture2D*>(&texture)->GLTexture();
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

		index_ = 0;

		fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (level_ < 0)
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(fbo_,
										GL_DEPTH_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, rbo_);
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferRenderbufferEXT(fbo_,
										GL_STENCIL_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, rbo_);
				}
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
										GL_DEPTH_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, rbo_);
				if (IsStencilFormat(pf_))
				{
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
										GL_STENCIL_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, rbo_);
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
					glNamedFramebufferTexture2DEXT(fbo_,
						GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex_, level_);
				}
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferTexture2DEXT(fbo_,
						GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, tex_, level_);
				}
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

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

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		BOOST_ASSERT(fbo_ == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());
		if (level_ < 0)
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(fbo_,
										GL_DEPTH_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, 0);
				glNamedFramebufferRenderbufferEXT(fbo_,
										GL_STENCIL_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
										GL_DEPTH_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, 0);
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
										GL_STENCIL_ATTACHMENT_EXT,
										GL_RENDERBUFFER_EXT, 0);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_EXT_direct_state_access())
			{
				if (IsDepthFormat(pf_))
				{
					glNamedFramebufferTexture2DEXT(fbo_,
						GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferTexture2DEXT(fbo_,
						GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
				}
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(fbo_);

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

				re.BindFramebuffer(0);
			}
		}
	}
}
