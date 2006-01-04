// VertexBuffer.cpp
// KlayGE 顶点缓冲区类 实现文件
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
// 支持instancing (2005.10.22)
//
// 2.4.0
// 改名为VertexBuffer (2005.3.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/MapVector.hpp>

#include <boost/bind.hpp>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	class NullVertexStream : public VertexStream
	{
	public:
		NullVertexStream()
			: VertexStream(vertex_elements_type())
		{
		}

		bool IsStatic() const
		{
			return true;
		}

		void Assign(void const * /*src*/, uint32_t /*numVertex*/)
		{
		}
		void CopyToMemory(void* /*data*/)
		{
		}

		uint32_t NumVertices() const
		{
			return 0;
		}
	};

	VertexStream::VertexStream(vertex_elements_type const & vertex_elems)
			: vertex_elems_(vertex_elems),
				type_(ST_Geometry), freq_(1)
	{
		this->CheckVertexElems();
		this->RefreshVertexSize();
	}

	VertexStream::~VertexStream()
	{
	}

	VertexStreamPtr VertexStream::NullObject()
	{
		static VertexStreamPtr obj(new NullVertexStream);
		return obj;
	}

	void VertexStream::CheckVertexElems()
	{
		for (vertex_elements_type::const_iterator iter = vertex_elems_.begin();
			iter != vertex_elems_.end(); ++ iter)
		{
			if (iter->usage != VEU_BlendIndex)
			{
				BOOST_ASSERT(sizeof(float) == iter->component_size);
			}

			if (VEU_Position == iter->usage)
			{
				BOOST_ASSERT(iter->num_components >= 2);
			}

			if (VEU_Normal == iter->usage)
			{
				BOOST_ASSERT(3 == iter->num_components);
			}

			if ((VEU_Diffuse == iter->usage) || (VEU_Specular == iter->usage))
			{
				BOOST_ASSERT((3 == iter->num_components) || (4 == iter->num_components));
			}
		}
	}

	void VertexStream::CopyToMemory(void* data, vertex_elements_type const & rhs_vertex_elems)
	{
		if (rhs_vertex_elems == vertex_elems_)
		{
			this->CopyToMemory(data);
		}
		else
		{
			typedef MapVector<std::pair<VertexElementUsage, uint8_t>, std::pair<uint32_t, uint32_t> > ve_table_type;
			ve_table_type lhs_table;
			uint32_t offset = 0;
			for (uint32_t i = 0; i < this->NumElements(); ++ i)
			{
				vertex_element const & elem = this->Element(i);
				lhs_table.insert(std::make_pair(std::make_pair(elem.usage, elem.usage_index),
					std::make_pair(offset, elem.element_size())));
				offset += elem.element_size();
			}

			std::vector<uint8_t> lhs_buffer(this->StreamSize());
			this->CopyToMemory(&lhs_buffer[0]);

			uint8_t const * src_ptr = &lhs_buffer[0];
			uint8_t* dst_ptr = static_cast<uint8_t*>(data);
			for (uint32_t i = 0; i < this->NumVertices(); ++ i)
			{
				for (vertex_elements_type::const_iterator iter = rhs_vertex_elems.begin();
					iter != rhs_vertex_elems.end(); ++ iter)
				{
					vertex_element const & elem = *iter;
					ve_table_type::iterator lhs_iter = lhs_table.find(std::make_pair(elem.usage, elem.usage_index));

					BOOST_ASSERT(lhs_iter != lhs_table.end());
					BOOST_ASSERT(lhs_iter->second.second == elem.element_size());

					std::copy(&src_ptr[lhs_iter->second.first],
						&src_ptr[lhs_iter->second.first + lhs_iter->second.second], dst_ptr);
					dst_ptr += elem.element_size();
				}
				src_ptr += this->VertexSize();
			}
		}
	}

	void VertexStream::CopyToStream(VertexStream& rhs)
	{
		std::vector<uint8_t> target_buffer(this->StreamSize());
		this->CopyToMemory(&target_buffer[0]);

		rhs.Assign(&target_buffer[0], this->NumVertices());
	}

	uint32_t VertexStream::NumElements() const
	{
		return static_cast<uint32_t>(vertex_elems_.size());
	}

	vertex_element const & VertexStream::Element(uint32_t index) const
	{
		BOOST_ASSERT(index < vertex_elems_.size());
		
		return vertex_elems_[index];
	}

	vertex_elements_type const & VertexStream::Elements() const
	{
		return vertex_elems_;
	}

	uint16_t VertexStream::VertexSize() const
	{
		return vertex_size_;
	}

	uint32_t VertexStream::StreamSize() const
	{
		return this->VertexSize() * this->NumVertices();
	}

	VertexStream& VertexStream::Combine(VertexStreamPtr rhs)
	{
		BOOST_ASSERT(this->NumVertices() == rhs->NumVertices());

		std::vector<uint8_t> lhs_buffer(this->StreamSize());
		this->CopyToMemory(&lhs_buffer[0]);
		uint32_t const lhs_ver_size = this->VertexSize();

		std::vector<uint8_t> rhs_buffer(rhs->StreamSize());
		rhs->CopyToMemory(&rhs_buffer[0]);
		uint32_t const rhs_ver_size = rhs->VertexSize();

		std::vector<uint8_t> target_buffer(this->StreamSize() + rhs->StreamSize());

		std::vector<uint8_t>::iterator lhs_iter = lhs_buffer.begin();
		std::vector<uint8_t>::iterator rhs_iter = rhs_buffer.begin();
		std::vector<uint8_t>::iterator target_iter = target_buffer.begin();
		for (uint32_t i = 0; i < this->NumVertices(); ++ i)
		{
			std::copy(lhs_iter, lhs_iter + lhs_ver_size, target_iter);
			lhs_iter += lhs_ver_size;
			target_iter += lhs_ver_size;

			std::copy(rhs_iter, rhs_iter + rhs_ver_size, target_iter);
			rhs_iter += rhs_ver_size;
			target_iter += rhs_ver_size;
		}

		vertex_elems_.insert(vertex_elems_.end(), rhs->vertex_elems_.begin(), rhs->vertex_elems_.end());
		this->RefreshVertexSize();
		this->Assign(&target_buffer[0], this->NumVertices());

		return *this;
	}

	VertexStream& VertexStream::Append(VertexStreamPtr rhs)
	{
		BOOST_ASSERT(this->NumElements() == rhs->NumElements());
		BOOST_ASSERT(this->VertexSize() == rhs->VertexSize());

		std::vector<uint8_t> target_buffer(this->StreamSize());
		this->CopyToMemory(&target_buffer[0]);

		std::vector<uint8_t> rhs_buffer(rhs->StreamSize());
		rhs->CopyToMemory(&rhs_buffer[0], vertex_elems_);

		target_buffer.insert(target_buffer.end(), rhs_buffer.begin(), rhs_buffer.end());
		this->Assign(&target_buffer[0], this->NumVertices() + rhs->NumVertices());

		return *this;
	}

	void VertexStream::FrequencyDivider(stream_type type, uint32_t freq)
	{
		type_ = type;
		freq_ = freq;
	}

	VertexStream::stream_type VertexStream::StreamType() const
	{
		return type_;
	}

    uint32_t VertexStream::Frequency() const
	{
		return freq_;
	}

	void VertexStream::RefreshVertexSize()
	{
		vertex_size_ = 0;
		for (vertex_elements_type::const_iterator iter = vertex_elems_.begin();
			iter != vertex_elems_.end(); ++ iter)
		{
			vertex_size_ = vertex_size_ + iter->element_size();
		}
	}


	class NullIndexStream : public IndexStream
	{
	public:
		NullIndexStream()
			: IndexStream(IF_Index16)
		{
		}

		uint32_t NumIndices() const
		{
			return 0;
		}

		bool IsStatic() const
		{
			return true;
		}
		void Assign(void const * /*src*/, uint32_t /*numIndices*/)
		{
		}
		void CopyToMemory(void* /*data*/)
		{
		}
	};

	IndexStream::IndexStream(IndexFormat format)
		: format_(format)
	{
	}

	IndexStream::~IndexStream()
	{
	}

	IndexStreamPtr IndexStream::NullObject()
	{
		static IndexStreamPtr obj(new NullIndexStream);
		return obj;
	}

	uint32_t IndexStream::StreamSize() const
	{
		return ((IF_Index32 == format_) ? sizeof(uint32_t) : sizeof(uint16_t)) * this->NumIndices();
	}

	IndexStream& IndexStream::Append(IndexStreamPtr rhs, uint16_t base_index)
	{
		BOOST_ASSERT(rhs->format_ == format_);

		if (IF_Index32 == format_)
		{
			std::vector<uint32_t> target_buffer(this->NumIndices());
			this->CopyToMemory(&target_buffer[0]);

			std::vector<uint32_t> rhs_buffer(rhs->NumIndices());
			rhs->CopyToMemory(&rhs_buffer[0]);

			std::transform(rhs_buffer.begin(), rhs_buffer.end(), rhs_buffer.begin(),
				boost::bind(std::plus<uint32_t>(), base_index, _1));

			target_buffer.insert(target_buffer.end(), rhs_buffer.begin(), rhs_buffer.end());
			this->Assign(&target_buffer[0], static_cast<uint32_t>(target_buffer.size()));
		}
		else
		{
			std::vector<uint16_t> target_buffer(this->NumIndices());
			this->CopyToMemory(&target_buffer[0]);

			std::vector<uint16_t> rhs_buffer(rhs->NumIndices());
			rhs->CopyToMemory(&rhs_buffer[0]);

			std::transform(rhs_buffer.begin(), rhs_buffer.end(), rhs_buffer.begin(),
				boost::bind(std::plus<uint16_t>(), base_index, _1));

			target_buffer.insert(target_buffer.end(), rhs_buffer.begin(), rhs_buffer.end());
			this->Assign(&target_buffer[0], static_cast<uint32_t>(target_buffer.size()));
		}

		return *this;
	}

	void IndexStream::CopyToStream(IndexStream& rhs)
	{
		if (IF_Index32 == format_)
		{
			std::vector<uint32_t> target_buffer(this->NumIndices());
			this->CopyToMemory(&target_buffer[0]);
			rhs.Assign(&target_buffer[0], static_cast<uint32_t>(target_buffer.size()));
		}
		else
		{
			std::vector<uint16_t> target_buffer(this->NumIndices());
			this->CopyToMemory(&target_buffer[0]);
			rhs.Assign(&target_buffer[0], static_cast<uint32_t>(target_buffer.size()));
		}
	}


	class NullVertexBuffer : public VertexBuffer
	{
	public:
		NullVertexBuffer()
			: VertexBuffer(BT_PointList)
		{
		}
	};

	VertexBuffer::VertexBuffer(BufferType type)
			: type_(type)
	{
		vertexStreams_.reserve(8);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	VertexBufferPtr VertexBuffer::NullObject()
	{
		static VertexBufferPtr obj(new NullVertexBuffer);
		return obj;
	}

	VertexBuffer::BufferType VertexBuffer::Type() const
	{
		return type_;
	}

	uint32_t VertexBuffer::NumVertices() const
	{
		return vertexStreams_.empty() ? 0 : vertexStreams_[0]->NumVertices();
	}

	void VertexBuffer::AddVertexStream(VertexStreamPtr vertex_stream)
	{
		BOOST_ASSERT(vertex_stream);

		if (VertexStream::ST_Geometry == vertex_stream->StreamType())
		{
			vertexStreams_.push_back(vertex_stream);
		}
		else
		{
			BOOST_ASSERT(!instance_stream_);
			instance_stream_ = vertex_stream;
		}
	}

	VertexBuffer::VertexStreamIterator VertexBuffer::VertexStreamBegin()
	{
		return vertexStreams_.begin();
	}

	VertexBuffer::VertexStreamIterator VertexBuffer::VertexStreamEnd()
	{
		return vertexStreams_.end();
	}

	VertexBuffer::VertexStreamConstIterator VertexBuffer::VertexStreamBegin() const
	{
		return vertexStreams_.begin();
	}

	VertexBuffer::VertexStreamConstIterator VertexBuffer::VertexStreamEnd() const
	{
		return vertexStreams_.end();
	}

	bool VertexBuffer::UseIndices() const
	{
		return this->NumIndices() != 0;
	}

	uint32_t VertexBuffer::NumIndices() const
	{
		return (!indexStream_) ? 0 : indexStream_->NumIndices();
	}

	void VertexBuffer::SetIndexStream(IndexStreamPtr index_stream)
	{
		indexStream_ = index_stream;
	}

	IndexStreamPtr VertexBuffer::GetIndexStream() const
	{
		BOOST_ASSERT(indexStream_);
		return indexStream_;
	}

	VertexStreamPtr VertexBuffer::InstanceStream() const
	{
		return instance_stream_;
	}

	uint32_t VertexBuffer::NumInstance() const
	{
		return vertexStreams_[0]->Frequency();
	}

	void VertexBuffer::ExpandInstance(VertexStreamPtr& hint, uint32_t inst_no) const
	{
		BOOST_ASSERT(instance_stream_);

		uint32_t num_instance = this->NumInstance();
		BOOST_ASSERT(inst_no < num_instance);

		if (!hint || (instance_stream_->NumElements() != hint->NumElements())
			|| (instance_stream_->Elements() != hint->Elements()))
		{
			hint = this->ExpandInstance(inst_no);
		}
		else
		{
			uint32_t num_vertices = vertexStreams_[0]->NumVertices();

			VertexStreamPtr const & src_vs = instance_stream_;
			VertexStreamPtr dst_vs = hint;

			uint32_t const instance_size = src_vs->VertexSize();

			std::vector<uint8_t> ins_buffer(src_vs->StreamSize());
			src_vs->CopyToMemory(&ins_buffer[0]);

			std::vector<uint8_t> target_buffer(instance_size * num_vertices);

			for (uint32_t i = 0; i < num_vertices; ++ i)
			{
				std::copy(ins_buffer.begin() + inst_no * instance_size, ins_buffer.begin() + (inst_no + 1) * instance_size,
					&target_buffer[i * instance_size]);
			}

			dst_vs->Assign(&target_buffer[0], num_vertices);
			dst_vs->FrequencyDivider(VertexStream::ST_Geometry, 1);
		}
	}

	VertexStreamPtr VertexBuffer::ExpandInstance(uint32_t inst_no) const
	{
		BOOST_ASSERT(instance_stream_);

		uint32_t num_instance = this->NumInstance();
		BOOST_ASSERT(inst_no < num_instance);

		uint32_t num_vertices = vertexStreams_[0]->NumVertices();

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		VertexStreamPtr const & src_vs = instance_stream_;
		VertexStreamPtr ret = rf.MakeVertexStream(src_vs->Elements(), false);

		uint32_t const instance_size = src_vs->VertexSize();

		std::vector<uint8_t> ins_buffer(src_vs->StreamSize());
		src_vs->CopyToMemory(&ins_buffer[0]);

		std::vector<uint8_t> target_buffer(instance_size * num_vertices);

		for (uint32_t i = 0; i < num_vertices; ++ i)
		{
			std::copy(ins_buffer.begin() + inst_no * instance_size, ins_buffer.begin() + (inst_no + 1) * instance_size,
				&target_buffer[i * instance_size]);
		}

		ret->Assign(&target_buffer[0], num_vertices);
		ret->FrequencyDivider(VertexStream::ST_Geometry, 1);

		return ret;
	}

	VertexBuffer& VertexBuffer::Append(VertexBufferPtr rhs)
	{
		std::vector<uint32_t> mapping;
		mapping.reserve(vertexStreams_.size());
		for (VertexStreamsType::iterator lhs_iter = this->VertexStreamBegin();
			lhs_iter != this->VertexStreamEnd(); ++ lhs_iter)
		{
			VertexStreamPtr& lhs_vs = *lhs_iter;

			VertexStreamsType::iterator rhs_iter = rhs->VertexStreamBegin();
			while (rhs_iter != VertexStreamEnd())
			{
				VertexStreamPtr& rhs_vs = *rhs_iter;

				if (lhs_vs->Elements() == rhs_vs->Elements())
				{
					mapping.push_back(static_cast<uint32_t>(rhs_iter - rhs->VertexStreamBegin()));
					break;
				}

				++ rhs_iter;
			}
			BOOST_ASSERT(rhs_iter != VertexStreamEnd());
		}

		BOOST_ASSERT((*vertexStreams_.begin())->NumVertices() < 0xFFFF);
		indexStream_->Append(rhs->indexStream_, static_cast<uint16_t>((*vertexStreams_.begin())->NumVertices()));

		for (VertexStreamsType::iterator lhs_iter = this->VertexStreamBegin();
			lhs_iter != this->VertexStreamEnd(); ++ lhs_iter)
		{
			VertexStreamPtr& lhs_vs = *lhs_iter;

			lhs_vs->Append(rhs->vertexStreams_[mapping[lhs_iter - vertexStreams_.begin()]]);
		}
		if (instance_stream_ && rhs->instance_stream_)
		{
			instance_stream_->Append(rhs->instance_stream_);
		}

		return *this;
	}
}
