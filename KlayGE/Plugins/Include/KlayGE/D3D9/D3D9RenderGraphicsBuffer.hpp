// D3D9RenderGraphicsBuffer.hpp
// KlayGE D3D9渲染图形缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERGRAPHICSBUFFER_HPP
#define _D3D9RENDERGRAPHICSBUFFER_HPP

#include <KlayGE/RenderGraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9RenderGraphicsBuffer : public RenderGraphicsBuffer, public D3D9Resource
	{
		typedef boost::shared_ptr<IDirect3DSurface9> IDirect3DSurface9Ptr;

	public:
		D3D9RenderGraphicsBuffer(uint32_t width, uint32_t height);

		/// MUST call Detach before use the GraphicsBuffer
		void Attach(GraphicsBufferPtr vs);
		void Detach();

		boost::shared_ptr<IDirect3DSurface9> D3DRenderSurface() const;
		boost::shared_ptr<IDirect3DSurface9> D3DRenderZBuffer() const;

		bool RequiresTextureFlipping() const
			{ return false; }

	private:
		IDirect3DSurface9Ptr CreateSurface(D3DPOOL pool);
		void CopyToVertexStream();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		boost::shared_ptr<IDirect3DDevice9> d3d_device_;

		IDirect3DSurface9Ptr render_surf_;
		D3DFORMAT fmt_;
	};

	typedef boost::shared_ptr<D3D9RenderGraphicsBuffer> D3D9RenderGraphicsBufferPtr;
}

#endif			// _D3D9RENDERGRAPHICSBUFFER_HPP
