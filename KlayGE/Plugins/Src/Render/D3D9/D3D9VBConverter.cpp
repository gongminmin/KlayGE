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
#include <KlaygE/RenderBuffer.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9VBConverter.hpp>

namespace KlayGE
{
	// 更新D3D9的VertexBuffer
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::Update(const RenderBuffer& rb)
	{
		this->VertexPrimFromRB(rb);
		this->UpdateStreams(rb);
	}

	// 更新多流，建立各个缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::UpdateStreams(const RenderBuffer& rb)
	{
		COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

		VertexDeclType shaderDecl;
		shaderDecl.reserve(currentDecl_.size());

		D3DVERTEXELEMENT9 element;
		element.Offset = 0;
		element.Method = D3DDECLMETHOD_DEFAULT;

		for (RenderBuffer::VertexStreamConstIterator iter = rb.VertexStreamBegin();
			iter != rb.VertexStreamEnd(); ++ iter)
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


			D3D9VertexStream& d3d9vs(static_cast<D3D9VertexStream&>(stream));
			COMPtr<IDirect3DVertexBuffer9> d3d9vb;
			if (d3d9vs.IsStatic())
			{
				size_t size;
				d3d9vs.D3D9Buffer(d3d9vb, size);
			}
			else
			{
				COMPtr<IDirect3DVertexBuffer9>& buffer(bufferPool_[iter - rb.VertexStreamBegin()].first);
				size_t& bufferSize(bufferPool_[iter - rb.VertexStreamBegin()].second);
				d3d9vs.D3D9Buffer(buffer, bufferSize);
				d3d9vb = buffer;
			}

			TIF(d3dDevice->SetStreamSource(element.Stream,
				d3d9vb.Get(), 0, stream.ElementSize() * stream.ElementNum()));
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
			|| !Engine::MemoryInstance().Cmp(&currentDecl_[0], &shaderDecl[0],
												sizeof(shaderDecl[0]) * shaderDecl.size()))
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
	void D3D9VBConverter::VertexPrimFromRB(const RenderBuffer& rb)
	{
		vertexCount_ = rb.UseIndices() ? rb.NumIndices() : rb.NumVertices();
		switch (rb.Type())
		{
		case RenderBuffer::BT_PointList:
			primType_ = D3DPT_POINTLIST;
			primCount_ = vertexCount_;
			break;

		case RenderBuffer::BT_LineList:
			primType_ = D3DPT_LINELIST;
			primCount_ = vertexCount_ / 2;
			break;

		case RenderBuffer::BT_LineStrip:
			primType_ = D3DPT_LINESTRIP;
			primCount_ = vertexCount_ - 1;
			break;

		case RenderBuffer::BT_TriangleList:
			primType_ = D3DPT_TRIANGLELIST;
			primCount_ = vertexCount_ / 3;
			break;

		case RenderBuffer::BT_TriangleStrip:
			primType_ = D3DPT_TRIANGLESTRIP;
			primCount_ = vertexCount_ - 2;
			break;

		case RenderBuffer::BT_TriangleFan:
			primType_ = D3DPT_TRIANGLEFAN;
			primCount_ = vertexCount_ - 2;
			break;
		}
	}
}
