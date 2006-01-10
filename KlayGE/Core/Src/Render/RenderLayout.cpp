// RenderLayout.cpp
// KlayGE 渲染流布局类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/MapVector.hpp>

#include <boost/bind.hpp>

#include <KlayGE/RenderLayout.hpp>

namespace KlayGE
{
	class NullRenderLayout : public RenderLayout
	{
	public:
		NullRenderLayout()
			: RenderLayout(BT_PointList)
		{
		}
	};

	RenderLayout::RenderLayout(buffer_type type)
			: type_(type)
	{
		vertex_streams_.reserve(8);
	}

	RenderLayout::~RenderLayout()
	{
	}

	RenderLayoutPtr RenderLayout::NullObject()
	{
		static RenderLayoutPtr obj(new NullRenderLayout);
		return obj;
	}

	RenderLayout::buffer_type RenderLayout::Type() const
	{
		return type_;
	}

	uint32_t RenderLayout::NumVertices() const
	{
		return vertex_streams_.empty() ? 0 : (vertex_streams_[0].stream->Size() / vertex_streams_[0].vertex_size);
	}

	void RenderLayout::BindVertexStream(GraphicsBufferPtr buffer, vertex_elements_type const & vet,
		stream_type type, uint32_t freq)
	{
		BOOST_ASSERT(buffer);

		uint32_t size = 0;
		for (size_t i = 0; i < vet.size(); ++ i)
		{
			size += vet[i].element_size();
		}

		if (ST_Geometry == type)
		{
			StreamUnit vs;
			vs.stream = buffer;
			vs.format = vet;
			vs.vertex_size = size;
			vs.type = type;
			vs.freq = freq;
			vertex_streams_.push_back(vs);
		}
		else
		{
			BOOST_ASSERT(!instance_stream_.stream);
			instance_stream_.stream = buffer;
			instance_stream_.format = vet;
			instance_stream_.vertex_size = size;
			instance_stream_.type = type;
			instance_stream_.freq = freq;
		}
	}

	bool RenderLayout::UseIndices() const
	{
		return this->NumIndices() != 0;
	}

	uint32_t RenderLayout::NumIndices() const
	{
		if (index_stream_)
		{
			if (IF_Index16 == index_format_)
			{
				return index_stream_->Size() / sizeof(uint16_t);
			}
			else
			{
				return index_stream_->Size() / sizeof(uint32_t);
			}
		}
		else
		{
			return 0;
		}
	}

	void RenderLayout::BindIndexStream(GraphicsBufferPtr buffer, IndexFormat format)
	{
		index_stream_ = buffer;
		index_format_ = format;
	}

	GraphicsBufferPtr RenderLayout::GetIndexStream() const
	{
		BOOST_ASSERT(index_stream_);
		return index_stream_;
	}

	GraphicsBufferPtr RenderLayout::InstanceStream() const
	{
		return instance_stream_.stream;
	}

	uint32_t RenderLayout::NumInstance() const
	{
		return vertex_streams_[0].freq;
	}

	void RenderLayout::ExpandInstance(GraphicsBufferPtr& hint, uint32_t inst_no) const
	{
		BOOST_ASSERT(instance_stream_.stream);

		uint32_t num_instance = this->NumInstance();
		BOOST_ASSERT(inst_no < num_instance);

		uint32_t num_vertices = this->NumVertices();
		
		if (!hint)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			hint = rf.MakeVertexBuffer(BU_Dynamic);
		}

		std::vector<uint8_t> instance_buffer(instance_stream_.stream->Size());
		{
			GraphicsBuffer::Mapper mapper(*instance_stream_.stream, BA_Read_Only);
			std::copy(mapper.Pointer<uint8_t>(), mapper.Pointer<uint8_t>() + instance_stream_.stream->Size(),
				instance_buffer.begin());
		}
		hint->Resize(instance_stream_.vertex_size * num_vertices);
		GraphicsBuffer::Mapper dst_mapper(*hint, BA_Write_Only);

		for (uint32_t i = 0; i < num_vertices; ++ i)
		{
			std::copy(&instance_buffer[0] + inst_no * instance_stream_.vertex_size,
				&instance_buffer[0] + (inst_no + 1) * instance_stream_.vertex_size,
				dst_mapper.Pointer<uint8_t>() + i * instance_stream_.vertex_size);
		}
	}
}
