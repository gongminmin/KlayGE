#ifndef _D3D9VERTEXBUFFER_HPP
#define _D3D9VERTEXBUFFER_HPP

#include <KlayGE/COMPtr.hpp>

#include <d3d9.h>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	class D3D9StaticVertexStream : public VertexStream
	{
	public:
		D3D9StaticVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
			: VertexStream(type, elementSize, elementNum)
			{ }

		bool IsStatic() const
			{ return true; }

		void Assign(const void* src, size_t vertexNum, size_t stride = 0);
		void CopyTo(void* dest, size_t vertexNum) const;

		size_t VertexNum() const
			{ return vertexNum_; }

		COMPtr<IDirect3DVertexBuffer9> D3D9Buffer() const
			{ return buffer_; }

	private:
		COMPtr<IDirect3DVertexBuffer9> buffer_;
		size_t vertexNum_;
	};


	class D3D9StaticIndexStream : public IndexStream
	{
	public:
		bool IsStatic() const
			{ return true; }

		void Assign(const void* src, size_t indexNum);
		void CopyTo(void* dest, size_t indexNum) const;

		size_t IndexNum() const
			{ return indexNum_; }

	private:
		COMPtr<IDirect3DIndexBuffer9> buffer_;
		size_t indexNum_;
	};
}

#endif			// _D3D9VERTEXBUFFER_HPP