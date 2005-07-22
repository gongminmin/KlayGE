// D3D9VertexStream.hpp
// KlayGE D3D9顶点流类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 改为派生自D3D9Resource (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9VERTEXSTREAM_HPP
#define _D3D9VERTEXSTREAM_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>

#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9VertexStream : public VertexStream, public D3D9Resource
	{
	public:
		D3D9VertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t ElementsPerVertex, bool staticStream);

		bool IsStatic() const;

		void Assign(void const * src, size_t numVertices, size_t stride = 0);

		boost::shared_ptr<IDirect3DVertexBuffer9> D3D9Buffer() const;
		size_t NumVertices() const;

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		boost::shared_ptr<IDirect3DDevice9> d3d_device_;

		boost::shared_ptr<IDirect3DVertexBuffer9> buffer_;
		size_t currentSize_;

		size_t numVertices_;

		bool staticStream_;
	};

	typedef boost::shared_ptr<D3D9VertexStream> D3D9VertexStreamPtr;
}

#endif			// _D3D9VERTEXSTREAM_HPP
