// D3D10GraphicsBuffer.hpp
// KlayGE D3D10图形缓冲区类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10GRAPHICSBUFFER_HPP
#define _D3D10GRAPHICSBUFFER_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D10
#include <KlayGE/config/auto_link.hpp>

#include <boost/smart_ptr.hpp>

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D10/D3D10Typedefs.hpp>

namespace KlayGE
{
	class D3D10GraphicsBuffer : public GraphicsBuffer
	{
	public:
		D3D10GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags, ElementInitData* init_data);

		ID3D10BufferPtr const & D3DBuffer() const
		{
			return buffer_;
		}

		void CopyToBuffer(GraphicsBuffer& rhs);

	protected:
		void GetD3DFlags(D3D10_USAGE& usage, UINT& cpu_access_flags);

	private:
		void DoResize();

		void* Map(BufferAccess ba);
		void Unmap();

	private:
		ID3D10DevicePtr d3d_device_;
		ID3D10BufferPtr buffer_;

		uint32_t bind_flags_;
		uint32_t hw_buf_size_;
	};
	typedef boost::shared_ptr<D3D10GraphicsBuffer> D3D10GraphicsBufferPtr;
}

#endif			// _D3D10GRAPHICSBUFFER_HPP
