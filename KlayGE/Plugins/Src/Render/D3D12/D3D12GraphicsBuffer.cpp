/**
 * @file D3D12GraphicsBuffer.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <algorithm>
#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12GraphicsBuffer.hpp>

namespace KlayGE
{
	D3D12GraphicsBuffer::D3D12GraphicsBuffer(BufferUsage usage, uint32_t access_hint,
							uint32_t size_in_byte, ElementFormat fmt)
						: GraphicsBuffer(usage, access_hint, size_in_byte),
							next_free_index_(0), counter_offset_(0),
							fmt_as_shader_res_(fmt), curr_state_(D3D12_RESOURCE_STATE_COMMON)
	{
	}

	D3D12GraphicsBuffer::~D3D12GraphicsBuffer()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.ForceCPUGPUSync();
		}
	}

	void D3D12GraphicsBuffer::CreateHWResource(void const * subres_init)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		D3D12_RESOURCE_STATES init_state;
		D3D12_HEAP_PROPERTIES heap_prop;
		if (EAH_CPU_Read == access_hint_)
		{
			init_state = D3D12_RESOURCE_STATE_COPY_DEST;
			heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((0 == access_hint_) || (access_hint_ & EAH_CPU_Read) || (access_hint_ & EAH_CPU_Write))
		{
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			init_state = D3D12_RESOURCE_STATE_COMMON;
			heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		}
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		uint32_t total_size = size_in_byte_;
		if ((access_hint_ & EAH_GPU_Write)
			&& !((access_hint_ & EAH_GPU_Structured) || (access_hint_ & EAH_GPU_Unordered)))
		{
			total_size = ((size_in_byte_ + 4 - 1) & ~(4 - 1)) + sizeof(uint64_t);
		}
		else if ((access_hint_ & EAH_GPU_Unordered) && (fmt_as_shader_res_ != EF_Unknown)
			&& ((access_hint_ & EAH_Append) || (access_hint_ & EAH_Counter)))
		{
			total_size = ((size_in_byte_ + D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1))
				+ sizeof(uint64_t);
		}

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = total_size;
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (access_hint_ & EAH_GPU_Unordered)
		{
			res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		ID3D12Resource* buffer;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, init_state, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&buffer)));
		buffer_ = MakeCOMPtr(buffer);
		buffer_pool_.push_back(buffer_);
		next_free_index_ = buffer_pool_.size();
		curr_state_ = init_state;

		if (subres_init != nullptr)
		{
			ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DResCmdList();
			std::lock_guard<std::mutex> lock(re.D3DResCmdListMutex());

			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
			res_desc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
				&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				IID_ID3D12Resource, reinterpret_cast<void**>(&buffer)));
			ID3D12ResourcePtr buffer_upload = MakeCOMPtr(buffer);

			void* p;
			buffer_upload->Map(0, nullptr, &p);
			memcpy(p, subres_init, size_in_byte_);
			buffer_upload->Unmap(0, nullptr);

			cmd_list->CopyResource(buffer_.get(), buffer_upload.get());

			re.CommitResCmd();
		}

		uint32_t const structure_byte_stride = NumFormatBytes(fmt_as_shader_res_);

		if ((access_hint_ & EAH_GPU_Read) && (fmt_as_shader_res_ != EF_Unknown))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3d_sr_view;
			d3d_sr_view.Format = (access_hint_ & EAH_GPU_Structured) ? DXGI_FORMAT_UNKNOWN : D3D12Mapping::MappingFormat(fmt_as_shader_res_);
			d3d_sr_view.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3d_sr_view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3d_sr_view.Buffer.FirstElement = 0;
			d3d_sr_view.Buffer.NumElements = size_in_byte_ / structure_byte_stride;
			d3d_sr_view.Buffer.StructureByteStride = (access_hint_ & EAH_GPU_Structured) ? structure_byte_stride : 0;
			d3d_sr_view.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			d3d_sr_view_ = MakeSharedPtr<D3D12ShaderResourceViewSimulation>(buffer_, d3d_sr_view);
		}

		if ((access_hint_ & EAH_GPU_Write)
			&& !((access_hint_ & EAH_GPU_Structured) || (access_hint_ & EAH_GPU_Unordered)))
		{
			counter_offset_ = (size_in_byte_ + 4 - 1) & ~(4 - 1);
		}
		else if ((access_hint_ & EAH_GPU_Unordered) && (fmt_as_shader_res_ != EF_Unknown))
		{
			if ((access_hint_ & EAH_Append) || (access_hint_ & EAH_Counter))
			{
				counter_offset_ = (size_in_byte_ + D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1)
					& ~(D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1);

				heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
				res_desc.Width = sizeof(uint64_t);
				res_desc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
					&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
					IID_ID3D12Resource, reinterpret_cast<void**>(&buffer)));
				buffer_counter_upload_ = MakeCOMPtr(buffer);
			}
			else
			{
				counter_offset_ = 0;
			}

			D3D12_UNORDERED_ACCESS_VIEW_DESC d3d_ua_view;
			if (access_hint_ & EAH_Raw)
			{
				d3d_ua_view.Format = DXGI_FORMAT_R32_TYPELESS;
				d3d_ua_view.Buffer.StructureByteStride = 0;
			}
			else if (access_hint_ & EAH_GPU_Structured)
			{
				d3d_ua_view.Format = DXGI_FORMAT_UNKNOWN;
				d3d_ua_view.Buffer.StructureByteStride = structure_byte_stride;
			}
			else
			{
				d3d_ua_view.Format = D3D12Mapping::MappingFormat(fmt_as_shader_res_);
				d3d_ua_view.Buffer.StructureByteStride = 0;
			}
			d3d_ua_view.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			d3d_ua_view.Buffer.FirstElement = 0;
			d3d_ua_view.Buffer.NumElements = size_in_byte_ / structure_byte_stride;
			d3d_ua_view.Buffer.CounterOffsetInBytes = counter_offset_;
			if (access_hint_ & EAH_Raw)
			{
				d3d_ua_view.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			}
			else
			{
				d3d_ua_view.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			}

			d3d_ua_view_ = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(buffer_, d3d_ua_view);
		}
	}

	void D3D12GraphicsBuffer::DeleteHWResource()
	{
		d3d_sr_view_.reset();
		d3d_ua_view_.reset();
		counter_offset_ = 0;
		buffer_counter_upload_.reset();
		buffer_.reset();
		buffer_pool_.clear();
	}

	void* D3D12GraphicsBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(buffer_);

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (ba)
		{
		case BA_Read_Only:
		case BA_Read_Write:
			re.ForceCPUGPUSync();
			break;

		case BA_Write_Only:
			if ((0 == access_hint_) || (EAH_CPU_Write == access_hint_) || ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_))
			{
				if (next_free_index_ == buffer_pool_.size())
				{
					this->CreateHWResource(nullptr);
				}
				else
				{
					buffer_ = buffer_pool_[next_free_index_];
					++ next_free_index_;
				}
				re.AddResourceForRecyclingAfterSync(this);
			}
			else
			{
				re.ForceCPUGPUSync();
			}
			break;

		case BA_Write_No_Overwrite:
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		void* p;
		TIF(buffer_->Map(0, nullptr, &p));
		return p;
	}

	void D3D12GraphicsBuffer::Unmap()
	{
		BOOST_ASSERT(buffer_);

		buffer_->Unmap(0, nullptr);
	}

	void D3D12GraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		BOOST_ASSERT(this->Size() <= rhs.Size());

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();
		D3D12GraphicsBuffer& d3d_gb = *checked_cast<D3D12GraphicsBuffer*>(&rhs);

		D3D12_RESOURCE_BARRIER src_barrier_before, dst_barrier_before;
		src_barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		src_barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		D3D12_HEAP_TYPE src_heap_type;
		if (EAH_CPU_Read == access_hint_)
		{
			src_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			src_heap_type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((access_hint_ & EAH_CPU_Read) || (access_hint_ & EAH_CPU_Write))
		{
			src_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
			src_heap_type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			src_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			src_heap_type = D3D12_HEAP_TYPE_DEFAULT;
		}
		src_barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		src_barrier_before.Transition.pResource = buffer_.get();
		src_barrier_before.Transition.Subresource = 0;
		dst_barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		dst_barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		D3D12_HEAP_TYPE dst_heap_type;
		if (EAH_CPU_Read == rhs.AccessHint())
		{
			dst_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			dst_heap_type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((rhs.AccessHint() & EAH_CPU_Read) || (rhs.AccessHint() & EAH_CPU_Write))
		{
			dst_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
			dst_heap_type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			dst_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			dst_heap_type = D3D12_HEAP_TYPE_DEFAULT;
		}
		dst_barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		dst_barrier_before.Transition.pResource = d3d_gb.D3DBuffer().get();
		dst_barrier_before.Transition.Subresource = 0;

		int n = 0;
		D3D12_RESOURCE_BARRIER barrier_before[2];
		if (src_heap_type != dst_heap_type)
		{
			if (D3D12_HEAP_TYPE_DEFAULT == src_heap_type)
			{
				barrier_before[n] = src_barrier_before;
				++ n;
			}
			if (D3D12_HEAP_TYPE_DEFAULT == dst_heap_type)
			{
				barrier_before[n] = dst_barrier_before;
				++ n;
			}
		}
		if (n > 0)
		{
			cmd_list->ResourceBarrier(n, &barrier_before[0]);
		}

		cmd_list->CopyBufferRegion(d3d_gb.D3DBuffer().get(), 0, buffer_.get(), 0, size_in_byte_);

		D3D12_RESOURCE_BARRIER barrier_after[2];
		if (n > 0)
		{
			barrier_after[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after[0].Transition.StateBefore = barrier_before[0].Transition.StateAfter;
			barrier_after[0].Transition.StateAfter = barrier_before[0].Transition.StateBefore;
			barrier_after[0].Transition.pResource = buffer_.get();
			barrier_after[0].Transition.Subresource = 0;
		}
		if (n > 1)
		{
			barrier_after[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after[1].Transition.StateBefore = barrier_before[1].Transition.StateAfter;
			barrier_after[1].Transition.StateAfter = barrier_before[1].Transition.StateBefore;
			barrier_after[1].Transition.pResource = d3d_gb.D3DBuffer().get();
			barrier_after[1].Transition.Subresource = 0;
		}
		if (n > 0)
		{
			cmd_list->ResourceBarrier(n, &barrier_after[0]);
		}
	}

	void D3D12GraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const * data)
	{
		uint8_t* p = static_cast<uint8_t*>(this->Map(BA_Write_Only));
		memcpy(p + offset, data, size);
		this->Unmap();
	}

	bool D3D12GraphicsBuffer::UpdateResourceBarrier(D3D12_RESOURCE_BARRIER& barrier, D3D12_RESOURCE_STATES target_state)
	{
		if (curr_state_ == target_state)
		{
			return false;
		}
		else
		{
			barrier.Transition.pResource = buffer_.get();
			barrier.Transition.StateBefore = curr_state_;
			barrier.Transition.StateAfter = target_state;
			curr_state_ = target_state;
			return true;
		}
	}

	void D3D12GraphicsBuffer::ResetBufferPool()
	{
		next_free_index_ = 0;
	}
}
