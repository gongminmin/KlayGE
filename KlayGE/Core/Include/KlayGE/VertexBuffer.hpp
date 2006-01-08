// VertexBuffer.hpp
// KlayGE 顶点缓冲区类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 支持32位的IndexStream (2006.1.4)
//
// 3.1.0
// 分离了实例和几何流 (2005.10.31)
//
// 3.0.0
// 支持在单流中存储多种成员 (2005.10.15)
// 支持instancing (2005.10.21)
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
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	enum IndexFormat
	{
		IF_Index32,
		IF_Index16
	};

	enum BufferUsage
	{
		BU_Static,
		BU_Dynamic
	};
	enum BufferAccess
	{
		BA_Read_Only,
		BA_Write_Only,
		BA_Read_Write
	};

	enum VertexElementUsage
	{
		// vertex positions
		VEU_Position,
		// vertex normals included (for lighting)
		VEU_Normal,
		// Vertex colors - diffuse
		VEU_Diffuse,
		// Vertex colors - specular
		VEU_Specular,
		// Vertex blend weights
		VEU_BlendWeight,
		// Vertex blend indices
		VEU_BlendIndex,
		// at least one set of texture coords (exact number specified in class)
		VEU_TextureCoord,
		// Vertex tangent
		VEU_Tangent,
		// Vertex binormal
		VEU_Binormal
	};

	struct vertex_element
	{
		vertex_element()
		{
		}
		vertex_element(VertexElementUsage usage, uint8_t usage_index, uint8_t component_size, uint8_t num_components)
			: usage(usage), usage_index(usage_index), component_size(component_size), num_components(num_components)
		{
		}

		VertexElementUsage usage;
		uint8_t usage_index;

		// 表示元素中每个成分的大小，比如Position元素的成分是size(float)
		uint8_t component_size;
		// 表示一个元素有几个成分表示，比如Position元素是由(x, y, z)组成，所以为3
		uint8_t num_components;

		uint16_t element_size() const
		{
			return component_size * num_components;
		}

		friend bool
		operator==(vertex_element const & lhs, vertex_element const & rhs)
		{
			return (lhs.usage == rhs.usage)
				&& (lhs.usage_index == rhs.usage_index)
				&& (lhs.component_size == rhs.component_size)
				&& (lhs.num_components == rhs.num_components);
		}
	};
	typedef std::vector<vertex_element> vertex_elements_type;

	class VertexStream
	{
	public:
		enum stream_type
		{
			ST_Geometry,
			ST_Instance
		};

		class Mapper : boost::noncopyable
		{
		public:
			Mapper(VertexStream& buffer, BufferAccess ba)
				: buffer_(buffer)
			{
				data_ = buffer_.Map(ba);
			}
			~Mapper()
			{
				buffer_.Unmap();
			}

			template <typename T>
			const T* Pointer() const
			{
				return static_cast<T*>(data_);
			}
			template <typename T>
			T* Pointer()
			{
				return static_cast<T*>(data_);
			}

		private:
			VertexStream& buffer_;
			void* data_;
		};

	public:
		explicit VertexStream(BufferUsage usage);
		virtual ~VertexStream();

		static VertexStreamPtr NullObject();

		void Resize(uint32_t size_in_byte);
		uint32_t Size() const
		{
			return size_in_byte_;
		}

		BufferUsage Usage() const
		{
			return usage_;
		}

		virtual void* Map(BufferAccess ba) = 0;
		virtual void Unmap() = 0;

		void CopyToStream(VertexStream& rhs);

		void FrequencyDivider(stream_type type, uint32_t freq);
		stream_type StreamType() const;
		uint32_t Frequency() const;

	private:
		virtual void DoCreate() = 0;

	protected:
		BufferUsage usage_;

		uint32_t size_in_byte_;

		stream_type type_;
		uint32_t freq_;
	};


	class IndexStream
	{
	public:
		class Mapper : boost::noncopyable
		{
		public:
			Mapper(IndexStream& buffer, BufferAccess ba)
				: buffer_(buffer)
			{
				data_ = buffer_.Map(ba);
			}
			~Mapper()
			{
				buffer_.Unmap();
			}

			template <typename T>
			const T* Pointer() const
			{
				return static_cast<T*>(data_);
			}
			template <typename T>
			T* Pointer()
			{
				return static_cast<T*>(data_);
			}

		private:
			IndexStream& buffer_;
			void* data_;
		};

	public:
		explicit IndexStream(BufferUsage usage);
		virtual ~IndexStream();

		static IndexStreamPtr NullObject();

		void Resize(uint32_t size_in_byte);
		uint32_t Size() const
		{
			return size_in_byte_;
		}

		BufferUsage Usage() const
		{
			return usage_;
		}

		virtual void* Map(BufferAccess ba) = 0;
		virtual void Unmap() = 0;

		void CopyToStream(IndexStream& rhs);

	private:
		virtual void DoCreate() = 0;

	protected:
		BufferUsage usage_;

		uint32_t size_in_byte_;
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

		static VertexBufferPtr NullObject();

		BufferType Type() const;

		uint32_t NumVertices() const;

		template <typename tuple_type>
		void AddVertexStream(VertexStreamPtr vertex_stream, tuple_type const & vertex_elems)
		{
			this->AddVertexStream(vertex_stream, Tuple2Vector(vertex_elems));
		}
		void AddVertexStream(VertexStreamPtr vertex_stream, vertex_elements_type const & vet);

		VertexStreamIterator VertexStreamBegin();
		VertexStreamIterator VertexStreamEnd();
		VertexStreamConstIterator VertexStreamBegin() const;
		VertexStreamConstIterator VertexStreamEnd() const;
		vertex_elements_type const & VertexStreamFormat(uint32_t index) const
		{
			return vertex_stream_formats_[index];
		}
		uint32_t VertexSize(uint32_t index) const
		{
			return vertex_sizes_[index];
		}

		bool UseIndices() const;
		uint32_t NumIndices() const;

		void SetIndexStream(IndexStreamPtr index_stream, IndexFormat format);
		IndexStreamPtr GetIndexStream() const;
		IndexFormat GetIndexStreamFormat() const
		{
			return index_format_;
		}

		VertexStreamPtr InstanceStream() const;
		vertex_elements_type const & InstanceStreamFormat() const
		{
			return instance_format_;
		}
		uint32_t InstanceSize() const
		{
			return instance_size_;
		}
		uint32_t NumInstance() const;

		void ExpandInstance(VertexStreamPtr& hint, uint32_t inst_no) const;

	private:
		template <typename tuple_type>
		vertex_elements_type Tuple2Vector(tuple_type const & t)
		{
			vertex_elements_type ret;
			ret.push_back(boost::tuples::get<0>(t));

			vertex_elements_type tail(Tuple2Vector(t.get_tail()));
			ret.insert(ret.end(), tail.begin(), tail.end());

			return ret;
		}
		template <>
		vertex_elements_type Tuple2Vector<boost::tuples::null_type>(boost::tuples::null_type const & /*t*/)
		{
			return vertex_elements_type();
		}

	protected:
		BufferType type_;

		VertexStreamsType vertex_streams_;
		std::vector<vertex_elements_type> vertex_stream_formats_;
		std::vector<uint32_t> vertex_sizes_;

		IndexStreamPtr index_stream_;
		IndexFormat index_format_;

		VertexStreamPtr instance_stream_;
		vertex_elements_type instance_format_;
		uint32_t instance_size_;
	};
}

#endif		// _VERTEXBUFFER_HPP
