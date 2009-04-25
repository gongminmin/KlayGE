// D3D11GraphicsBuffer.hpp
// KlayGE D3D11图形缓冲区类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11GRAPHICSBUFFER_HPP
#define _D3D11GRAPHICSBUFFER_HPP

#pragma once

#include <boost/smart_ptr.hpp>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11GraphicsBuffer : public GraphicsBuffer
	{
	public:
		D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags, ElementInitData* init_data);

		ID3D11BufferPtr const & D3DBuffer() const
		{
			return buffer_;
		}

		void CopyToBuffer(GraphicsBuffer& rhs);

	protected:
		void GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags);

	private:
		void DoResize();

		void* Map(BufferAccess ba);
		void Unmap();

	private:
		ID3D11DevicePtr d3d_device_;
		ID3D11DeviceContextPtr d3d_imm_ctx_;
		ID3D11BufferPtr buffer_;

		uint32_t bind_flags_;
		uint32_t hw_buf_size_;
	};
	typedef boost::shared_ptr<D3D11GraphicsBuffer> D3D11GraphicsBufferPtr;
}

#endif			// _D3D11GRAPHICSBUFFER_HPP
