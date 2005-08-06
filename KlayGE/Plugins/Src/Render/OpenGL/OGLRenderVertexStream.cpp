// OGLRenderVertexStream.hpp
// KlayGE OpenGL渲染到顶点流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>
#include <KlayGE/OpenGL/OGLRenderVertexStream.hpp>

namespace KlayGE
{
	OGLRenderVertexStream::OGLRenderVertexStream(uint32_t width, uint32_t height)
	{
		left_ = 0;
		top_ = 0;
		width_ = width;
		height_ = height;

		viewport_.left		= left_;
		viewport_.top		= top_;
		viewport_.width		= width_;
		viewport_.height	= height_;

		if (glloader_GL_EXT_framebuffer_object())
		{
			if (glloader_GL_VERSION_1_5())
			{
				glBindBuffer_ = glBindBuffer;
				glBufferData_ = glBufferData;
			}
			else
			{
				if (glloader_GL_ARB_vertex_buffer_object())
				{
					glBindBuffer_ = glBindBufferARB;
					glBufferData_ = glBufferDataARB;
				}
				else
				{
					THR(E_FAIL);
				}
			}

			if (!glloader_GL_ARB_pixel_buffer_object())
			{
				THR(E_FAIL);
			}

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

			glGenTextures(1, &texture_);
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture_);
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_FLOAT_RGBA_NV, width, height,
				0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glGenFramebuffersEXT_(1, &fbo_);
			glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, fbo_);
			glFramebufferTexture2DEXT_(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, texture_, 0);

			glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, 0);
		}
		else
		{
			THR(E_FAIL);
		}
	}

	void OGLRenderVertexStream::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderVertexStream::Attach(VertexStreamPtr vs)
	{
		BOOST_ASSERT(glIsFramebufferEXT_(fbo_));
		BOOST_ASSERT(dynamic_cast<OGLVertexStream*>(vs.get()) != NULL);

		vs_ = vs;

		OGLVertexStream& ogl_vs = static_cast<OGLVertexStream&>(*vs_);
		glBindBuffer_(GL_PIXEL_PACK_BUFFER_ARB, ogl_vs.OGLvbo());
		glBufferData_(GL_PIXEL_PACK_BUFFER_ARB,
			reinterpret_cast<GLsizeiptr>(width_ * height_ * vs->ElementsPerVertex() * sizeof(GLfloat)), NULL, GL_DYNAMIC_DRAW);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
	}

	void OGLRenderVertexStream::Detach()
	{
		BOOST_ASSERT(glIsFramebufferEXT_(fbo_));

		GLenum format;
		if (3 == vs_->ElementsPerVertex())
		{
			format = GL_RGB;
		}
		else
		{
			BOOST_ASSERT(4 == vs_->ElementsPerVertex());

			format = GL_RGBA;
		}

		GLenum type;
		switch (vs_->Type())
		{
		case VST_Positions:
		case VST_Normals:
		case VST_TextureCoords0:
		case VST_TextureCoords1:
		case VST_TextureCoords2:
		case VST_TextureCoords3:
		case VST_TextureCoords4:
		case VST_TextureCoords5:
		case VST_TextureCoords6:
		case VST_TextureCoords7:
			type = GL_FLOAT;
			{
				std::vector<float> dummy(width_ * height_ * vs_->ElementsPerVertex());
				vs_->Assign(&dummy[0], width_ * height_);
			}
			break;

		case VST_Diffuses:
		case VST_Speculars:
			type = GL_UNSIGNED_BYTE;
			{
				std::vector<uint8_t> dummy(width_ * height_ * vs_->ElementsPerVertex());
				vs_->Assign(&dummy[0], width_ * height_);
			}
			break;

		default:
			BOOST_ASSERT(false);
			type = GL_FLOAT;
			break;
		}

		OGLVertexStream& ogl_vs = static_cast<OGLVertexStream&>(*vs_);
		glBindBuffer_(GL_PIXEL_PACK_BUFFER_ARB, ogl_vs.OGLvbo());
		glReadPixels(0, 0, width_, height_, format, type, 0);

		glBindFramebufferEXT_(GL_FRAMEBUFFER_EXT, 0);
	}
}
