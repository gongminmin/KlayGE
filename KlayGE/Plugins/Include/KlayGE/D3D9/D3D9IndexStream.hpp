#ifndef _D3D9INDEXSTREAM_HPP
#define _D3D9INDEXSTREAM_HPP

#include <KlayGE/COMPtr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	class D3D9IndexStream : public IndexStream
	{
	public:
		virtual ~D3D9IndexStream();

		virtual void D3D9Buffer(COMPtr<IDirect3DIndexBuffer9>& buffer, size_t& size) const = 0;
	};

	class D3D9DynamicIndexStream : public D3D9IndexStream
	{
	public:
		bool IsStatic() const
			{ return false; }

		void Assign(const void* src, size_t numIndices);

		size_t NumIndices() const
			{ return data_.size(); }

		void D3D9Buffer(COMPtr<IDirect3DIndexBuffer9>& buffer, size_t& size) const;

	protected:
		std::vector<U16, alloc<U16> > data_;
	};

	class D3D9StaticIndexStream : public D3D9IndexStream
	{
	public:
		D3D9StaticIndexStream();

		bool IsStatic() const
			{ return true; }

		void Assign(const void* src, size_t numIndices);

		size_t NumIndices() const
			{ return numIndices_; }

		void D3D9Buffer(COMPtr<IDirect3DIndexBuffer9>& buffer, size_t& size) const;

	protected:
		COMPtr<IDirect3DIndexBuffer9> buffer_;
		size_t numIndices_;
	};
}

#endif			// _D3D9INDEXSTREAM_HPP
