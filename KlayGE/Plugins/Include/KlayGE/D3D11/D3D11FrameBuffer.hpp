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

		ID3D11RenderTargetView* D3DRTView(uint32_t n) const;
		ID3D11DepthStencilView* D3DDSView() const;
		ID3D11UnorderedAccessView* D3DUAView(uint32_t n) const;

		std::wstring const & Description() const override;

		void OnBind() override;
		void OnUnbind() override;

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil) override;
		virtual void Discard(uint32_t flags) override;

	private:
		std::vector<void*> d3d_rt_src_;
		std::vector<uint32_t> d3d_rt_first_subres_;
		std::vector<uint32_t> d3d_rt_num_subres_;

		std::vector<ID3D11RenderTargetView*> d3d_rt_view_;
		ID3D11DepthStencilView* d3d_ds_view_;
		std::vector<ID3D11UnorderedAccessView*> d3d_ua_view_;
		std::vector<UINT> d3d_ua_init_count_;

		D3D11_VIEWPORT d3d_viewport_;
	};
}

#endif			// _D3D11FRAMEBUFFER_HPP
