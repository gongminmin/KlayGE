// RenderLayout.cpp
// KlayGE 渲染流布局类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/RenderLayout.hpp>

namespace KlayGE
{
	RenderLayout::RenderLayout()
			: topo_type_(TT_PointList),
				index_format_(EF_Unknown),
				force_num_vertices_(0xFFFFFFFF),
				force_num_indices_(0xFFFFFFFF),
				force_num_instances_(0xFFFFFFFF),
				start_vertex_location_(0),
				start_index_location_(0),
				base_vertex_location_(0),
				start_instance_location_(0),
				streams_dirty_(true)
	{
		vertex_streams_.reserve(4);
	}

	RenderLayout::~RenderLayout()
	{
	}

	void RenderLayout::NumVertices(uint32_t n)
	{
		force_num_vertices_ = n;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::NumVertices() const
	{
		uint32_t n;
		if (0xFFFFFFFF == force_num_vertices_)
		{
			n = vertex_streams_[0].stream->Size() / vertex_streams_[0].vertex_size;
		}
		else
		{
			n = force_num_vertices_;
		}
		return n;
	}

	void RenderLayout::BindVertexStream(GraphicsBufferPtr const & buffer, std::span<VertexElement const> vet,
		stream_type type, uint32_t freq)
	{
		BOOST_ASSERT(buffer);

		uint32_t size = 0;
		for (int i = 0; i < vet.size(); ++ i)
		{
			size += vet[i].element_size();
		}

		if (ST_Geometry == type)
		{
			for (size_t i = 0; i < vertex_streams_.size(); ++ i)
			{
				if (MakeSpan(vertex_streams_[i].format) == vet)
				{
					vertex_streams_[i].stream = buffer;
					vertex_streams_[i].vertex_size = size;
					vertex_streams_[i].type = type;
					vertex_streams_[i].freq = freq;

					streams_dirty_ = true;
					return;
				}
			}

			StreamUnit vs;
			vs.stream = buffer;
			vs.format.assign(vet.begin(), vet.end());
			vs.vertex_size = size;
			vs.type = type;
			vs.freq = freq;
			vertex_streams_.push_back(vs);
		}
		else
		{
			instance_stream_.stream = buffer;
			instance_stream_.format.assign(vet.begin(), vet.end());
			instance_stream_.vertex_size = size;
			instance_stream_.type = type;
			instance_stream_.freq = freq;
		}

		streams_dirty_ = true;
	}

	bool RenderLayout::UseIndices() const
	{
		return this->NumIndices() != 0;
	}

	void RenderLayout::NumIndices(uint32_t n)
	{
		force_num_indices_ = n;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::NumIndices() const
	{
		uint32_t n = 0;
		if (index_stream_)
		{
			if (0xFFFFFFFF == force_num_indices_)
			{
				n = index_stream_->Size() / NumFormatBytes(index_format_);
			}
			else
			{
				n = force_num_indices_;
			}
		}
		return n;
	}

	void RenderLayout::BindIndexStream(GraphicsBufferPtr const & buffer, ElementFormat format)
	{
		BOOST_ASSERT((EF_R16UI == format) || (EF_R32UI == format));

		index_stream_ = buffer;
		index_format_ = format;

		streams_dirty_ = true;
	}

	GraphicsBufferPtr const & RenderLayout::GetIndexStream() const
	{
		BOOST_ASSERT(index_stream_);
		return index_stream_;
	}

	GraphicsBufferPtr const & RenderLayout::InstanceStream() const
	{
		return instance_stream_.stream;
	}

	void RenderLayout::InstanceStream(GraphicsBufferPtr const & buffer)
	{
		instance_stream_.stream = buffer;
		streams_dirty_ = true;
	}

	void RenderLayout::NumInstances(uint32_t n)
	{
		force_num_instances_ = n;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::NumInstances() const
	{
		uint32_t n;
		if (0xFFFFFFFF == force_num_instances_)
		{
			if (vertex_streams_.empty())
			{
				n = 1;
			}
			else
			{
				n = vertex_streams_[0].freq;
			}
		}
		else
		{
			n = force_num_instances_;
		}
		return n;
	}

	void RenderLayout::StartVertexLocation(uint32_t location)
	{
		start_vertex_location_ = location;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::StartVertexLocation() const
	{
		return start_vertex_location_;
	}

	void RenderLayout::StartIndexLocation(uint32_t location)
	{
		start_index_location_ = location;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::StartIndexLocation() const
	{
		return start_index_location_;
	}

	void RenderLayout::StartInstanceLocation(uint32_t location)
	{
		start_instance_location_ = location;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::StartInstanceLocation() const
	{
		return start_instance_location_;
	}

	void RenderLayout::BindIndirectArgs(GraphicsBufferPtr const & args_buff)
	{
		indirect_args_buff_ = args_buff;
		streams_dirty_ = true;
	}

	GraphicsBufferPtr const & RenderLayout::GetIndirectArgs() const
	{
		return indirect_args_buff_;
	}

	void RenderLayout::IndirectArgsOffset(uint32_t offset)
	{
		indirect_args_offset = offset;
		streams_dirty_ = true;
	}

	uint32_t RenderLayout::IndirectArgsOffset() const
	{
		return indirect_args_offset;
	}
}
