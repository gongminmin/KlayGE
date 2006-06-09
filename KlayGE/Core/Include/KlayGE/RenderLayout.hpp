// RenderLayout.hpp
// KlayGE 渲染流布局类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERLAYOUT_HPP
#define _RENDERLAYOUT_HPP

#include <KlayGE/PreDeclare.hpp>

#include <vector>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/ElementFormat.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
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
			: usage(usage), usage_index(usage_index)
		{
			switch (component_size)
			{
			case 1:
				switch (num_components)
				{
				case 1:
					format = EF_L8;
					break;

				case 3:
					format = EF_RGB8;
					break;

				case 4:
					format = EF_ARGB8;
					break;
				}
				break;

			case 2:
				switch (num_components)
				{
				case 1:
					format = EF_R16F;
					break;

				case 2:
					format = EF_GR16F;
					break;

				case 3:
					format = EF_BGR16F;
					break;

				case 4:
					format = EF_ABGR16F;
					break;
				}
				break;

			case 4:
				switch (num_components)
				{
				case 1:
					format = EF_R32F;
					break;

				case 2:
					format = EF_GR32F;
					break;

				case 3:
					format = EF_BGR32F;
					break;

				case 4:
					format = EF_ABGR32F;
					break;
				}
				break;
			}
		}

		VertexElementUsage usage;
		uint8_t usage_index;

		ElementFormat format;

		uint16_t element_size() const
		{
			return ElementFormatBytes(format);
		}

		friend bool
		operator==(vertex_element const & lhs, vertex_element const & rhs)
		{
			return (lhs.usage == rhs.usage)
				&& (lhs.usage_index == rhs.usage_index)
				&& (lhs.format == rhs.format);
		}
	};
	typedef std::vector<vertex_element> vertex_elements_type;


	class RenderLayout
	{
	public:
		enum buffer_type
		{
			BT_PointList,
			BT_LineList,
			BT_LineStrip,
			BT_TriangleList,
			BT_TriangleStrip,
			BT_TriangleFan
		};

		enum stream_type
		{
			ST_Geometry,
			ST_Instance
		};

		explicit RenderLayout(buffer_type type);
		virtual ~RenderLayout() = 0;

		static RenderLayoutPtr NullObject();

		buffer_type Type() const;

		uint32_t NumVertices() const;

		template <typename tuple_type>
		void BindVertexStream(GraphicsBufferPtr buffer, tuple_type const & vertex_elems,
			stream_type type = ST_Geometry, uint32_t freq = 1)
		{
			this->BindVertexStream(buffer, Tuple2Vector(vertex_elems), type, freq);
		}
		void BindVertexStream(GraphicsBufferPtr buffer, vertex_elements_type const & vet,
			stream_type type = ST_Geometry, uint32_t freq = 1);

		uint32_t NumVertexStreams() const
		{
			return static_cast<uint32_t>(vertex_streams_.size());
		}
		GraphicsBufferPtr GetVertexStream(uint32_t index) const
		{
			return vertex_streams_[index].stream;
		}
		vertex_elements_type const & VertexStreamFormat(uint32_t index) const
		{
			return vertex_streams_[index].format;
		}
		uint32_t VertexSize(uint32_t index) const
		{
			return vertex_streams_[index].vertex_size;
		}
		stream_type VertexStreamType(uint32_t index) const
		{
			return vertex_streams_[index].type;
		}
		uint32_t VertexStreamFrequency(uint32_t index) const
		{
			return vertex_streams_[index].freq;
		}
		void VertexStreamFrequencyDivider(uint32_t index, stream_type type, uint32_t freq)
		{
			vertex_streams_[index].type = type;
			vertex_streams_[index].freq = freq;
		}

		bool UseIndices() const;
		uint32_t NumIndices() const;

		void BindIndexStream(GraphicsBufferPtr index_stream, ElementFormat format);
		GraphicsBufferPtr GetIndexStream() const;
		ElementFormat IndexStreamFormat() const
		{
			return index_format_;
		}

		GraphicsBufferPtr InstanceStream() const;
		vertex_elements_type const & InstanceStreamFormat() const
		{
			return instance_stream_.format;
		}
		uint32_t InstanceSize() const
		{
			return instance_stream_.vertex_size;
		}
		uint32_t NumInstance() const;

		void ExpandInstance(GraphicsBufferPtr& hint, uint32_t inst_no) const;

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
		buffer_type type_;

		struct StreamUnit
		{
			GraphicsBufferPtr stream;
			vertex_elements_type format;
			uint32_t vertex_size;

			stream_type type;
			uint32_t freq;
		};

		std::vector<StreamUnit> vertex_streams_;
		StreamUnit instance_stream_;

		GraphicsBufferPtr index_stream_;
		ElementFormat index_format_;
	};
}

#endif		// _RENDERLAYOUT_HPP
