// OGLESFrameBuffer.hpp
// KlayGE OpenGL ES渲染到纹理类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESFRAMEBUFFER_HPP
#define _OGLESFRAMEBUFFER_HPP

#pragma once

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderView.hpp>

namespace KlayGE
{
	class OGLESFrameBuffer : public FrameBuffer
	{
	public:
		explicit OGLESFrameBuffer(bool off_screen);
		~OGLESFrameBuffer() override;

		std::wstring const & Description() const override;

		void OnBind() override;
		void OnUnbind() override;

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil) override;
		void Discard(uint32_t flags) override;

		GLuint OGLFbo() const noexcept
		{
			return fbo_;
		}

	protected:
		GLuint fbo_;

		std::vector<GLenum> gl_targets_;
	};

	typedef std::shared_ptr<OGLESFrameBuffer> OGLESFrameBufferPtr;
}

#endif			// _OGLESFRAMEBUFFER_HPP
