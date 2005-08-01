// OGLRenderTexture.cpp
// KlayGE OpenGL渲染到纹理类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/RenderTexture.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderTexture.hpp>

namespace KlayGE
{
	OGLRenderTexture::OGLRenderTexture()
		: fbo_(-1)
	{
		left_ = 0;
		top_ = 0;

		if (glloader_GL_EXT_framebuffer_object())
		{
			glIsRenderbufferEXT_ = glIsRenderbufferEXT;
			glBindRenderbufferEXT_ = glBindRenderbufferEXT;
			glDeleteRenderbuffersEXT_ = glDeleteRenderbuffersEXT;
			glGenRenderbuffersEXT_ = glGenRenderbuffersEXT;
			glRenderbufferStorageEXT_ = glRenderbufferStorageEXT;
			glGetRenderbufferParameterivEXT_ = glGetRenderbufferParameterivEXT;
			glIsFramebufferEXT_ = glIsFramebufferEXT;
			glBindFramebufferEXT_ = glBindFramebufferEXT;
			glDeleteFramebuffersEXT_ = glDeleteFramebuffersEXT;
			glGenFramebuffersEXT_ = glGenFramebuffersEXT;
			glCheckFramebufferStatusEXT_ = glCheckFramebufferStatusEXT;
			glFramebufferTexture1DEXT_ = glFramebufferTexture1DEXT;
			glFramebufferTexture2DEXT_ = glFramebufferTexture2DEXT;
			glFramebufferTexture3DEXT_ = glFramebufferTexture3DEXT;
			glFramebufferRenderbufferEXT_ = glFramebufferRenderbufferEXT;
			glGetFramebufferAttachmentParameterivEXT_ = glGetFramebufferAttachmentParameterivEXT;
			glGenerateMipmapEXT_ = glGenerateMipmapEXT;

			glGenFramebuffersEXT_(1, &fbo_);
		}
		else
		{
			THR(E_FAIL);
		}
	}

	void OGLRenderTexture::CustomAttribute(std::string const & name, void* pData)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderTexture::AttachTexture2D(TexturePtr texture2D)
	{
		BOOST_ASSERT(glIsFramebufferEXT_(fbo_));
		BOOST_ASSERT(Texture::TT_2D == texture2D->Type());
		BOOST_ASSERT(dynamic_cast<OGLTexture*>(texture2D.get()) != NULL);

		OGLTexture& ogl_tex = static_cast<OGLTexture&>(*texture2D);

		glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, fbo_);
		glFramebufferTexture2DEXT_(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ogl_tex.GLTexture(), 0);
	}
	
	void OGLRenderTexture::AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face)
	{
		BOOST_ASSERT(glIsFramebufferEXT_(fbo_));
		BOOST_ASSERT(Texture::TT_Cube == textureCube->Type());
		BOOST_ASSERT(dynamic_cast<OGLTexture*>(textureCube.get()) != NULL);

		OGLTexture& ogl_tex = static_cast<OGLTexture&>(*textureCube);

		glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, fbo_);
		glFramebufferTexture2DEXT_(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ogl_tex.GLTextureFace(face - Texture::CF_Positive_X), 0);
	}

	void OGLRenderTexture::DetachTexture()
	{
		BOOST_ASSERT(glIsFramebufferEXT(fbo_));

		glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, 0);
	}
}
