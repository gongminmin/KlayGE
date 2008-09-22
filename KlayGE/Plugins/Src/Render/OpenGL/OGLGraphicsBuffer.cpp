// OGLGraphicsBuffer.hpp
// KlayGE OpenGL图形缓冲区类 实现文件
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

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>

namespace KlayGE
{
	OGLGraphicsBuffer::OGLGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target)
			: GraphicsBuffer(usage, access_hint),
				target_(target)
	{
		BOOST_ASSERT((GL_ARRAY_BUFFER == target) || (GL_ELEMENT_ARRAY_BUFFER == target));

		glGenBuffers(1, &vb_);
	}

	OGLGraphicsBuffer::~OGLGraphicsBuffer()
	{
		glDeleteBuffers(1, &vb_);
	}

	void OGLGraphicsBuffer::DoResize()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		glBindBuffer(target_, vb_);
		glBufferData(target_,
				static_cast<GLsizeiptr>(size_in_byte_), NULL,
				(BU_Static == usage_) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	}

	void* OGLGraphicsBuffer::Map(BufferAccess ba)
	{
		glBindBuffer(target_, vb_);

		GLenum flag = 0;
		switch (ba)
		{
		case BA_Read_Only:
			flag = GL_READ_ONLY;
			break;

		case BA_Write_Only:
			flag = GL_WRITE_ONLY;
			break;

		case BA_Read_Write:
			flag = GL_READ_WRITE;
			break;
		}

		return glMapBuffer(target_, flag);
	}

	void OGLGraphicsBuffer::Unmap()
	{
		glBindBuffer(target_, vb_);
		glUnmapBuffer(target_);
	}

	void OGLGraphicsBuffer::Active()
	{
		glBindBuffer(target_, vb_);
	}

	void OGLGraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		GraphicsBuffer::Mapper lhs_mapper(*this, BA_Read_Only);
		GraphicsBuffer::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}
}
