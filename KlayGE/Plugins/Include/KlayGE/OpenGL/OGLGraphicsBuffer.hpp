// OGLGraphicsBuffer.hpp
// KlayGE OpenGL图形缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 把OGLIndexStream和OGLVertexStream合并成OGLGraphicsBuffer (2006.1.10)
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
// 只支持vbo (2005.7.31)
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
//
// 2.0.4
// 初次建立 (2004.4.3)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLGRAPHICSBUFFERHPP
#define _OGLGRAPHICSBUFFERHPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_OpenGL
#include <KlayGE/config/auto_link.hpp>

#include <vector>
#include <glloader/glloader.h>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	class OGLGraphicsBuffer : public GraphicsBuffer
	{
	public:
		explicit OGLGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target);
		~OGLGraphicsBuffer();

		void CopyToBuffer(GraphicsBuffer& rhs);

		void Active();

		GLuint OGLvbo() const
		{
			return vb_;
		}

	private:
		void DoResize();

		void* Map(BufferAccess ba);
		void Unmap();

	private:
		GLuint vb_;
		GLenum target_;
	};
}

#endif			// _OGLGRAPHICSBUFFERHPP
