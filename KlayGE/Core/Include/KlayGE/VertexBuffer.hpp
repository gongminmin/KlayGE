// VertexBuffer.hpp
// KlayGE VertexBuffer类 头文件
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

#ifndef _VERTEXBUFFER_HPP
#define _VERTEXBUFFER_HPP

#pragma comment(lib, "KlayGE_Core.lib")

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SharePtr.hpp>
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
		virtual void CopyTo(void* dest, size_t vertexNum) const = 0;

		virtual size_t VertexNum() const = 0;

		size_t ElementSize() const
			{ return elementSize_; }
		size_t ElementNum() const
			{ return elementNum_; }

	protected:
		U8 elementSize_;
		U8 elementNum_;
		VertexStreamType type_;
	};

	class DynamicVertexStream : public VertexStream
	{
	public:
		DynamicVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
			: VertexStream(type, elementSize, elementNum)
			{ }

		bool IsStatic() const
			{ return false; }

		void Assign(const void* data, size_t vertexNum, size_t stride = 0)
		{
			const size_t vertexSize(this->ElementSize() * this->ElementNum());
			vertices_.resize(vertexNum * vertexSize);

			if (stride != 0)
			{
				U8* dest(&vertices_[0]);
				const U8* src(static_cast<const U8*>(static_cast<const void*>(data)));
				for (size_t i = 0; i < vertexNum; ++ i)
				{
					memcpy(dest, src, vertexSize);

					dest += vertexSize;
					src += vertexSize + stride;
				}
			}
			else
			{
				memcpy(&vertices_[0], data, vertices_.size());
			}
		}

		void CopyTo(void* dest, size_t vertexNum) const
			{ memcpy(dest, &vertices_[0], vertexNum * this->ElementSize() * this->ElementNum()); }

		size_t VertexNum() const
			{ return vertices_.size() / (this->ElementSize() * this->ElementNum()); }

	private:
		std::vector<U8, alloc<U8> > vertices_;
	};

	class IndexStream
	{
	public:
		virtual ~IndexStream()
			{ }

		virtual bool IsStatic() const = 0;

		virtual void Assign(const void* src, size_t indexNum) = 0;
		virtual void CopyTo(void* dest, size_t indexNum) const = 0;

		virtual size_t IndexNum() const = 0;
	};

	class DynamicIndexStream : public IndexStream
	{
	public:
		bool IsStatic() const
			{ return false; }

		void Assign(const void* src, size_t indexNum)
		{
			indices_.resize(indexNum);
			memcpy(&indices_[0], src, indexNum * sizeof(U16));
		}

		void CopyTo(void* dest, size_t indexNum) const
			{ memcpy(dest, &indices_[0], indexNum * sizeof(U16)); }

		size_t IndexNum() const
			{ return indices_.size(); }

	private:
		std::vector<U16, alloc<U16> > indices_;
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

		typedef std::vector<VertexStreamPtr, alloc<VertexStreamPtr> > VertexStreamsType;
		typedef VertexStreamsType::iterator VertexStreamIterator;
		typedef VertexStreamsType::const_iterator VertexStreamConstIterator;

		VertexBuffer(BufferType type)
			: type_(type)
			{ }

		BufferType Type() const
			{ return type_; }

		size_t NumVertices() const
			{ return vertexStreams_.empty() ? 0 : vertexStreams_[0]->VertexNum(); }

		void AddVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum, bool staticStream = false);
		VertexStreamPtr GetVertexStream(VertexStreamType type) const;

		VertexStreamIterator VertexStreamBegin();
		VertexStreamIterator VertexStreamEnd();
		VertexStreamConstIterator VertexStreamBegin() const;
		VertexStreamConstIterator VertexStreamEnd() const;


		bool UseIndices() const
			{ return this->NumIndices() != 0; }
		size_t NumIndices() const
			{ return (NULL == indexStream_.Get()) ? 0 : indexStream_->IndexNum(); }

		void AddIndexStream(bool staticStream = false);
		IndexStreamPtr GetIndexStream() const
			{ return indexStream_; }


	private:
		VertexStreamsType vertexStreams_;

		IndexStreamPtr indexStream_;

		BufferType type_;
	};
}

#endif		// _VERTEXBUFFER_HPP
