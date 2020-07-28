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

	class D3D11ShaderResourceView : public ShaderResourceView
	{
	public:
		virtual ID3D11ShaderResourceView* RetrieveD3DShaderResourceView() const = 0;

	protected:
		ID3D11Device1* d3d_device_;
		ID3D11DeviceContext1* d3d_imm_ctx_;

		mutable ID3D11ShaderResourceViewPtr d3d_sr_view_;
		void* sr_src_;
	};

	class D3D11TextureShaderResourceView final : public D3D11ShaderResourceView
	{
	public:
		D3D11TextureShaderResourceView(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			uint32_t first_level, uint32_t num_levels);

		ID3D11ShaderResourceView* RetrieveD3DShaderResourceView() const override;
	};

	class D3D11CubeTextureFaceShaderResourceView final : public D3D11ShaderResourceView
	{
	public:
		D3D11CubeTextureFaceShaderResourceView(TexturePtr const& texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			uint32_t first_level, uint32_t num_levels);

		ID3D11ShaderResourceView* RetrieveD3DShaderResourceView() const override;
	};

	class D3D11BufferShaderResourceView final : public D3D11ShaderResourceView
	{
	public:
		D3D11BufferShaderResourceView(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

		ID3D11ShaderResourceView* RetrieveD3DShaderResourceView() const override;
	};

	class D3D11RenderTargetView : public RenderTargetView
	{
	public:
		D3D11RenderTargetView(void* src, uint32_t first_subres, uint32_t num_subres);

		void ClearColor(Color const & clr) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
		void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;

		virtual ID3D11RenderTargetView* RetrieveD3DRenderTargetView() const = 0;

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

	protected:
		ID3D11Device1* d3d_device_;
		ID3D11DeviceContext1* d3d_imm_ctx_;

		mutable ID3D11RenderTargetViewPtr d3d_rt_view_;
		void* rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;
	};

	class D3D11Texture1D2DCubeRenderTargetView final : public D3D11RenderTargetView
	{
	public:
		D3D11Texture1D2DCubeRenderTargetView(TexturePtr const & texture_1d_2d_cube, ElementFormat pf, int first_array_index, int array_size,
			int level);

		ID3D11RenderTargetView* RetrieveD3DRenderTargetView() const override;
	};

	class D3D11Texture3DRenderTargetView final : public D3D11RenderTargetView
	{
	public:
		D3D11Texture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level);

		ID3D11RenderTargetView* RetrieveD3DRenderTargetView() const override;
	};

	class D3D11TextureCubeFaceRenderTargetView final : public D3D11RenderTargetView
	{
	public:
		D3D11TextureCubeFaceRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);

		ID3D11RenderTargetView* RetrieveD3DRenderTargetView() const override;
	};

	class D3D11BufferRenderTargetView final : public D3D11RenderTargetView
	{
	public:
		D3D11BufferRenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

		ID3D11RenderTargetView* RetrieveD3DRenderTargetView() const override;
	};


	class D3D11DepthStencilView : public DepthStencilView
	{
	public:
		D3D11DepthStencilView(void* src, uint32_t first_subres, uint32_t num_subres);

		void ClearDepth(float depth) override;
		void ClearStencil(int32_t stencil) override;
		void ClearDepthStencil(float depth, int32_t stencil) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb) override;
		void OnDetached(FrameBuffer& fb) override;

		virtual ID3D11DepthStencilView* RetrieveD3DDepthStencilView() const = 0;

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

	protected:
		ID3D11Device1* d3d_device_;
		ID3D11DeviceContext1* d3d_imm_ctx_;

		mutable ID3D11DepthStencilViewPtr d3d_ds_view_;
		void* rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;
	};
	
	class D3D11Texture1D2DCubeDepthStencilView final : public D3D11DepthStencilView
	{
	public:
		D3D11Texture1D2DCubeDepthStencilView(TexturePtr const & texture_1d_2d_cube, ElementFormat pf, int first_array_index, int array_size,
			int level);
		D3D11Texture1D2DCubeDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
			uint32_t sample_quality);

		ID3D11DepthStencilView* RetrieveD3DDepthStencilView() const override;
	};

	class D3D11Texture3DDepthStencilView final : public D3D11DepthStencilView
	{
	public:
		D3D11Texture3DDepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level);

		ID3D11DepthStencilView* RetrieveD3DDepthStencilView() const override;
	};

	class D3D11TextureCubeFaceDepthStencilView final : public D3D11DepthStencilView
	{
	public:
		D3D11TextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);

		ID3D11DepthStencilView* RetrieveD3DDepthStencilView() const override;
	};


	class D3D11UnorderedAccessView : public UnorderedAccessView
	{
	public:
		D3D11UnorderedAccessView(void* src, uint32_t first_subres, uint32_t num_subres);

		void Clear(float4 const & val) override;
		void Clear(uint4 const & val) override;

		void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t index) override;
		void OnDetached(FrameBuffer& fb, uint32_t index) override;

		virtual ID3D11UnorderedAccessView* RetrieveD3DUnorderedAccessView() const = 0;

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

	protected:
		ID3D11Device1* d3d_device_;
		ID3D11DeviceContext1* d3d_imm_ctx_;

		mutable ID3D11UnorderedAccessViewPtr d3d_ua_view_;
		void* ua_src_;
		uint32_t ua_first_subres_;
		uint32_t ua_num_subres_;
	};

	class D3D11Texture1D2DCubeUnorderedAccessView final : public D3D11UnorderedAccessView
	{
	public:
		D3D11Texture1D2DCubeUnorderedAccessView(TexturePtr const & texture_1d_2d_cube, ElementFormat pf, int first_array_index,
			int array_size, int level);

		ID3D11UnorderedAccessView* RetrieveD3DUnorderedAccessView() const override;
	};

	class D3D11Texture3DUnorderedAccessView final : public D3D11UnorderedAccessView
	{
	public:
		D3D11Texture3DUnorderedAccessView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level);

		ID3D11UnorderedAccessView* RetrieveD3DUnorderedAccessView() const override;
	};

	class D3D11TextureCubeFaceUnorderedAccessView final : public D3D11UnorderedAccessView
	{
	public:
		D3D11TextureCubeFaceUnorderedAccessView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);

		ID3D11UnorderedAccessView* RetrieveD3DUnorderedAccessView() const override;
	};

	class D3D11BufferUnorderedAccessView final : public D3D11UnorderedAccessView
	{
	public:
		D3D11BufferUnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

		ID3D11UnorderedAccessView* RetrieveD3DUnorderedAccessView() const override;
	};
}

#endif			// _D3D11RENDERVIEW_HPP
