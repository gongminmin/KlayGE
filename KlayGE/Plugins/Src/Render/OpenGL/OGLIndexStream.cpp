// OGLIndexStream.cpp
// KlayGE OpenGL索引数据流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
// 只支持vbo (2005.7.31)
// 只支持OpenGL 1.5及以上 (2005.8.12)
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

#include <KlayGE/OpenGL/OGLIndexStream.hpp>

namespace KlayGE
{
	OGLIndexStream::OGLIndexStream(bool staticStream)
		: static_stream_(staticStream)
	{
		glGenBuffers(1, &ib_);
	}

	OGLIndexStream::~OGLIndexStream()
	{
		glDeleteBuffers(1, &ib_);
	}

	void OGLIndexStream::Assign(void const * src, size_t numIndices)
	{
		numIndices_ = numIndices;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				reinterpret_cast<GLsizeiptr>(numIndices * sizeof(uint16_t)), src,
				this->IsStatic() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	}

	void OGLIndexStream::CopyToMemory(void* data)
	{
		uint16_t* destPtr = static_cast<uint16_t*>(data);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);

		uint16_t* srcPtr = static_cast<uint16_t*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
				GL_READ_ONLY | (this->IsStatic() ? GL_STATIC_READ : GL_DYNAMIC_READ)));

		std::copy(srcPtr, srcPtr + this->NumIndices(), destPtr);

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}

	void OGLIndexStream::Active()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);
	}
}
