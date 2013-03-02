// D3D11RenderView.hpp
// KlayGE D3D11渲染视图类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
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
	typedef shared_ptr<D3D11RenderView> D3D11RenderViewPtr;

	class D3D11RenderTargetRenderView : public D3D11RenderView
	{
	public:
		D3D11RenderTargetRenderView(Texture& texture_1d_2d_cube, int first_array_index, int array_size, int level);
		D3D11RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D11RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D11RenderTargetRenderView(GraphicsBuffer& gb, uint32_t width, uint32_t height, ElementFormat pf);
		D3D11RenderTargetRenderView(ID3D11RenderTargetViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D11RenderTargetViewPtr const & D3DRenderTargetView() const
		{
			return rt_view_;
		}

		void* RTSrc() const
		{
			return rt_src_;
		}
		uint32_t RTFirstSubRes() const
		{
			return rt_first_subres_;
		}
		uint32_t RTNumSubRes() const
		{
			return rt_num_subres_;
		}

	private:
		ID3D11RenderTargetViewPtr rt_view_;
		void* rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;
	};
	typedef shared_ptr<D3D11RenderTargetRenderView> D3D11RenderTargetRenderViewPtr;

	class D3D11DepthStencilRenderView : public D3D11RenderView
	{
	public:
		D3D11DepthStencilRenderView(Texture& texture_1d_2d_cube, int first_array_index, int array_size, int level);
		D3D11DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D11DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D11DepthStencilRenderView(ID3D11DepthStencilViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);
		D3D11DepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D11DepthStencilViewPtr const & D3DDepthStencilView() const
		{
			return ds_view_;
		}

		void* RTSrc() const
		{
			return rt_src_;
		}
		uint32_t RTFirstSubRes() const
		{
			return rt_first_subres_;
		}
		uint32_t RTNumSubRes() const
		{
			return rt_num_subres_;
		}

	private:
		ID3D11DepthStencilViewPtr ds_view_;
		void* rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;
	};
	typedef shared_ptr<D3D11DepthStencilRenderView> D3D11DepthStencilRenderViewPtr;


	class D3D11UnorderedAccessView : public UnorderedAccessView
	{
	public:
		D3D11UnorderedAccessView(Texture& texture_1d_2d_cube, int first_array_index, int array_size, int level);
		D3D11UnorderedAccessView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D11UnorderedAccessView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D11UnorderedAccessView(GraphicsBuffer& gb, ElementFormat pf);
		D3D11UnorderedAccessView(ID3D11UnorderedAccessViewPtr const & view, uint32_t width, uint32_t height, ElementFormat pf);
		virtual ~D3D11UnorderedAccessView();

		void Clear(float4 const & val);
		void Clear(uint4 const & val);

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		ID3D11UnorderedAccessViewPtr const & D3DUnorderedAccessView() const
		{
			return ua_view_;
		}

		void* UASrc() const
		{
			return ua_src_;
		}
		uint32_t UAFirstSubRes() const
		{
			return ua_first_subres_;
		}
		uint32_t UANumSubRes() const
		{
			return ua_num_subres_;
		}

	private:
		ID3D11DevicePtr d3d_device_;
		ID3D11DeviceContextPtr d3d_imm_ctx_;

		ID3D11UnorderedAccessViewPtr ua_view_;
		void* ua_src_;
		uint32_t ua_first_subres_;
		uint32_t ua_num_subres_;
	};
	typedef shared_ptr<D3D11UnorderedAccessView> D3D11UnorderedAccessViewPtr;
}

#endif			// _D3D11RENDERVIEW_HPP
