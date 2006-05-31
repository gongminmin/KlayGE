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
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Util.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>

namespace KlayGE
{
	OGLFrameBuffer::OGLFrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		glGenFramebuffersEXT(1, &fbo_);
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		glDeleteFramebuffersEXT(1, &fbo_);
	}

	void OGLFrameBuffer::AttachTexture2D(uint32_t n, TexturePtr texture2D)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));
		BOOST_ASSERT(Texture::TT_2D == texture2D->Type());

		if (Texture::TU_RenderTarget != texture2D->Usage())
		{
			texture2D->Usage(Texture::TU_RenderTarget);
		}

		this->UpdateParams(n, texture2D);

		OGLTexture& ogl_tex = *checked_cast<OGLTexture*>(texture2D.get());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT + n, GL_TEXTURE_2D, ogl_tex.GLTexture(), 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		active_ = true;
	}
	
	void OGLFrameBuffer::AttachTextureCube(uint32_t n, TexturePtr textureCube, Texture::CubeFaces face)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));
		BOOST_ASSERT(Texture::TT_Cube == textureCube->Type());

		if (Texture::TU_RenderTarget != textureCube->Usage())
		{
			textureCube->Usage(Texture::TU_RenderTarget);
		}

		this->UpdateParams(n, textureCube);
		faces_[n] = face;

		OGLTexture& ogl_tex = *checked_cast<OGLTexture*>(textureCube.get());

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT + n, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face - Texture::CF_Positive_X,
			ogl_tex.GLTexture(), 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		active_ = true;
	}

	void OGLFrameBuffer::AttachGraphicsBuffer(uint32_t n, GraphicsBufferPtr gb,
			uint32_t width, uint32_t height)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));

		this->UpdateParams(n, gb, width, height);

		OGLGraphicsBuffer& ogl_gb = *checked_cast<OGLGraphicsBuffer*>(gbuffers_[n].get());
		glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ogl_gb.OGLvbo());
		glBufferData(GL_PIXEL_PACK_BUFFER_ARB,
			reinterpret_cast<GLsizeiptr>(width_ * height_ * 4 * sizeof(float)), NULL, GL_DYNAMIC_DRAW);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT + n, GL_TEXTURE_RECTANGLE_ARB,
			texs_[n], 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		active_ = true;
	}

	void OGLFrameBuffer::Detach(uint32_t n)
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));

		if (privateTexs_[n])
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + n, GL_TEXTURE_2D,
				0, 0);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}
		else
		{
			if (gbuffers_[n])
			{
				gbuffers_[n]->Resize(width_ * height_ * 4 * sizeof(GL_FLOAT));

				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

				OGLGraphicsBuffer& ogl_gb = static_cast<OGLGraphicsBuffer&>(*gbuffers_[n]);
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ogl_gb.OGLvbo());
				glReadPixels(0, 0, width_, height_, GL_RGBA, GL_FLOAT, 0);

				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			}
		}
	}

	void OGLFrameBuffer::UpdateParams(uint32_t n, TexturePtr texture)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (n < re.DeviceCaps().max_simultaneous_rts)
		{
			if (privateTexs_.size() < n + 1)
			{
				privateTexs_.resize(n + 1);
				faces_.resize(n + 1);

				gbuffers_.resize(n + 1);

				texs_.resize(n + 1);
			}
		}
		else
		{
			THR(E_FAIL);
		}

		privateTexs_[n] = texture;
		texs_[n] = checked_cast<OGLTexture*>(texture.get())->GLTexture();

		if (0 == n)
		{
			width_ = privateTexs_[n]->Width(0);
			height_ = privateTexs_[n]->Height(0);
		}

		viewport_.width		= width_;
		viewport_.height	= height_;

		colorDepth_ = 128;
		isDepthBuffered_ = false;
	}

	void OGLFrameBuffer::UpdateParams(uint32_t n, GraphicsBufferPtr gb, uint32_t width, uint32_t height)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (n < re.DeviceCaps().max_simultaneous_rts)
		{
			if (privateTexs_.size() < n + 1)
			{
				privateTexs_.resize(n + 1);
				faces_.resize(n + 1);

				gbuffers_.resize(n + 1);

				texs_.resize(n + 1);
			}
		}
		else
		{
			THR(E_FAIL);
		}

		gbuffers_[n] = gb;
		if (0 == n)
		{
			width_ = width;
			height_ = height;
		}

		viewport_.width		= width_;
		viewport_.height	= height_;

		colorDepth_ = 128;
		isDepthBuffered_ = false;

		glGenTextures(1, &texs_[n]);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texs_[n]);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width, height,
			0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
}
