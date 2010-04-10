// D3D10FrameBuffer.hpp
// KlayGE D3D10帧缓存类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10FRAMEBUFFER_HPP
#define _D3D10FRAMEBUFFER_HPP

#pragma once

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/D3D10/D3D10Typedefs.hpp>

namespace KlayGE
{
	class D3D10FrameBuffer : public FrameBuffer
	{
	public:
		D3D10FrameBuffer();
		virtual ~D3D10FrameBuffer();

		ID3D10RenderTargetViewPtr D3DRTView(uint32_t n) const;
		ID3D10DepthStencilViewPtr D3DDSView() const;

		virtual std::wstring const & Description() const;

		virtual void OnBind();
		virtual void OnUnbind();

		bool RequiresFlipping() const
		{
			return true;
		}

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil);

	private:
		D3D10_VIEWPORT d3d_viewport_;
	};

	typedef boost::shared_ptr<D3D10FrameBuffer> D3D10FrameBufferPtr;
}

#endif			// _D3D10RENDERTEXTURE_HPP
