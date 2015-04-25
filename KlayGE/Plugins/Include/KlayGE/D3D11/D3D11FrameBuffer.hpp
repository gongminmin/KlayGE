// D3D11FrameBuffer.hpp
// KlayGE D3D11帧缓存类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11FRAMEBUFFER_HPP
#define _D3D11FRAMEBUFFER_HPP

#pragma once

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11FrameBuffer : public FrameBuffer
	{
	public:
		D3D11FrameBuffer();
		virtual ~D3D11FrameBuffer();

		ID3D11RenderTargetViewPtr D3DRTView(uint32_t n) const;
		ID3D11DepthStencilViewPtr D3DDSView() const;
		ID3D11UnorderedAccessViewPtr D3DUAView(uint32_t n) const;

		virtual std::wstring const & Description() const;

		virtual void OnBind();
		virtual void OnUnbind();

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil);
		virtual void Discard(uint32_t flags) KLAYGE_OVERRIDE;

	private:
		D3D11_VIEWPORT d3d_viewport_;
	};

	typedef shared_ptr<D3D11FrameBuffer> D3D11FrameBufferPtr;
}

#endif			// _D3D11FRAMEBUFFER_HPP
