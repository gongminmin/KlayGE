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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>

namespace KlayGE
{
	OGLGraphicsBuffer::OGLGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
					uint32_t size_in_byte, ElementFormat fmt)
			: GraphicsBuffer(usage, access_hint, size_in_byte),
				vb_(0), tex_(0), target_(target), fmt_as_shader_res_(fmt)
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

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glCreateBuffers(1, &vb_);
		}
		else
		{
			glGenBuffers(1, &vb_);
		}

		GLbitfield flags = 0;
		if (BU_Dynamic == usage_)
		{
			flags |= GL_DYNAMIC_STORAGE_BIT;
		}
		if (access_hint_ & EAH_CPU_Read)
		{
			flags |= GL_MAP_READ_BIT;
		}
		if (access_hint_ & EAH_CPU_Write)
		{
			flags |= GL_MAP_WRITE_BIT;
		}

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glNamedBufferStorage(vb_, static_cast<GLsizeiptr>(size_in_byte_), data, flags);
		}
		else
		{
			auto& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindBuffer(target_, vb_);

			if (glloader_GL_VERSION_4_4() || glloader_GL_ARB_buffer_storage())
			{
				glBufferStorage(target_, static_cast<GLsizeiptr>(size_in_byte_), data, flags);
			}
			else
			{
				GLenum usage;
				if (BU_Static == usage_)
				{
					if (access_hint_ & EAH_CPU_Read)
					{
						usage = GL_STATIC_READ;
					}
					else
					{
						usage = GL_STATIC_DRAW;
					}
				}
				else
				{
					if (access_hint_ & EAH_CPU_Read)
					{
						usage = GL_DYNAMIC_READ;
					}
					else
					{
						usage = GL_DYNAMIC_DRAW;
					}
				}

				glBufferData(target_, static_cast<GLsizeiptr>(size_in_byte_), data, usage);
			}
		}

		if ((access_hint_ & EAH_GPU_Read) && (fmt_as_shader_res_ != EF_Unknown))
		{
			GLint internal_fmt;
			GLenum gl_fmt;
			GLenum gl_type;
			OGLMapping::MappingFormat(internal_fmt, gl_fmt, gl_type, fmt_as_shader_res_);

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glCreateTextures(GL_TEXTURE_BUFFER, 1, &tex_);
				glTextureBuffer(tex_, internal_fmt, vb_);
			}
			else
			{
				glGenTextures(1, &tex_);
				// TODO: It could affect the texture binding cache in OGLRenderEngine
				glBindTexture(GL_TEXTURE_BUFFER, tex_);
				glTexBuffer(GL_TEXTURE_BUFFER, internal_fmt, vb_);
				glBindTexture(GL_TEXTURE_BUFFER, 0);
			}
		}
	}

	void OGLGraphicsBuffer::DeleteHWResource()
	{
		if (tex_ != 0)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteTextures(1, &tex_);
			}
			else
			{
				glDeleteTextures(1, &tex_);
			}

			tex_ = 0;
		}

		if (vb_ != 0)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
		if (!(re.HackForIntel()) && (ba == BA_Write_Only) && (BU_Dynamic == usage_))
		{
			GLuint access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				p = glMapNamedBufferRange(vb_, 0, static_cast<GLsizeiptr>(size_in_byte_), access);
			}
			else if (glloader_GL_EXT_direct_state_access())
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
				KFL_UNREACHABLE("Not implemented");
				break;
			}

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				p = glMapNamedBuffer(vb_, flag);
			}
			else if (glloader_GL_EXT_direct_state_access())
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
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glUnmapNamedBuffer(vb_);
		}
		else if (glloader_GL_EXT_direct_state_access())
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
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glCopyNamedBufferSubData(vb_, checked_cast<OGLGraphicsBuffer*>(&rhs)->vb_, 0, 0, size_in_byte_);
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			glNamedCopyBufferSubDataEXT(vb_, checked_cast<OGLGraphicsBuffer*>(&rhs)->vb_, 0, 0, size_in_byte_);
		}
		else
		{
			re.BindBuffer(GL_COPY_READ_BUFFER, vb_);
			re.BindBuffer(GL_COPY_WRITE_BUFFER, checked_cast<OGLGraphicsBuffer*>(&rhs)->vb_);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size_in_byte_);
		}
	}

	void OGLGraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const * data)
	{
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glNamedBufferSubData(vb_, offset, size, data);
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			glNamedBufferSubDataEXT(vb_, offset, size, data);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindBuffer(target_, vb_);
			glBufferSubData(target_, offset, size, data);
		}
	}
}
