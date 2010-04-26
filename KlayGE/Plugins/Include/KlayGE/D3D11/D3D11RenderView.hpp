// D3D11RenderView.hpp
// KlayGE D3D11渲染视图类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERVIEW_HPP
#define _D3D11RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <boost/noncopyable.hpp>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11Texture1D;
	class D3D11Texture2D;
	class D3D11Texture3D;
	class D3D11TextureCube;
	class D3D11GraphicsBuffer;

	class D3D11RenderView : public RenderView
	{
	public:
		D3D11RenderView();
		virtual ~D3D11RenderView();

	protected:
		ID3D11DevicePtr d3d_device_;
		ID3D11DeviceContextPtr d3d_imm_ctx_;
	};
	typedef boost::shared_ptr<D3D11RenderView> D3D11RenderViewPtr;

	class D3D11RenderTargetRenderView : public D3D11RenderView
	{
	public:
		D3D11RenderTargetRenderView(Texture& texture_1d_2d_cube, int array_index, int level);
		D3D11RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D11RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D11RenderTargetRenderView(GraphicsBuffer& gb, uint32_t width, uint32_t height, ElementFormat pf);
		D3D11RenderTargetRenderView(ID3D11RenderTargetViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);

		void Clear(Color const & clr);
		void Clear(float depth);
		void Clear(int32_t stencil);
		void Clear(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D11RenderTargetViewPtr const & D3DRenderTargetView() const
		{
			return rt_view_;
		}

	private:
		ID3D11RenderTargetViewPtr rt_view_;
	};
	typedef boost::shared_ptr<D3D11RenderTargetRenderView> D3D11RenderTargetRenderViewPtr;

	class D3D11DepthStencilRenderView : public D3D11RenderView
	{
	public:
		D3D11DepthStencilRenderView(Texture& texture_1d_2d_cube, int array_index, int level);
		D3D11DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D11DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D11DepthStencilRenderView(ID3D11DepthStencilViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);
		D3D11DepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);

		void Clear(Color const & clr);
		void Clear(float depth);
		void Clear(int32_t stencil);
		void Clear(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D11DepthStencilViewPtr const & D3DDepthStencilView() const
		{
			return ds_view_;
		}

	private:
		ID3D11DepthStencilViewPtr ds_view_;
	};
	typedef boost::shared_ptr<D3D11DepthStencilRenderView> D3D11DepthStencilRenderViewPtr;
}

#endif			// _D3D11RENDERVIEW_HPP
