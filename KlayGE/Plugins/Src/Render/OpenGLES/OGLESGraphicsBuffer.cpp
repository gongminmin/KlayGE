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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <algorithm>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>

namespace KlayGE
{
	OGLESGraphicsBuffer::OGLESGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target, ElementInitData const * init_data)
			: GraphicsBuffer(usage, access_hint),
				target_(target)
	{
		BOOST_ASSERT((GL_ARRAY_BUFFER == target) || (GL_ELEMENT_ARRAY_BUFFER == target));

		glGenBuffers(1, &vb_);

		if (init_data != nullptr)
		{
			size_in_byte_ = init_data->row_pitch;

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindBuffer(target_, vb_);
			glBufferData(target_,
					static_cast<GLsizeiptr>(size_in_byte_), init_data->data,
					(BU_Static == usage_) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
			buf_data_.assign(static_cast<uint8_t const *>(init_data->data),
				static_cast<uint8_t const *>(init_data->data) + size_in_byte_);
		}
	}

	OGLESGraphicsBuffer::~OGLESGraphicsBuffer()
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
	}

	void OGLESGraphicsBuffer::DoResize()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(target_, vb_);
		glBufferData(target_,
				static_cast<GLsizeiptr>(size_in_byte_), nullptr,
				(BU_Static == usage_) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		buf_data_.resize(size_in_byte_);
	}

	void* OGLESGraphicsBuffer::Map(BufferAccess ba)
	{
		last_ba_ = ba;

		switch (ba)
		{
		case BA_Write_Only:
			// TODO: fix OES_mapbuffer
			/*if (glloader_GLES_OES_mapbuffer())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindBuffer(target_, vb_);
				return glMapBufferOES(target_, GL_WRITE_ONLY_OES);
			}
			else*/
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
				// TODO: fix OES_mapbuffer
				/*if (glloader_GLES_OES_mapbuffer())
				{
					glUnmapBufferOES(target_);
				}
				else*/
				{
					glBufferSubData(target_, 0, static_cast<GLsizeiptr>(size_in_byte_), &buf_data_[0]);
				}
			}
			break;
			
		default:
			break;
		}
	}

	void OGLESGraphicsBuffer::Active()
	{
		// TODO: fix me
		glBindBuffer(target_, vb_);
		//OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		//re.BindBuffer(target_, vb_);
	}

	void OGLESGraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		GraphicsBuffer::Mapper lhs_mapper(*this, BA_Read_Only);
		GraphicsBuffer::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}
}
