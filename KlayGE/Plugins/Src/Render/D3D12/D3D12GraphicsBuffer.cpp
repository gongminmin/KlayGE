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
#include <KFL/ErrorHandling.hpp>
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
							counter_offset_(0),
							fmt_as_shader_res_(fmt)
	{
		curr_states_.resize(1, D3D12_RESOURCE_STATE_COMMON);
	}

	void D3D12GraphicsBuffer::CreateHWResource(void const * subres_init)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

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

		d3d_resource_ = this->CreateBuffer(access_hint_, total_size);
		gpu_vaddr_ = d3d_resource_->GetGPUVirtualAddress();

		D3D12_RESOURCE_DESC res_desc = d3d_resource_->GetDesc();
		D3D12_HEAP_PROPERTIES heap_prop;
		D3D12_HEAP_FLAGS heap_flags;
		d3d_resource_->GetHeapProperties(&heap_prop, &heap_flags);

		D3D12_RESOURCE_STATES init_state;
		if (EAH_CPU_Read == access_hint_)
		{
			init_state = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else if ((0 == access_hint_) || (access_hint_ & EAH_CPU_Read) || (access_hint_ & EAH_CPU_Write))
		{
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else
		{
			init_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		}

		curr_states_[0] = init_state;

		if (subres_init != nullptr)
		{
			ID3D12GraphicsCommandList* cmd_list = re.D3DResCmdList();
			std::lock_guard<std::mutex> lock(re.D3DResCmdListMutex());

			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
			res_desc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			ID3D12Resource* buffer;
			TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
				&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				IID_ID3D12Resource, reinterpret_cast<void**>(&buffer)));
			ID3D12ResourcePtr buffer_upload = MakeCOMPtr(buffer);

			D3D12_RANGE read_range;
			read_range.Begin = 0;
			read_range.End = 0;

			void* p;
			buffer_upload->Map(0, &read_range, &p);
			memcpy(p, subres_init, size_in_byte_);
			buffer_upload->Unmap(0, nullptr);

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (this->UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_COPY_DEST))
			{
				cmd_list->ResourceBarrier(1, &barrier);
			}

			cmd_list->CopyResource(d3d_resource_.get(), buffer_upload.get());

			curr_states_[0] = init_state;

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

			d3d_sr_view_ = MakeSharedPtr<D3D12ShaderResourceViewSimulation>(this, d3d_sr_view);
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
				ID3D12Resource* buffer;
				TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
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

			d3d_ua_view_ = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(this, d3d_ua_view);
		}
	}

	void D3D12GraphicsBuffer::DeleteHWResource()
	{
		d3d_sr_view_.reset();
		d3d_ua_view_.reset();
		counter_offset_ = 0;
		buffer_counter_upload_.reset();
		d3d_resource_.reset();
	}

	void* D3D12GraphicsBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(d3d_resource_);

		mapped_ba_ = ba;

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (ba)
		{
		case BA_Read_Only:
		case BA_Read_Write:
			re.ForceFinish();
			break;

		case BA_Write_Only:
			if ((0 == access_hint_) || (EAH_CPU_Write == access_hint_) || ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_))
			{
				re.RecycleTempBuffer(d3d_resource_, true, size_in_byte_);
				d3d_resource_ = re.AllocTempBuffer(true, size_in_byte_);
				gpu_vaddr_ = d3d_resource_->GetGPUVirtualAddress();
			}
			else
			{
				re.ForceFinish();
			}
			break;

		case BA_Write_No_Overwrite:
			break;

		default:
			KFL_UNREACHABLE("Invalid buffer access mode");
		}

		D3D12_RANGE read_range;
		read_range.Begin = 0;
		read_range.End = (ba == BA_Write_Only) ? 0 : size_in_byte_;

		void* p;
		TIFHR(d3d_resource_->Map(0, &read_range, &p));
		return p;
	}

	void D3D12GraphicsBuffer::Unmap()
	{
		BOOST_ASSERT(d3d_resource_);

		D3D12_RANGE write_range;
		write_range.Begin = 0;
		write_range.End = (mapped_ba_ == BA_Read_Only) ? 0 : size_in_byte_;

		d3d_resource_->Unmap(0, &write_range);
	}

	void D3D12GraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		BOOST_ASSERT(this->Size() <= rhs.Size());

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();
		D3D12GraphicsBuffer& d3d_gb = *checked_cast<D3D12GraphicsBuffer*>(&rhs);

		D3D12_HEAP_TYPE src_heap_type;
		if (EAH_CPU_Read == access_hint_)
		{
			src_heap_type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((access_hint_ & EAH_CPU_Read) || (access_hint_ & EAH_CPU_Write))
		{
			src_heap_type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			src_heap_type = D3D12_HEAP_TYPE_DEFAULT;
		}
		D3D12_HEAP_TYPE dst_heap_type;
		if (EAH_CPU_Read == rhs.AccessHint())
		{
			dst_heap_type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((rhs.AccessHint() & EAH_CPU_Read) || (rhs.AccessHint() & EAH_CPU_Write))
		{
			dst_heap_type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			dst_heap_type = D3D12_HEAP_TYPE_DEFAULT;
		}

		if ((src_heap_type == dst_heap_type) && (src_heap_type != D3D12_HEAP_TYPE_DEFAULT))
		{
			uint8_t const * src = static_cast<uint8_t const *>(this->Map(BA_Read_Only));
			uint8_t* dst = static_cast<uint8_t*>(d3d_gb.Map(BA_Write_Only));
			memcpy(dst, src, this->Size());
			d3d_gb.Unmap();
			this->Unmap();
		}
		else
		{
			UINT n = 0;
			D3D12_RESOURCE_BARRIER barriers[2];
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (this->UpdateResourceBarrier(0, barrier,
				src_heap_type == D3D12_HEAP_TYPE_UPLOAD ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_SOURCE))
			{
				barriers[n] = barrier;
				++ n;
			}
			if (d3d_gb.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_COPY_DEST))
			{
				barriers[n] = barrier;
				++ n;
			}
			if (n > 0)
			{
				cmd_list->ResourceBarrier(n, barriers);
			}

			cmd_list->CopyBufferRegion(d3d_gb.D3DResource().get(), 0, d3d_resource_.get(), 0, size_in_byte_);
		}
	}

	void D3D12GraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const * data)
	{
		if ((0 == access_hint_) || (access_hint_ & EAH_CPU_Read) || (access_hint_ & EAH_CPU_Write))
		{
			uint8_t* p = static_cast<uint8_t*>(this->Map(BA_Write_Only));
			memcpy(p + offset, data, size);
			this->Unmap();
		}
		else
		{
			auto& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto cmd_list = re.D3DRenderCmdList();

			auto upload_buff = re.AllocTempBuffer(true, size);

			D3D12_RANGE read_range;
			read_range.Begin = 0;
			read_range.End = 0;

			void* p;
			TIFHR(upload_buff->Map(0, &read_range, &p));
			memcpy(p, data, size);
			upload_buff->Unmap(0, nullptr);

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (this->UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_COPY_DEST))
			{
				cmd_list->ResourceBarrier(1, &barrier);
			}

			cmd_list->CopyBufferRegion(d3d_resource_.get(), offset, upload_buff.get(), 0, size);

			re.RecycleTempBuffer(upload_buff, true, size);
		}
	}
}
