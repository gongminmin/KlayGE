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
	class OGLGraphicsBuffer : public GraphicsBuffer
	{
	public:
		explicit OGLGraphicsBuffer(BufferUsage usage, uint32_t access_hint, GLenum target,
			uint32_t size_in_byte);
		~OGLGraphicsBuffer();

		void CopyToBuffer(GraphicsBuffer& rhs);

		virtual void CreateHWResource(void const * init_data) override;
		virtual void DeleteHWResource() override;

		void Active(bool force);

		GLuint GLvbo() const
		{
			return vb_;
		}
		GLenum GLType() const
		{
			return target_;
		}

	private:
		void* Map(BufferAccess ba);
		void Unmap();

	private:
		GLuint vb_;
		GLenum target_;
	};
}

#endif			// _OGLGRAPHICSBUFFERHPP
