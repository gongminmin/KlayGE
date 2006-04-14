// OGLRenderGraphicsBuffer.hpp
// KlayGE OpenGL渲染图形缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERGRAPHICSBUFFER_HPP
#define _OGLRENDERGRAPHICSBUFFER_HPP

#include <KlayGE/RenderGraphicsBuffer.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderGraphicsBuffer : public RenderGraphicsBuffer
	{
	public:
		OGLRenderGraphicsBuffer(uint32_t width, uint32_t height);

		void Attach(GraphicsBufferPtr buffer);
		void Detach();

		virtual void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const
			{ return false; }

	private:
		glIsRenderbufferEXTFUNC glIsRenderbufferEXT_;
		glBindRenderbufferEXTFUNC glBindRenderbufferEXT_;
		glDeleteRenderbuffersEXTFUNC glDeleteRenderbuffersEXT_;
		glGenRenderbuffersEXTFUNC glGenRenderbuffersEXT_;
		glRenderbufferStorageEXTFUNC glRenderbufferStorageEXT_;
		glGetRenderbufferParameterivEXTFUNC glGetRenderbufferParameterivEXT_;
		glIsFramebufferEXTFUNC glIsFramebufferEXT_;
		glBindFramebufferEXTFUNC glBindFramebufferEXT_;
		glDeleteFramebuffersEXTFUNC glDeleteFramebuffersEXT_;
		glGenFramebuffersEXTFUNC glGenFramebuffersEXT_;
		glCheckFramebufferStatusEXTFUNC glCheckFramebufferStatusEXT_;
		glFramebufferTexture1DEXTFUNC glFramebufferTexture1DEXT_;
		glFramebufferTexture2DEXTFUNC glFramebufferTexture2DEXT_;
		glFramebufferTexture3DEXTFUNC glFramebufferTexture3DEXT_;
		glFramebufferRenderbufferEXTFUNC glFramebufferRenderbufferEXT_;
		glGetFramebufferAttachmentParameterivEXTFUNC glGetFramebufferAttachmentParameterivEXT_;
		glGenerateMipmapEXTFUNC glGenerateMipmapEXT_;

	private:
		GLuint texture_;
		GLuint fbo_;
	};

	typedef boost::shared_ptr<OGLRenderGraphicsBuffer> OGLRenderVertexStreamPtr;
}

#endif			// _OGLRENDERGRAPHICSBUFFER_HPP
