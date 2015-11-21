// OGLESGraphicsBuffer.cpp
// KlayGE OpenGL ES 2图形缓冲区类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
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

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>

namespace KlayGE
{
	OGLESGraphicsBuffer::OGLESGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
				uint32_t size_in_byte)
			: GraphicsBuffer(usage, access_hint, size_in_byte),
				vb_(0), target_(target)
	{
		BOOST_ASSERT((GL_ARRAY_BUFFER == target) || (GL_ELEMENT_ARRAY_BUFFER == target)
			|| (GL_UNIFORM_BUFFER == target));
	}

	OGLESGraphicsBuffer::~OGLESGraphicsBuffer()
	{
		this->DeleteHWResource();
	}

	void OGLESGraphicsBuffer::CreateHWResource(void const * data)
	{
		BOOST_ASSERT(0 == vb_);

		glGenBuffers(1, &vb_);

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(target_, vb_);
		glBufferData(target_,
			static_cast<GLsizeiptr>(size_in_byte_), data,
			(BU_Static == usage_) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		if (data != nullptr)
		{
			buf_data_.assign(static_cast<uint8_t const *>(data),
				static_cast<uint8_t const *>(data) + size_in_byte_);
		}
		else
		{
			buf_data_.resize(size_in_byte_);
		}
	}

	void OGLESGraphicsBuffer::DeleteHWResource()
	{
		if (vb_ != 0)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteBuffers(1, &vb_);
			}
			else
			{
				glDeleteBuffers(1, &vb_);
			}

			vb_ = 0;
		}
	}

	void* OGLESGraphicsBuffer::Map(BufferAccess ba)
	{
		last_ba_ = ba;

		switch (ba)
		{
		case BA_Write_Only:
			if (glloader_GLES_VERSION_3_0())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindBuffer(target_, vb_);
				uint32_t flags = GL_MAP_WRITE_BIT;
				if (BU_Dynamic == usage_)
				{
					flags |= GL_MAP_INVALIDATE_BUFFER_BIT;
				}
				return glMapBufferRange(target_, 0, static_cast<GLsizeiptr>(size_in_byte_), flags);
			}
			else if (glloader_GLES_OES_mapbuffer())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindBuffer(target_, vb_);
				return glMapBufferOES(target_, GL_WRITE_ONLY_OES);
			}
			else
			{
				return &buf_data_[0];
			}

		default:
			return &buf_data_[0];
		}
	}

	void OGLESGraphicsBuffer::Unmap()
	{
		switch (last_ba_)
		{
		case BA_Write_Only:
		case BA_Read_Write:
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindBuffer(target_, vb_);
				if (glloader_GLES_VERSION_3_0())
				{
					glUnmapBuffer(target_);
				}
				else if (glloader_GLES_OES_mapbuffer())
				{
					glUnmapBufferOES(target_);
				}
				else
				{
					glBufferSubData(target_, 0, static_cast<GLsizeiptr>(size_in_byte_), &buf_data_[0]);
				}
			}
			break;
			
		default:
			break;
		}
	}

	void OGLESGraphicsBuffer::Active(bool force)
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(target_, vb_, force);
	}

	void OGLESGraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		GraphicsBuffer::Mapper lhs_mapper(*this, BA_Read_Only);
		GraphicsBuffer::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}
}
