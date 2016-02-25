// OGLFrameBuffer.hpp
// KlayGE OpenGL��Ⱦ�������� ͷ�ļ�
// Ver 3.3.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.3.0
// ��ΪFrameBuffer (2006.5.30)
//
// 2.8.0
// ���ν��� (2005.8.1)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLFRAMEBUFFER_HPP
#define _OGLFRAMEBUFFER_HPP

#pragma once

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderView.hpp>

namespace KlayGE
{
	class OGLFrameBuffer : public FrameBuffer
	{
	public:
		explicit OGLFrameBuffer(bool off_screen);
		virtual ~OGLFrameBuffer();

		virtual std::wstring const & Description() const;

		void OnBind();

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil);
		virtual void Discard(uint32_t flags) override;

		GLuint OGLFbo() const
		{
			return fbo_;
		}

	protected:
		GLuint fbo_;
	};

	typedef std::shared_ptr<OGLFrameBuffer> OGLFrameBufferPtr;
}

#endif			// _OGLFRAMEBUFFER_HPP
