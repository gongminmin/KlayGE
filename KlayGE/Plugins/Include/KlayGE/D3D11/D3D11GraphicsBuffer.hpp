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

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11GraphicsBuffer : public GraphicsBuffer
	{
	public:
		D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags,
			uint32_t size_in_byte, ElementFormat fmt);

		ID3D11Buffer* D3DBuffer() const
		{
			return buffer_.get();
		}

		ID3D11ShaderResourceViewPtr const & D3DShaderResourceView() const
		{
			return d3d_sr_view_;
		}
		ID3D11RenderTargetViewPtr const & D3DRenderTargetView() const;
		ID3D11UnorderedAccessViewPtr const & D3DUnorderedAccessView() const
		{
			return d3d_ua_view_;
		}

		void CopyToBuffer(GraphicsBuffer& rhs);

		virtual void CreateHWResource(void const * init_data) override;
		virtual void DeleteHWResource() override;

	protected:
		void GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags);

	private:
		void* Map(BufferAccess ba);
		void Unmap();

	private:
		ID3D11Device* d3d_device_;
		ID3D11DeviceContext* d3d_imm_ctx_;
		ID3D11BufferPtr buffer_;
		ID3D11ShaderResourceViewPtr d3d_sr_view_;
		mutable ID3D11RenderTargetViewPtr d3d_rt_view_;
		ID3D11UnorderedAccessViewPtr d3d_ua_view_;

		uint32_t bind_flags_;
		ElementFormat fmt_as_shader_res_;
	};
}

#endif			// _D3D11GRAPHICSBUFFER_HPP
