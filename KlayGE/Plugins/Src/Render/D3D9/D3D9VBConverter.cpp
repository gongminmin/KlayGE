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
#include <KlaygE/VertexBuffer.hpp>

#include <KlayGE/D3D9/D3D9VBConverter.hpp>

namespace KlayGE
{
	// 连接上d3dDevice
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::Attach(const COMPtr<IDirect3DDevice9>& d3dDevice)
	{
		d3dDevice_ = d3dDevice;
	}

	// 更新D3D9的VertexBuffer
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::Update(const VertexBuffer& vb)
	{
		this->VertexPrimFromVB(vb);

		this->UpdateStreams(vb);

		this->CopyAllBuffers(vb);
	}

	// 更新多流，建立各个缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::UpdateStreams(const VertexBuffer& vb)
	{
		VertexDeclType shaderDecl;
		shaderDecl.reserve(currentDecl_.size());

		D3DVERTEXELEMENT9 element;
		element.Offset = 0;
		element.Method = D3DDECLMETHOD_DEFAULT;

		// Vertex xyzs
		{
			element.Stream		= shaderDecl.size();
			element.Type		= D3DDECLTYPE_FLOAT3;
			element.Usage		= D3DDECLUSAGE_POSITION;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);

			this->UpdateAStream(element.Stream, xyzBuffer_,
				sizeof(D3DVECTOR), vb.NumVertices());
		}

		// Normal
		if (vb.vertexOptions & VertexBuffer::VO_Normals)
		{
			element.Stream		= shaderDecl.size();
			element.Type		= D3DDECLTYPE_FLOAT3;
			element.Usage		= D3DDECLUSAGE_NORMAL;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);

