// D3D10RenderView.hpp
// KlayGE D3D10渲染视图类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10RENDERVIEW_HPP
#define _D3D10RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <boost/noncopyable.hpp>

#include <KlayGE/D3D10/D3D10Typedefs.hpp>

namespace KlayGE
{
	class D3D10Texture1D;
	class D3D10Texture2D;
	class D3D10Texture3D;
	class D3D10TextureCube;
	class D3D10GraphicsBuffer;

	class D3D10RenderView : public RenderView
	{
	public:
		D3D10RenderView();
		virtual ~D3D10RenderView();

	protected:
		ID3D10DevicePtr d3d_device_;
	};
	typedef boost::shared_ptr<D3D10RenderView> D3D10RenderViewPtr;

	class D3D10RenderTargetRenderView : public D3D10RenderView
	{
	public:
		D3D10RenderTargetRenderView(Texture& texture_1d_2d_cube, int array_index, int level);
		D3D10RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D10RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D10RenderTargetRenderView(GraphicsBuffer& gb, uint32_t width, uint32_t height, ElementFormat pf);
		D3D10RenderTargetRenderView(ID3D10RenderTargetViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D10RenderTargetViewPtr const & D3DRenderTargetView() const
		{
			return rt_view_;
		}

	private:
		ID3D10RenderTargetViewPtr rt_view_;
	};
	typedef boost::shared_ptr<D3D10RenderTargetRenderView> D3D10RenderTargetRenderViewPtr;

	class D3D10DepthStencilRenderView : public D3D10RenderView
	{
	public:
		D3D10DepthStencilRenderView(Texture& texture_1d_2d_cube, int array_index, int level);
		D3D10DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D10DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D10DepthStencilRenderView(ID3D10DepthStencilViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);
		D3D10DepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D10DepthStencilViewPtr const & D3DDepthStencilView() const
		{
			return ds_view_;
		}

	private:
		ID3D10DepthStencilViewPtr ds_view_;
	};
	typedef boost::shared_ptr<D3D10DepthStencilRenderView> D3D10DepthStencilRenderViewPtr;
}

#endif			// _D3D10RENDERVIEW_HPP
