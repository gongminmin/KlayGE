// D3D9VBConverter.cpp
// KlayGE 从KlayGE::VertexBuffer转化成D3D9VB的类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexBuffer.hpp>

#include <KlayGE/D3D9/D3D9VBConverter.hpp>

namespace KlayGE
{
	// 更新D3D9的VertexBuffer
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::Update(const VertexBuffer& vb)
	{
		this->VertexPrimFromVB(vb);
		this->UpdateStreams(vb);
	}

	// 更新多流，建立各个缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::UpdateStreams(const VertexBuffer& vb)
	{
		COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

		VertexDeclType shaderDecl;
		shaderDecl.reserve(currentDecl_.size());

		D3DVERTEXELEMENT9 element;
		element.Offset = 0;
		element.Method = D3DDECLMETHOD_DEFAULT;

		for (VertexBuffer::VertexStreamConstIterator iter = vb.VertexStreamBegin();
			iter != vb.VertexStreamEnd(); ++ iter)
		{
			VertexStream& stream(*(*iter));
			VertexStreamType type(stream.Type());

			element.Stream = shaderDecl.size();

			switch (type)
			{
			// Vertex xyzs
			case VST_Positions:
				element.Type		= D3DDECLTYPE_FLOAT1 - 1 + stream.ElementNum();
				element.Usage		= D3DDECLUSAGE_POSITION;
				element.UsageIndex	= 0;
				break;

			// Normal
			case VST_Normals:
				element.Type		= D3DDECLTYPE_FLOAT1 - 1 + stream.ElementNum();
				element.Usage		= D3DDECLUSAGE_NORMAL;
				element.UsageIndex	= 0;
				break;

			// Vertex colors
			case VST_Diffuses:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_COLOR;
				element.UsageIndex	= 0;
				break;

			// Vertex speculars
			case VST_Speculars:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_COLOR;
				element.UsageIndex	= 1;
				break;
			
			// Blend Weights
			case VST_BlendWeights:
				element.Type		= D3DDECLTYPE_FLOAT4;
				element.Usage		= D3DDECLUSAGE_BLENDWEIGHT;
				element.UsageIndex	= 0;
				break;

			// Blend Indices
			case VST_BlendIndices:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_BLENDINDICES;
				element.UsageIndex	= 0;
				break;

			// Do texture coords
			case VST_TextureCoords0:
			case VST_TextureCoords1:
			case VST_TextureCoords2:
			case VST_TextureCoords3:
			case VST_TextureCoords4:
			case VST_TextureCoords5:
			case VST_TextureCoords6:
			case VST_TextureCoords7:
				element.Type		= D3DDECLTYPE_FLOAT1 - 1 + stream.ElementNum();
				element.Usage		= D3DDECLUSAGE_TEXCOORD;
				element.UsageIndex	= type - VST_TextureCoords0;
				break;
			}

			shaderDecl.push_back(element);


			const size_t vertexSize(stream.ElementSize() * stream.ElementNum());
			if (stream.IsStatic())
			{
				D3D9StaticVertexStream& d3d9stream(static_cast<D3D9StaticVertexStream&>(stream));
				TIF(d3dDevice->SetStreamSource(element.Stream,
					d3d9stream.D3D9Buffer().Get(), 0, vertexSize));
			}
			else
			{
				HardwareVertexBuffer& hvb(buffers_[element.Stream]);
				this->UpdateAStream(element.Stream, hvb, vertexSize, stream.VertexNum());
				this->CopyABuffer(hvb, stream);
			}
		}

		{
			element.Stream		= 0xFF;
			element.Type		= D3DDECLTYPE_UNUSED;
			element.Usage		= 0;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);
		}

		// Clear any previous steam sources
		for (U32 i = shaderDecl.size() - 1; i < currentDecl_.size(); ++ i)
		{
			d3dDevice->SetStreamSource(i, NULL, 0, 0);
		}


		if ((currentDecl_.size() != shaderDecl.size())
			|| !Engine::MemoryInstance().Cmp(&currentDecl_[0], &shaderDecl[0], sizeof(shaderDecl[0]) * shaderDecl.size()))
		{
			currentDecl_ = shaderDecl;

			IDirect3DVertexDeclaration9* theVertexDecl;
			d3dDevice->CreateVertexDeclaration(&currentDecl_[0], &theVertexDecl);
			currentVertexDecl_ = COMPtr<IDirect3DVertexDeclaration9>(theVertexDecl);
		}

		d3dDevice->SetVertexDeclaration(currentVertexDecl_.Get());
	}

	// 从VB得到多边形类型、顶点数目和多边形数目
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::VertexPrimFromVB(const VertexBuffer& vb)
	{
		vertexCount_ = vb.UseIndices() ? vb.NumIndices() : vb.NumVertices();
		switch (vb.Type())
		{
		case VertexBuffer::BT_PointList:
			primType_ = D3DPT_POINTLIST;
			primCount_ = vertexCount_;
			break;

		case VertexBuffer::BT_LineList:
			primType_ = D3DPT_LINELIST;
			primCount_ = vertexCount_ / 2;
			break;

		case VertexBuffer::BT_LineStrip:
			primType_ = D3DPT_LINESTRIP;
			primCount_ = vertexCount_ - 1;
			break;

		case VertexBuffer::BT_TriangleList:
			primType_ = D3DPT_TRIANGLELIST;
			primCount_ = vertexCount_ / 3;
			break;

		case VertexBuffer::BT_TriangleStrip:
			primType_ = D3DPT_TRIANGLESTRIP;
			primCount_ = vertexCount_ - 2;
			break;

		case VertexBuffer::BT_TriangleFan:
			primType_ = D3DPT_TRIANGLEFAN;
			primCount_ = vertexCount_ - 2;
			break;
		}
	}

	// 更新一个流
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::UpdateAStream(U16 streamNo, HardwareVertexBuffer& buffer,
		size_t vertexSize, size_t numVertices)
	{
		COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

		if (buffer.count < numVertices)
		{
			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice->CreateVertexBuffer(vertexSize * numVertices, 
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer.buffer = COMPtr<IDirect3DVertexBuffer9>(theBuffer);
			buffer.count = numVertices;
		}

		TIF(d3dDevice->SetStreamSource(streamNo, buffer.buffer.Get(), 0, vertexSize));
	}

	// 拷贝一个缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::CopyABuffer(HardwareVertexBuffer& buffer, VertexStream& src)
	{
		void* dest;
		TIF(buffer.buffer->Lock(0, 0, &dest, D3DLOCK_DISCARD));
		src.CopyTo(dest, src.VertexNum());
		buffer.buffer->Unlock();
	}
}
