// VertexBuffer.hpp
// KlayGE 顶点缓冲区类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
//
// 2.4.0
// 改名为VertexBuffer (2005.3.7)
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

#ifndef _VERTEXBUFFER_HPP
#define _VERTEXBUFFER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <vector>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

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
		VertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t ElementsPerVertex);
		virtual ~VertexStream();

		VertexStreamType Type() const;

		virtual bool IsStatic() const = 0;

		virtual void Assign(void const * src, size_t numVertex) = 0;
		virtual void CopyToMemory(void* data) = 0;

		virtual size_t NumVertices() const = 0;

		size_t SizeOfElement() const;
		size_t ElementsPerVertex() const;

	protected:
		uint8_t sizeElement_;
		uint8_t elementPerVertex_;
		VertexStreamType type_;
	};

	class IndexStream
	{
	public:
		virtual ~IndexStream();

		virtual size_t NumIndices() const = 0;

		virtual bool IsStatic() const = 0;
		virtual void Assign(void const * src, size_t numIndices) = 0;
		virtual void CopyToMemory(void* data) = 0;
	};


	class VertexBuffer
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

		typedef std::vector<VertexStreamPtr> VertexStreamsType;
		typedef VertexStreamsType::iterator VertexStreamIterator;
		typedef VertexStreamsType::const_iterator VertexStreamConstIterator;

		explicit VertexBuffer(BufferType type);
		virtual ~VertexBuffer() = 0;

		BufferType Type() const;

		size_t NumVertices() const;

		void AddVertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t numElement, bool staticStream = false);
		void AddVertexStream(VertexStreamPtr vstream);
		VertexStreamPtr GetVertexStream(VertexStreamType type) const;

		VertexStreamIterator VertexStreamBegin();
		VertexStreamIterator VertexStreamEnd();
		VertexStreamConstIterator VertexStreamBegin() const;
		VertexStreamConstIterator VertexStreamEnd() const;


		bool UseIndices() const;
		size_t NumIndices() const;

		void AddIndexStream(bool staticStream = false);
		IndexStreamPtr GetIndexStream() const;

	protected:
		BufferType type_;

		VertexStreamsType vertexStreams_;
		IndexStreamPtr indexStream_;
	};
}

#endif		// _VERTEXBUFFER_HPP
