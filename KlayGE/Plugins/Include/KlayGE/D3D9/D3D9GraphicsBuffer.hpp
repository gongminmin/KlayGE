// D3D9GraphicsBuffer.hpp
// KlayGE D3D9图形缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9GRAPHICSBUFFER_HPP
#define _D3D9GRAPHICSBUFFER_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>

#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9GraphicsBuffer : public GraphicsBuffer, public D3D9Resource
	{
	public:
		explicit D3D9GraphicsBuffer(BufferUsage usage)
			: GraphicsBuffer(usage)
		{
		}

		virtual ~D3D9GraphicsBuffer()
		{
		}
	};
	typedef boost::shared_ptr<D3D9GraphicsBuffer> D3D9GraphicsBufferPtr;


	class D3D9IndexBuffer : public D3D9GraphicsBuffer
	{
	public:
		explicit D3D9IndexBuffer(BufferUsage usage);

		void* Map(BufferAccess ba);
		void Unmap();

		void Active();
		void Deactive();

		ID3D9IndexBufferPtr D3D9Buffer() const;
		void SwitchFormat(ElementFormat format);

	private:
		void DoResize();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		ElementFormat format_;

		ID3D9DevicePtr d3d_device_;
		ID3D9IndexBufferPtr buffer_;
	};
	typedef boost::shared_ptr<D3D9IndexBuffer> D3D9IndexBufferPtr;


	class D3D9VertexBuffer : public D3D9GraphicsBuffer
	{
	public:
		explicit D3D9VertexBuffer(BufferUsage usage);

		void* Map(BufferAccess ba);
		void Unmap();

		void Active(uint32_t stream, uint32_t stride);
		void Deactive(uint32_t stream);

		ID3D9VertexBufferPtr D3D9Buffer() const;

	private:
		void DoResize();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		ID3D9DevicePtr d3d_device_;
		ID3D9VertexBufferPtr buffer_;
	};
	typedef boost::shared_ptr<D3D9VertexBuffer> D3D9VertexBufferPtr;
}

#endif			// _D3D9GRAPHICSBUFFER_HPP
