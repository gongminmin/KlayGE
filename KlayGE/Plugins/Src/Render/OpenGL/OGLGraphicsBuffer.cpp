// OGLGraphicsBuffer.cpp
// KlayGE OpenGL图形缓冲区类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持GL_ARB_copy_buffer (2009.8.5)
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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>

namespace KlayGE
{
	OGLGraphicsBuffer::OGLGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
					uint32_t size_in_byte)
			: GraphicsBuffer(usage, access_hint, size_in_byte),
				vb_(0), target_(target)
	{
		BOOST_ASSERT((GL_ARRAY_BUFFER == target) || (GL_ELEMENT_ARRAY_BUFFER == target)
			|| (GL_UNIFORM_BUFFER == target));
	}

	OGLGraphicsBuffer::~OGLGraphicsBuffer()
	{
		this->DeleteHWResource();
	}

	void OGLGraphicsBuffer::CreateHWResource(void const * data)
	{
		BOOST_ASSERT(0 == vb_);

		glGenBuffers(1, &vb_);

		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedBufferDataEXT(vb_, static_cast<GLsizeiptr>(size_in_byte_), data,
				(BU_Static == usage_) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindBuffer(target_, vb_);
			glBufferData(target_,
				static_cast<GLsizeiptr>(size_in_byte_), data,
				(BU_Static == usage_) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		}
	}

	void OGLGraphicsBuffer::DeleteHWResource()
	{
		if (vb_ != 0)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteBuffers(1, &vb_);
			}
			else
			{
				glDeleteBuffers(1, &vb_);
			}

			vb_ = 0;
		}
	}

	void* OGLGraphicsBuffer::Map(BufferAccess ba)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		void* p;
		if (!(re.HackForIntel()) && ((glloader_GL_VERSION_3_0() || glloader_GL_ARB_map_buffer_range())
			&& (ba == BA_Write_Only) && (BU_Dynamic == usage_)))
		{
			GLuint access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
			if (glloader_GL_EXT_direct_state_access())
			{
				p = glMapNamedBufferRangeEXT(vb_, 0, static_cast<GLsizeiptr>(size_in_byte_), access);
			}
			else
			{
				re.BindBuffer(target_, vb_);
				p = glMapBufferRange(target_, 0, static_cast<GLsizeiptr>(size_in_byte_), access);
			}
		}
		else
		{
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

			case BA_Write_No_Overwrite:
				// TODO
				BOOST_ASSERT(false);
				break;
			}

			if (glloader_GL_EXT_direct_state_access())
			{
				p = glMapNamedBufferEXT(vb_, flag);
			}
			else
			{
				re.BindBuffer(target_, vb_);
				p = glMapBuffer(target_, flag);
			}
		}
		return p;
	}

	void OGLGraphicsBuffer::Unmap()
	{
		if (glloader_GL_EXT_direct_state_access())
		{
			glUnmapNamedBufferEXT(vb_);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindBuffer(target_, vb_);
			glUnmapBuffer(target_);
		}
	}

	void OGLGraphicsBuffer::Active(bool force)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(target_, vb_, force);
	}

	void OGLGraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		if (glloader_GL_VERSION_3_1() || glloader_GL_ARB_copy_buffer())
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindBuffer(GL_COPY_READ_BUFFER, vb_);
			re.BindBuffer(GL_COPY_WRITE_BUFFER, checked_cast<OGLGraphicsBuffer*>(&rhs)->vb_);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
						  0, 0, size_in_byte_);
		}
		else
		{
			GraphicsBuffer::Mapper lhs_mapper(*this, BA_Read_Only);
			GraphicsBuffer::Mapper rhs_mapper(rhs, BA_Write_Only);
			std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
				rhs_mapper.Pointer<uint8_t>());
		}
	}
}
