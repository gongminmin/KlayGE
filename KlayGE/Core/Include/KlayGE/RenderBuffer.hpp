// RenderBuffer.hpp
// KlayGE 渲染缓冲区类 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 修改了纹理坐标 (2004.3.16)
//
// 2.0.3
// 去掉了VO_2D (2004.3.1)
// 改用vector存放数据 (2004.3.13)
//
// 2.0.0
// 初次建立 (2003.8.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERBUFFER_HPP
#define _RENDERBUFFER_HPP

#pragma comment(lib, "KlayGE_Core.lib")

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <vector>

namespace KlayGE
{
	enum VertexStreamType
	{
		// vertex positions
		VST_Positions = 1,
		// vertex normals included (for lighting)
		VST_Normals = 2,
		// Vertex colors - diffuse
		VST_Diffuses = 3,
		// Vertex colors - specular
		VST_Speculars = 4,
		// Vertex blend weights
		VST_BlendWeights = 5,
		// Vertex blend indices
		VST_BlendIndices = 6,
		// at least one set of texture coords (exact number specified in class)
		VST_TextureCoords0 = 7,
		VST_TextureCoords1 = 8,
		VST_TextureCoords2 = 9,
		VST_TextureCoords3 = 10,
		VST_TextureCoords4 = 11,
		VST_TextureCoords5 = 12,
		VST_TextureCoords6 = 13,
		VST_TextureCoords7 = 14,
	};

	class VertexStream
	{
	public:
		VertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
			: type_(type),
				elementSize_(elementSize), elementNum_(elementNum)
			{ }

		virtual ~VertexStream()
			{ }

		VertexStreamType Type() const
			{ return type_; }

		virtual bool IsStatic() const = 0;

		virtual void Assign(const void* src, size_t vertexNum, size_t stride = 0) = 0;

		virtual size_t NumVertices() const = 0;

		size_t ElementSize() const
			{ return elementSize_; }
		size_t ElementNum() const
			{ return elementNum_; }

	protected:
		U8 elementSize_;
		U8 elementNum_;
		VertexStreamType type_;
	};

	class IndexStream
	{
	public:
		virtual ~IndexStream()
			{ }

		virtual size_t NumIndices() const = 0;

		virtual bool IsStatic() const = 0;
		virtual void Assign(const void* src, size_t numIndices) = 0;
	};


	class RenderBuffer
	{
	public:
		enum BufferType
		{
			BT_PointList,
			BT_LineList,
			BT_LineStrip,
			BT_TriangleList,
			BT_TriangleStrip,
			BT_TriangleFan
		};

		typedef std::vector<VertexStreamPtr, alloc<VertexStreamPtr> > VertexStreamsType;
		typedef VertexStreamsType::iterator VertexStreamIterator;
		typedef VertexStreamsType::const_iterator VertexStreamConstIterator;

		RenderBuffer(BufferType type)
			: type_(type)
			{ vertexStreams_.reserve(16); }

		BufferType Type() const
			{ return type_; }

		size_t NumVertices() const
			{ return vertexStreams_.empty() ? 0 : vertexStreams_[0]->NumVertices(); }

		void AddVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum, bool staticStream = false);
		VertexStreamPtr GetVertexStream(VertexStreamType type) const;

		VertexStreamIterator VertexStreamBegin();
		VertexStreamIterator VertexStreamEnd();
		VertexStreamConstIterator VertexStreamBegin() const;
		VertexStreamConstIterator VertexStreamEnd() const;


		bool UseIndices() const
			{ return this->NumIndices() != 0; }

		size_t NumIndices() const
			{ return indexStream_->NumIndices(); }

		void AddIndexStream(bool staticStream = false);
		IndexStreamPtr GetIndexStream() const
			{ return indexStream_; }


	private:
		VertexStreamsType vertexStreams_;

		IndexStreamPtr indexStream_;

		BufferType type_;
	};
}

#endif		// _RENDERBUFFER_HPP