			this->UpdateAStream(element.Stream, normalBuffer_,
				sizeof(D3DVECTOR), vb.NumVertices());
		}

		// Vertex colors
		if (vb.vertexOptions & VertexBuffer::VO_Diffuses)
		{
			element.Stream		= shaderDecl.size();
			element.Type		= D3DDECLTYPE_FLOAT4;
			element.Usage		= D3DDECLUSAGE_COLOR;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);

			this->UpdateAStream(element.Stream, diffuseBuffer_,
				sizeof(float) * 4, vb.NumVertices());
		}

		// Vertex speculars
		if (vb.vertexOptions & VertexBuffer::VO_Speculars)
		{
			element.Stream		= shaderDecl.size();
			element.Type		= D3DDECLTYPE_FLOAT4;
			element.Usage		= D3DDECLUSAGE_COLOR;
			element.UsageIndex	= 1;
			shaderDecl.push_back(element);

			this->UpdateAStream(element.Stream, specularBuffer_,
				sizeof(float) * 4, vb.NumVertices());
		}

		// Do texture coords
		if (vb.vertexOptions & VertexBuffer::VO_TextureCoords)
		{
			for (U8 i = 0; i < vb.numTextureCoordSets; ++ i)
			{
				element.Stream		= shaderDecl.size();
				element.Type		= D3DDECLTYPE_FLOAT1 + vb.numTextureDimensions[i] - 1;
				element.Usage		= D3DDECLUSAGE_TEXCOORD;
				element.UsageIndex	= i;
				shaderDecl.push_back(element);

				this->UpdateAStream(element.Stream, textures_[i][vb.numTextureDimensions[i] - 1],
					vb.numTextureDimensions[i] * sizeof(float), vb.NumVertices());
			}
		}

		// Blend Weights
		if (vb.vertexOptions & VertexBuffer::VO_BlendWeights)
		{
			element.Stream		= shaderDecl.size();
			element.Type		= D3DDECLTYPE_FLOAT4;
			element.Usage		= D3DDECLUSAGE_BLENDWEIGHT;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);

			this->UpdateAStream(element.Stream, blendWeights_,
				sizeof(float) * 4, vb.NumVertices());
		}

		// Blend Indices
		if (vb.vertexOptions & VertexBuffer::VO_BlendIndices)
		{
			element.Stream		= shaderDecl.size();
			element.Type		= D3DDECLTYPE_FLOAT4;
			element.Usage		= D3DDECLUSAGE_BLENDINDICES;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);

			this->UpdateAStream(element.Stream, blendIndices_,
				sizeof(float) * 4, vb.NumVertices());
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
			d3dDevice_->SetStreamSource(i, NULL, 0, 0);
		}


		if ((currentDecl_.size() != shaderDecl.size())
			|| !Engine::MemoryInstance().Cmp(&currentDecl_[0], &shaderDecl[0], sizeof(shaderDecl[0]) * shaderDecl.size()))
		{
			currentDecl_ = shaderDecl;

			IDirect3DVertexDeclaration9* theVertexDecl;
			d3dDevice_->CreateVertexDeclaration(&currentDecl_[0], &theVertexDecl);
			currentVertexDecl_ = COMPtr<IDirect3DVertexDeclaration9>(theVertexDecl);
		}

		d3dDevice_->SetVertexDeclaration(currentVertexDecl_.Get());
	}

	// 从VB得到多边形类型、顶点数目和多边形数目
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::VertexPrimFromVB(const VertexBuffer& vb)
	{
		vertexCount_ = vb.UseIndices() ? vb.NumIndices() : vb.NumVertices();
		switch (vb.type)
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

	// 拷贝所有的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::CopyAllBuffers(const VertexBuffer& vb)
	{
		{
			this->CopyABuffer(xyzBuffer_, &vb.vertices[0],
				sizeof(D3DVECTOR), vb.NumVertices());
		}

		if (vb.vertexOptions & VertexBuffer::VO_Normals)
		{
			this->CopyABuffer(normalBuffer_, &vb.normals[0],
				sizeof(D3DVECTOR), vb.NumVertices());
		}

		if (vb.vertexOptions & VertexBuffer::VO_Diffuses)
		{
			this->CopyABuffer(diffuseBuffer_, &vb.diffuses[0],
				sizeof(float) * 4, vb.NumVertices());
		}

		if (vb.vertexOptions & VertexBuffer::VO_Speculars)
		{
			this->CopyABuffer(specularBuffer_, &vb.speculars[0],
				sizeof(float) * 4, vb.NumVertices());
		}

		if (vb.vertexOptions & VertexBuffer::VO_TextureCoords)
		{
			for (U32 i = 0; i < vb.numTextureCoordSets; ++ i)
			{
				const VertexBuffer::TexCoordsType& texCoords(vb.texCoords[i]);
				this->CopyABuffer(textures_[i][vb.numTextureDimensions[i] - 1], &texCoords[0],
					vb.numTextureDimensions[i] * sizeof(float), vb.NumVertices());
			}
		}

		if (vb.vertexOptions & VertexBuffer::VO_BlendWeights)
		{
			this->CopyABuffer(blendWeights_, &vb.blendWeights[0],
				sizeof(float) * 4, vb.NumVertices());
		}

		if (vb.vertexOptions & VertexBuffer::VO_BlendIndices)
		{
			this->CopyABuffer(blendIndices_, &vb.blendIndices[0],
				sizeof(float) * 4, vb.NumVertices());
		}
	}

	// 更新一个流
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::UpdateAStream(U16 streamNo, HardwareVertexBuffer& buffer,
		size_t vertexSize, size_t vertexNum)
	{
		if (buffer.count < vertexNum)
		{
			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice_->CreateVertexBuffer(vertexSize * vertexNum, 
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer.buffer = COMPtr<IDirect3DVertexBuffer9>(theBuffer);
			buffer.count = vertexNum;
		}

		TIF(d3dDevice_->SetStreamSource(streamNo, buffer.buffer.Get(), 0, vertexSize));
	}

	// 拷贝一个缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9VBConverter::CopyABuffer(HardwareVertexBuffer& buffer, const void* srcData,
		size_t vertexSize, size_t vertexNum)
	{
		U8* dest;
		TIF(buffer.buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_DISCARD));
		const U8* src(reinterpret_cast<const U8*>(srcData));

		// 如果有跨距，就把数据拷贝到一个块中
		Engine::MemoryInstance().Cpy(dest, src, vertexSize * vertexNum);

		buffer.buffer->Unlock();
	}
}
