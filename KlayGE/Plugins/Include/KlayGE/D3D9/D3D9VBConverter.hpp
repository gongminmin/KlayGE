#ifndef _D3D9VBCONVERTER_HPP
#define _D3D9VBCONVERTER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/array.hpp>

#include <vector>
#include <d3d9.h>

namespace KlayGE
{
	class D3D9VBConverter
	{
	public:
		void Update(const RenderBuffer& rb);

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
		array<std::pair<COMPtr<IDirect3DVertexBuffer9>, size_t>, 16> bufferPool_;

		D3DPRIMITIVETYPE primType_;
		U32 vertexCount_;
		U32 primCount_;

		typedef std::vector<D3DVERTEXELEMENT9, alloc<D3DVERTEXELEMENT9> > VertexDeclType;
		VertexDeclType currentDecl_;
		COMPtr<IDirect3DVertexDeclaration9> currentVertexDecl_;

	private:
		void UpdateStreams(const RenderBuffer& rb);
		void VertexPrimFromRB(const RenderBuffer& rb);
	};
}

#endif			// _D3D9VBCONVERTER_HPP