// OGLESGraphicsBuffer.hpp
// KlayGE OpenGL ESͼ�λ������� ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESGRAPHICSBUFFERHPP
#define _OGLESGRAPHICSBUFFERHPP

#pragma once

#include <vector>
#include <glloader/glloader.h>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	class OGLESGraphicsBuffer final : public GraphicsBuffer
	{
	public:
		explicit OGLESGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
			uint32_t size_in_byte, uint32_t structure_byte_stride);
		~OGLESGraphicsBuffer() override;

		void CopyToBuffer(GraphicsBuffer& target) override;
		void CopyToSubBuffer(GraphicsBuffer& target,
			uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

		void CreateHWResource(void const * init_data) override;
		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) override;

		void Active(bool force);

		GLuint GLvbo() const
		{
			return vb_;
		}
		GLuint RetrieveGLTexture(ElementFormat fmt);
		GLuint GLType() const
		{
			return target_;
		}

	private:
		void* Map(BufferAccess ba) override;
		void Unmap() override;

	private:
		GLuint vb_;
		GLuint tex_ = 0;
		GLenum target_;
		BufferAccess last_ba_;
		std::vector<uint8_t> buf_data_;
	};
}

#endif			// _OGLESGRAPHICSBUFFERHPP
