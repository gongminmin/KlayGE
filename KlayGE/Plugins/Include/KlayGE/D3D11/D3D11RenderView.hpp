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

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11Texture1D;
	class D3D11Texture2D;
	class D3D11Texture3D;
	class D3D11TextureCube;
	class D3D11GraphicsBuffer;

	class D3D11RenderTargetView : public RenderTargetView
	{
	public:
		D3D11RenderTargetView(TexturePtr const & texture_1d_2d_cube, ElementFormat pf, int first_array_index, int array_size, int level);
		D3D11RenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice, uint32_t num_slices,
			int level);
		D3D11RenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face, int level);
		D3D11RenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

		ID3D11RenderTargetView* D3DRenderTargetView() const
		{
			return d3d_rt_view_.get();
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
		void BindDiscardFunc();
		void HWDiscard();
		void FackDiscard();

	private:
		ID3D11Device* d3d_device_;
		ID3D11DeviceContext* d3d_imm_ctx_;
		ID3D11DeviceContext1* d3d_imm_ctx_1_;

		ID3D11RenderTargetViewPtr d3d_rt_view_;
		void* rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;

		std::function<void()> discard_func_;
	};

	class D3D11DepthStencilView : public DepthStencilView
	{
	public:
		D3D11DepthStencilView(TexturePtr const & texture_1d_2d_cube, ElementFormat pf, int first_array_index, int array_size, int level);
		D3D11DepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice, uint32_t num_slices,
			int level);
		D3D11DepthStencilView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face, int level);
		D3D11DepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);

		void ClearDepth(float depth) override;
		void ClearStencil(int32_t stencil) override;
		void ClearDepthStencil(float depth, int32_t stencil) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb) override;
		void OnDetached(FrameBuffer& fb) override;

		ID3D11DepthStencilView* D3DDepthStencilView() const
		{
			return d3d_ds_view_.get();
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
		void BindDiscardFunc();
		void HWDiscard();
		void FackDiscard();

	private:
		ID3D11Device* d3d_device_;
		ID3D11DeviceContext* d3d_imm_ctx_;
		ID3D11DeviceContext1* d3d_imm_ctx_1_;

		ID3D11DepthStencilViewPtr d3d_ds_view_;
		void* rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;

		std::function<void()> discard_func_;
	};


	class D3D11UnorderedAccessView : public UnorderedAccessView
	{
	public:
		D3D11UnorderedAccessView(TexturePtr const & texture_1d_2d_cube, ElementFormat pf, int first_array_index, int array_size, int level);
		D3D11UnorderedAccessView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level);
		D3D11UnorderedAccessView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face, int level);
		D3D11UnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);
		virtual ~D3D11UnorderedAccessView();

		void Clear(float4 const & val);
		void Clear(uint4 const & val);

		void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t index) override;
		void OnDetached(FrameBuffer& fb, uint32_t index) override;

		ID3D11UnorderedAccessView* D3DUnorderedAccessView() const
		{
			return d3d_ua_view_.get();
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
		void BindDiscardFunc();
		void HWDiscard();
		void FackDiscard();

	private:
		ID3D11Device* d3d_device_;
		ID3D11DeviceContext* d3d_imm_ctx_;
		ID3D11DeviceContext1* d3d_imm_ctx_1_;

		ID3D11UnorderedAccessViewPtr d3d_ua_view_;
		void* ua_src_;
		uint32_t ua_first_subres_;
		uint32_t ua_num_subres_;

		std::function<void()> discard_func_;
	};
}

#endif			// _D3D11RENDERVIEW_HPP
