// OGLGraphicsBuffer.hpp
// KlayGE OpenGLͼ�λ������� ͷ�ļ�
// Ver 3.2.0
// ��Ȩ����(C) ������, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.2.0
// ��OGLIndexStream��OGLVertexStream�ϲ���OGLGraphicsBuffer (2006.1.10)
//
// 2.8.0
// ������CopyToMemory (2005.7.24)
// ֻ֧��vbo (2005.7.31)
//
// 2.7.0
// ֧��vertex_buffer_object (2005.6.19)
//
// 2.0.4
// ���ν��� (2004.4.3)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLGRAPHICSBUFFERHPP
#define _OGLGRAPHICSBUFFERHPP

#pragma once

#include <vector>
#include <glloader/glloader.h>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	class OGLGraphicsBuffer final : public GraphicsBuffer
	{
	public:
		explicit OGLGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
			uint32_t size_in_byte, uint32_t structure_byte_stride);
		~OGLGraphicsBuffer() override;

		void CopyToBuffer(GraphicsBuffer& target) override;
		void CopyToSubBuffer(GraphicsBuffer& target,
			uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

		void CreateHWResource(void const * init_data) override;
		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) override;

		void Active(bool force);

		GLuint GLvbo() const noexcept
		{
			return vb_;
		}
		GLuint RetrieveGLTexture(ElementFormat fmt);
		GLenum GLType() const noexcept
		{
			return target_;
		}

	private:
		void* Map(BufferAccess ba) override;
		void Unmap() override;

	private:
		GLuint vb_;
		GLuint tex_{0};
		GLenum target_;
	};
}

#endif			// _OGLGRAPHICSBUFFERHPP
