// D3D9VertexStream.hpp
// KlayGE D3D9索引流类 头文件
// Ver 2.3.1
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.1
// 改为派生自D3D9Resource (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9INDEXSTREAM_HPP
#define _D3D9INDEXSTREAM_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9IndexStream : public IndexStream, public D3D9Resource
	{
	public:
		D3D9IndexStream(bool staticStream);

		bool IsStatic() const;

		void Assign(void const * src, size_t numIndices);

		boost::shared_ptr<IDirect3DIndexBuffer9> D3D9Buffer() const;
		size_t NumIndices() const;

		void OnLostDevice();
		void OnResetDevice();

	private:
		boost::shared_ptr<IDirect3DIndexBuffer9> buffer_;
		size_t currentSize_;

		size_t numIndices_;

		bool staticStream_;

		bool reseted_;
	};

	typedef boost::shared_ptr<D3D9IndexStream> D3D9IndexStreamPtr;
}

#endif			// _D3D9INDEXSTREAM_HPP
