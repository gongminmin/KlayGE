// OGLIndexStream.cpp
// KlayGE OpenGL索引数据流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
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
		: use_vbo_(false), static_stream_(staticStream)
	{
		if (glloader_GL_VERSION_1_5())
		{
			glBindBuffer_			= glBindBuffer;
			glBufferData_			= glBufferData;
			glBufferSubData_		= glBufferSubData;
			glDeleteBuffers_		= glDeleteBuffers;
			glGenBuffers_			= glGenBuffers;
			glGetBufferParameteriv_	= glGetBufferParameteriv;
			glGetBufferPointerv_	= glGetBufferPointerv;
			glGetBufferSubData_		= glGetBufferSubData;
			glIsBuffer_				= glIsBuffer;
			glMapBuffer_			= glMapBuffer;
			glUnmapBuffer_			= glUnmapBuffer;

			use_vbo_ = true;
		}
		else
		{
			if (glloader_GL_ARB_vertex_buffer_object())
			{
				glBindBuffer_			= glBindBufferARB;
				glBufferData_			= glBufferDataARB;
				glBufferSubData_		= glBufferSubDataARB;
				glDeleteBuffers_		= glDeleteBuffersARB;
				glGenBuffers_			= glGenBuffersARB;
				glGetBufferParameteriv_	= glGetBufferParameterivARB;
				glGetBufferPointerv_	= glGetBufferPointervARB;
				glGetBufferSubData_		= glGetBufferSubDataARB;
				glIsBuffer_				= glIsBufferARB;
				glMapBuffer_			= glMapBufferARB;
				glUnmapBuffer_			= glUnmapBufferARB;

				use_vbo_ = true;
			}
		}

		if (use_vbo_)
		{
			glGenBuffers_(1, &ib_);
		}
	}

	OGLIndexStream::~OGLIndexStream()
	{
		if (use_vbo_)
		{
			glDeleteBuffers_(1, &ib_);
		}
	}

	void OGLIndexStream::Assign(void const * src, size_t numIndices)
	{
		numIndices_ = numIndices;

		if (use_vbo_)
		{
			glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ib_);
			glBufferData_(GL_ELEMENT_ARRAY_BUFFER,
				reinterpret_cast<GLsizeiptr>(numIndices * sizeof(uint16_t)), src,
				this->IsStatic() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		}
		else
		{
			buffer_.assign(static_cast<uint16_t const *>(src),
				static_cast<uint16_t const *>(src) + numIndices);
		}
	}

	void OGLIndexStream::CopyToMemory(void* data)
	{
		uint16_t* destPtr = static_cast<uint16_t*>(data);

		if (use_vbo_)
		{
			glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ib_);

			uint16_t* srcPtr = static_cast<uint16_t*>(glMapBuffer_(GL_ELEMENT_ARRAY_BUFFER,
				this->IsStatic() ? GL_STATIC_READ : GL_DYNAMIC_READ));

			std::copy(srcPtr, srcPtr + this->NumIndices(), destPtr);

			glUnmapBuffer(GL_ARRAY_BUFFER);
		}
		else
		{
			std::copy(buffer_.begin(), buffer_.end(), destPtr);
		}
	}

	void OGLIndexStream::Active()
	{
		if (use_vbo_)
		{
			glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ib_);
		}
	}
}
