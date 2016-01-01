// RenderLayout.hpp
// KlayGE 渲染流布局类 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 去掉了TT_TriangleFan (2007.6.23)
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERLAYOUT_HPP
#define _RENDERLAYOUT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <vector>
#include <tuple>

#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	enum VertexElementUsage
	{
		// vertex positions
		VEU_Position = 0,
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
		vertex_element(VertexElementUsage usage, uint8_t usage_index, ElementFormat format)
			: usage(usage), usage_index(usage_index), format(format)
		{
		}

		VertexElementUsage usage;
		uint8_t usage_index;

		ElementFormat format;

		uint16_t element_size() const
		{
			return NumFormatBytes(format);
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


	class KLAYGE_CORE_API RenderLayout
	{
	public:
		enum topology_type
		{
			TT_PointList,
			TT_LineList,
			TT_LineStrip,
			TT_TriangleList,
			TT_TriangleStrip,
			TT_LineList_Adj,
			TT_LineStrip_Adj,
			TT_TriangleList_Adj,
			TT_TriangleStrip_Adj,
			TT_1_Ctrl_Pt_PatchList,
			TT_2_Ctrl_Pt_PatchList,
			TT_3_Ctrl_Pt_PatchList,
			TT_4_Ctrl_Pt_PatchList,
			TT_5_Ctrl_Pt_PatchList,
			TT_6_Ctrl_Pt_PatchList,
			TT_7_Ctrl_Pt_PatchList,
			TT_8_Ctrl_Pt_PatchList,
			TT_9_Ctrl_Pt_PatchList,
			TT_10_Ctrl_Pt_PatchList,
			TT_11_Ctrl_Pt_PatchList,
			TT_12_Ctrl_Pt_PatchList,
			TT_13_Ctrl_Pt_PatchList,
			TT_14_Ctrl_Pt_PatchList,
			TT_15_Ctrl_Pt_PatchList,
			TT_16_Ctrl_Pt_PatchList,
			TT_17_Ctrl_Pt_PatchList,
			TT_18_Ctrl_Pt_PatchList,
			TT_19_Ctrl_Pt_PatchList,
			TT_20_Ctrl_Pt_PatchList,
			TT_21_Ctrl_Pt_PatchList,
			TT_22_Ctrl_Pt_PatchList,
			TT_23_Ctrl_Pt_PatchList,
			TT_24_Ctrl_Pt_PatchList,
			TT_25_Ctrl_Pt_PatchList,
			TT_26_Ctrl_Pt_PatchList,
			TT_27_Ctrl_Pt_PatchList,
			TT_28_Ctrl_Pt_PatchList,
			TT_29_Ctrl_Pt_PatchList,
			TT_30_Ctrl_Pt_PatchList,
			TT_31_Ctrl_Pt_PatchList,
			TT_32_Ctrl_Pt_PatchList
		};

		enum stream_type
		{
			ST_Geometry,
			ST_Instance
		};

		RenderLayout();
		virtual ~RenderLayout() = 0;

		void TopologyType(topology_type type)
		{
			topo_type_ = type;
		}
		topology_type TopologyType() const
		{
			return topo_type_;
		}

		void NumVertices(uint32_t n);
		uint32_t NumVertices() const;

		template <typename tuple_type>
		void BindVertexStream(GraphicsBufferPtr const & buffer, tuple_type const & vertex_elems,
			stream_type type = ST_Geometry, uint32_t freq = 1)
		{
			this->BindVertexStream(buffer, Tuple2Vector<tuple_type, std::tuple_size<tuple_type>::value>::Do(vertex_elems), type, freq);
		}
		void BindVertexStream(GraphicsBufferPtr const & buffer, vertex_elements_type const & vet,
			stream_type type = ST_Geometry, uint32_t freq = 1);

		uint32_t NumVertexStreams() const
		{
			return static_cast<uint32_t>(vertex_streams_.size());
		}
		GraphicsBufferPtr const & GetVertexStream(uint32_t index) const
		{
			return vertex_streams_[index].stream;
		}
		void SetVertexStream(uint32_t index, GraphicsBufferPtr const & gb)
		{
			vertex_streams_[index].stream = gb;
			streams_dirty_ = true;
		}
		template <typename tuple_type>
		void VertexStreamFormat(uint32_t index, tuple_type const & vertex_elems)
		{
			this->VertexStreamFormat(index, Tuple2Vector<tuple_type, std::tuple_size<tuple_type>::value>::Do(vertex_elems));
		}
		void VertexStreamFormat(uint32_t index, vertex_elements_type const & vet)
		{
			vertex_streams_[index].format = vet;
			uint32_t size = 0;
			for (size_t i = 0; i < vet.size(); ++ i)
			{
				size += vet[i].element_size();
			}
			vertex_streams_[index].vertex_size = size;

			streams_dirty_ = true;
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
			streams_dirty_ = true;
		}

		bool UseIndices() const;
		void NumIndices(uint32_t n);
		uint32_t NumIndices() const;

		void BindIndexStream(GraphicsBufferPtr const & index_stream, ElementFormat format);
		GraphicsBufferPtr const & GetIndexStream() const;
		ElementFormat IndexStreamFormat() const
		{
			return index_format_;
		}

		GraphicsBufferPtr const & InstanceStream() const;
		void InstanceStream(GraphicsBufferPtr const & buffer);
		vertex_elements_type const & InstanceStreamFormat() const
		{
			return instance_stream_.format;
		}
		uint32_t InstanceSize() const
		{
			return instance_stream_.vertex_size;
		}
		void NumInstances(uint32_t n);
		uint32_t NumInstances() const;

		void StartVertexLocation(uint32_t location);
		uint32_t StartVertexLocation() const;

		void StartIndexLocation(uint32_t location);
		uint32_t StartIndexLocation() const;

		void StartInstanceLocation(uint32_t location);
		uint32_t StartInstanceLocation() const;

		void BindIndirectArgs(GraphicsBufferPtr const & args_buff);
		GraphicsBufferPtr const & GetIndirectArgs() const;
		void IndirectArgsOffset(uint32_t offset);
		uint32_t IndirectArgsOffset() const;

		void ExpandInstance(GraphicsBufferPtr& hint, uint32_t inst_no) const;

	private:
		template <typename tuple_type, int N>
		struct Tuple2Vector
		{
			static vertex_elements_type Do(tuple_type const & t)
			{
				vertex_elements_type ret = Tuple2Vector<tuple_type, N - 1>::Do(t);
				ret.push_back(std::get<N - 1>(t));
				return ret;
			}
		};
		template <typename tuple_type>
		struct Tuple2Vector<tuple_type, 1>
		{
			static vertex_elements_type Do(tuple_type const & t)
			{
				return vertex_elements_type(1, std::get<0>(t));
			}
		};

	protected:
		topology_type topo_type_;

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

		uint32_t force_num_vertices_;
		uint32_t force_num_indices_;
		uint32_t force_num_instances_;

		uint32_t start_vertex_location_;
		uint32_t start_index_location_;
		int32_t base_vertex_location_;
		uint32_t start_instance_location_;

		GraphicsBufferPtr indirect_args_buff_;
		uint32_t indirect_args_offset;

		mutable bool streams_dirty_;
	};
}

#endif		// _RENDERLAYOUT_HPP
