#ifndef _D3D9VERTEXSTREAM_HPP
#define _D3D9VERTEXSTREAM_HPP

#include <KlayGE/COMPtr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	class D3D9VertexStream : public VertexStream
	{
	public:
		D3D9VertexStream(VertexStreamType type, U8 elementSize, U8 elementNum);
		virtual ~D3D9VertexStream();

		virtual void D3D9Buffer(COMPtr<IDirect3DVertexBuffer9>& buffer, size_t& size) const = 0;
	};

	class D3D9DynamicVertexStream : public D3D9VertexStream
	{
	public:
		D3D9DynamicVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum);

		bool IsStatic() const;
		size_t NumVertices() const;

		void Assign(const void* src, size_t numVertices, size_t stride = 0);

		void D3D9Buffer(COMPtr<IDirect3DVertexBuffer9>& buffer, size_t& size) const;

	protected:
		std::vector<U8, alloc<U8> > data_;
	};

	class D3D9StaticVertexStream : public D3D9VertexStream
	{
	public:
		D3D9StaticVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum);

		bool IsStatic() const;
		size_t NumVertices() const;

		void Assign(const void* src, size_t numVertices, size_t stride = 0);

		void D3D9Buffer(COMPtr<IDirect3DVertexBuffer9>& buffer, size_t& size) const;

	protected:
		COMPtr<IDirect3DVertexBuffer9> buffer_;
		size_t numVertices_;
	};
}

#endif			// _D3D9VERTEXSTREAM_HPP
