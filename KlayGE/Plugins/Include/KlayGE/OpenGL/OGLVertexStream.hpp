// OGLVertexStream.hpp
// KlayGE OpenGL顶点数据流类 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
//
// 2.0.4
// 初次建立 (2004.4.3)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLVERTEXSTREAM_HPP
#define _OGLVERTEXSTREAM_HPP

#include <vector>
#include <glloader/glloader.h>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	class OGLVertexStream : public VertexStream
	{
	public:
		OGLVertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t numElement, bool staticStream);
		~OGLVertexStream();

		bool IsStatic() const
		{
			return static_stream_;
		}
		size_t NumVertices() const
		{
			return buffer_.size() / this->SizeElement() / this->ElementsPerVertex();
		}
		bool UseVBO() const
		{
			return use_vbo_;
		}

		void Assign(void const * src, size_t numVertices, size_t stride);
		void Active();

		std::vector<uint8_t> const & OGLBuffer() const
		{
			return buffer_;
		}
		GLuint OGLvbo() const
		{
			return vb_;
		}

	protected:
		std::vector<uint8_t> buffer_;

		bool use_vbo_;
		bool static_stream_;

		glBindBufferARBFUNC glBindBuffer_;
		glBufferDataARBFUNC glBufferData_;
		glBufferSubDataARBFUNC glBufferSubData_;
		glDeleteBuffersARBFUNC glDeleteBuffers_;
		glGenBuffersARBFUNC glGenBuffers_;
		glGetBufferParameterivARBFUNC glGetBufferParameteriv_;
		glGetBufferPointervARBFUNC glGetBufferPointerv_;
		glGetBufferSubDataARBFUNC glGetBufferSubData_;
		glIsBufferARBFUNC glIsBuffer_;
		glMapBufferARBFUNC glMapBuffer_;
		glUnmapBufferARBFUNC glUnmapBuffer_;

		GLuint vb_;
	};
}

#endif			// _OGLVERTEXSTREAM_HPP
