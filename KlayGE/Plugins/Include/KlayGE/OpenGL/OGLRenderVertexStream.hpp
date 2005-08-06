// OGLRenderVertexStream.hpp
// KlayGE OpenGL渲染到顶点流类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERVERTEXSTREAM_HPP
#define _OGLRENDERVERTEXSTREAM_HPP

#include <KlayGE/RenderVertexStream.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")
#endif

namespace KlayGE
{
	class OGLRenderVertexStream : public RenderVertexStream
	{
	public:
		OGLRenderVertexStream(uint32_t width, uint32_t height);

		void Attach(VertexStreamPtr vs);
		void Detach();

		virtual void CustomAttribute(std::string const & name, void* pData);

		bool RequiresTextureFlipping() const
			{ return false; }

	private:
		glBindBufferFUNC glBindBuffer_;
		glBufferDataFUNC glBufferData_;

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

	typedef boost::shared_ptr<OGLRenderVertexStream> OGLRenderVertexStreamPtr;
}

#endif			// _OGLRENDERVERTEXSTREAM_HPP
