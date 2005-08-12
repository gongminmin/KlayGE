// OGLVertexStream.cpp
// KlayGE OpenGL顶点数据流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGL/OGLVertexStream.hpp>

namespace KlayGE
{
	OGLVertexStream::OGLVertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t numElement, bool staticStream)
			: VertexStream(type, sizeElement, numElement),
				static_stream_(staticStream)
	{
		glGenBuffers(1, &vb_);
	}

	OGLVertexStream::~OGLVertexStream()
	{
		glDeleteBuffers(1, &vb_);
	}

	void OGLVertexStream::Assign(void const * src, size_t numVertices)
	{
		size_t const vertexSize(this->SizeOfElement() * this->ElementsPerVertex());
		size_t const size(vertexSize * numVertices);

		numVertices_ = numVertices;

		glBindBuffer(GL_ARRAY_BUFFER, vb_);
		glBufferData(GL_ARRAY_BUFFER,
			reinterpret_cast<GLsizeiptr>(size), src, this->IsStatic() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	}

	void OGLVertexStream::CopyToMemory(void* data)
	{
		size_t const size(this->SizeOfElement() * this->ElementsPerVertex() * this->NumVertices());
		uint8_t* destPtr = static_cast<uint8_t*>(data);

		glBindBuffer(GL_ARRAY_BUFFER, vb_);

		uint8_t* srcPtr = static_cast<uint8_t*>(glMapBuffer(GL_ARRAY_BUFFER,
			GL_READ_ONLY | (this->IsStatic() ? GL_STATIC_READ : GL_DYNAMIC_READ)));

		std::copy(srcPtr, srcPtr + size, destPtr);

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	void OGLVertexStream::Active()
	{
		glBindBuffer(GL_ARRAY_BUFFER, vb_);
	}
}
