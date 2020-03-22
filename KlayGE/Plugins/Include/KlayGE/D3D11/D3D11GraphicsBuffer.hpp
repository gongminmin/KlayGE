// D3D11GraphicsBuffer.hpp
// KlayGE D3D11图形缓冲区类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11GRAPHICSBUFFER_HPP
#define _D3D11GRAPHICSBUFFER_HPP

#pragma once

#include <unordered_map>

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11GraphicsBuffer final : public GraphicsBuffer
	{
	public:
		D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags,
			uint32_t size_in_byte, uint32_t structure_byte_stride);

		ID3D11Buffer* D3DBuffer() const
		{
			return d3d_buffer_.get();
		}

		ID3D11ShaderResourceViewPtr const & RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems);
		ID3D11RenderTargetViewPtr const & RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems);
		ID3D11UnorderedAccessViewPtr const & RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

		void CopyToBuffer(GraphicsBuffer& target) override;
		void CopyToSubBuffer(GraphicsBuffer& target,
			uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

		void CreateHWResource(void const * init_data) override;
		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) override;

	private:
		void GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags);

		void* Map(BufferAccess ba) override;
		void Unmap() override;

	private:
		ID3D11Device1* d3d_device_;
		ID3D11DeviceContext1* d3d_imm_ctx_;

		ID3D11BufferPtr d3d_buffer_;

		uint32_t bind_flags_;

		// TODO: Not caching those views
		std::unordered_map<size_t, ID3D11ShaderResourceViewPtr> d3d_sr_views_;
		std::unordered_map<size_t, ID3D11RenderTargetViewPtr> d3d_rt_views_;
		std::unordered_map<size_t, ID3D11UnorderedAccessViewPtr> d3d_ua_views_;
	};
}

#endif			// _D3D11GRAPHICSBUFFER_HPP
