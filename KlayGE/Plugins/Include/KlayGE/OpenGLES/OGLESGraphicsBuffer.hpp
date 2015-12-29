// OGLESGraphicsBuffer.hpp
// KlayGE OpenGL ES图形缓冲区类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESGRAPHICSBUFFERHPP
#define _OGLESGRAPHICSBUFFERHPP

#pragma once

#include <vector>
#include <glloader/glloader.h>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	class OGLESGraphicsBuffer : public GraphicsBuffer
	{
	public:
		explicit OGLESGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
			uint32_t size_in_byte);
		~OGLESGraphicsBuffer();

		void CopyToBuffer(GraphicsBuffer& rhs);

		virtual void CreateHWResource(void const * init_data) override;
		virtual void DeleteHWResource() override;

		void Active(bool force);

		GLuint GLvbo() const
		{
			return vb_;
		}
		GLuint GLType() const
		{
			return target_;
		}

	private:
		void* Map(BufferAccess ba);
		void Unmap();

	private:
		GLuint vb_;
		GLenum target_;
		BufferAccess last_ba_;
		std::vector<uint8_t> buf_data_;
	};
}

#endif			// _OGLESGRAPHICSBUFFERHPP
