#ifndef _D3D9VBCONVERTER_HPP
#define _D3D9VBCONVERTER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/COMPtr.hpp>

#include <vector>
#include <d3d9.h>

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
		void Attach(const COMPtr<IDirect3DDevice9>& d3dDevice);
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
		HardwareVertexBuffer xyzBuffer_;
		HardwareVertexBuffer blendBuffer_;
		HardwareVertexBuffer normalBuffer_;
		HardwareVertexBuffer diffuseBuffer_;
		HardwareVertexBuffer specularBuffer_;
		HardwareVertexBuffer textures_[8][4];		// max 8 textures with max 4 units per texture
		HardwareVertexBuffer blendWeights_;
		HardwareVertexBuffer blendIndices_;

		COMPtr<IDirect3DDevice9>	d3dDevice_;

		D3DPRIMITIVETYPE primType_;
		U32 vertexCount_;
		U32 primCount_;

		typedef std::vector<D3DVERTEXELEMENT9, alloc<D3DVERTEXELEMENT9> > VertexDeclType;
		VertexDeclType currentDecl_;
		COMPtr<IDirect3DVertexDeclaration9> currentVertexDecl_;

	private:
		void UpdateStreams(const VertexBuffer& vb);
		void UpdateAStream(U16 streamNo, HardwareVertexBuffer& buffer, size_t vertexSize, size_t vertexNum);

		void VertexPrimFromVB(const VertexBuffer& vb);

		void CopyAllBuffers(const VertexBuffer& vb);
		void CopyABuffer(HardwareVertexBuffer& buffer, const void* srcData,
			size_t vertexSize, size_t numVertices);
	};
}

#endif			// _D3D9VBCONVERTER_HPP