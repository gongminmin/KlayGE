#ifndef _D3D9VBCONVERTER_HPP
#define _D3D9VBCONVERTER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/array.hpp>

#include <vector>
#include <d3d9.h>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	struct HardwareVertexBuffer
	{
		HardwareVertexBuffer()
			: count(0)
			{ }

		HardwareVertexBuffer(const HardwareVertexBuffer& rhs)
			: buffer(rhs.buffer),
				count(rhs.count)
			{ }

		COMPtr<IDirect3DVertexBuffer9> buffer;
		U32 count;
	};

	class D3D9VBConverter
	{
	public:
		void Update(const VertexBuffer& vb);

		D3DPRIMITIVETYPE PrimType() const
			{ return primType_; }
		U32 VertexCount() const
			{ return vertexCount_; }
		U32 PrimCount() const
			{ return primCount_; }

	private:
		// Vertex buffers.  Currently for rendering we need to place all the data
		// that we receive into one of the following vertex buffers (one for each 
		// component type).
		array<HardwareVertexBuffer, 16> buffers_;

		D3DPRIMITIVETYPE primType_;
		U32 vertexCount_;
		U32 primCount_;

		typedef std::vector<D3DVERTEXELEMENT9, alloc<D3DVERTEXELEMENT9> > VertexDeclType;
		VertexDeclType currentDecl_;
		COMPtr<IDirect3DVertexDeclaration9> currentVertexDecl_;

	private:
		void UpdateStreams(const VertexBuffer& vb);
		void UpdateAStream(U16 streamNo, HardwareVertexBuffer& buffer, size_t vertexSize, size_t vertexNum);
		void CopyABuffer(HardwareVertexBuffer& buffer, VertexStream& src);

		void VertexPrimFromVB(const VertexBuffer& vb);
	};
}

#endif			// _D3D9VBCONVERTER_HPP