#ifndef _D3D9INDEXSTREAM_HPP
#define _D3D9INDEXSTREAM_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9IndexStream : public IndexStream
	{
	public:
		D3D9IndexStream(bool staticStream);

		bool IsStatic() const;

		void Assign(void const * src, size_t numIndices);

		boost::shared_ptr<IDirect3DIndexBuffer9> D3D9Buffer() const;
		size_t NumIndices() const;

	private:
		boost::shared_ptr<IDirect3DIndexBuffer9> buffer_;
		size_t currentSize_;

		size_t numIndices_;

		bool staticStream_;
	};
}

#endif			// _D3D9INDEXSTREAM_HPP
