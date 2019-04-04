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
#include <KFL/Hash.hpp>
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
							uint32_t size_in_byte, uint32_t structure_byte_stride)
						: GraphicsBuffer(usage, access_hint, size_in_byte, structure_byte_stride),
							counter_offset_(0)
	{
		curr_states_.resize(1, D3D12_RESOURCE_STATE_COMMON);
	}

	D3D12ShaderResourceViewSimulationPtr const & D3D12GraphicsBuffer::RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_elem);
		HashCombine(hash_val, num_elems);

		auto iter = d3d_sr_views_.find(hash_val);
		if (iter != d3d_sr_views_.end())
		{
			return iter->second;
		}
		else
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Format = (access_hint_ & EAH_GPU_Structured) ? DXGI_FORMAT_UNKNOWN : D3D12Mapping::MappingFormat(pf);
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Buffer.FirstElement = first_elem;
			desc.Buffer.NumElements = num_elems;
			desc.Buffer.StructureByteStride = (access_hint_ & EAH_GPU_Structured) ? structure_byte_stride_ : 0;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			auto sr_view = MakeSharedPtr<D3D12ShaderResourceViewSimulation>(this, desc);
			return d3d_sr_views_.emplace(hash_val, sr_view).first->second;
		}
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12GraphicsBuffer::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_elem);
		HashCombine(hash_val, num_elems);

		auto iter = d3d_rt_views_.find(hash_val);
		if (iter != d3d_rt_views_.end())
		{
			return iter->second;
		}
		else
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc;
			desc.Format = D3D12Mapping::MappingFormat(pf);
			desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
			desc.Buffer.FirstElement = first_elem;
			desc.Buffer.NumElements = num_elems;

			auto rt_view = MakeSharedPtr<D3D12RenderTargetViewSimulation>(this, desc);
			return d3d_rt_views_.emplace(hash_val, rt_view).first->second;
		}
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12GraphicsBuffer::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_elem);
		HashCombine(hash_val, num_elems);

		auto iter = d3d_ua_views_.find(hash_val);
		if (iter != d3d_ua_views_.end())
		{
			return iter->second;
		}
		else
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC d3d_ua_view;
			if (access_hint_ & EAH_Raw)
			{
				d3d_ua_view.Format = DXGI_FORMAT_R32_TYPELESS;
				d3d_ua_view.Buffer.StructureByteStride = 0;
			}
			else if (access_hint_ & EAH_GPU_Structured)
			{
				d3d_ua_view.Format = DXGI_FORMAT_UNKNOWN;
				d3d_ua_view.Buffer.StructureByteStride = structure_byte_stride_;
			}
			else
			{
				d3d_ua_view.Format = D3D12Mapping::MappingFormat(pf);
				d3d_ua_view.Buffer.StructureByteStride = 0;
			}
			d3d_ua_view.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			d3d_ua_view.Buffer.FirstElement = first_elem;
			d3d_ua_view.Buffer.NumElements = num_elems;
			d3d_ua_view.Buffer.CounterOffsetInBytes = counter_offset_;
			if (access_hint_ & EAH_Raw)
			{
				d3d_ua_view.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			}
			else
			{
				d3d_ua_view.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			}

			auto ua_view = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(this, d3d_ua_view);
			return d3d_ua_views_.emplace(hash_val, ua_view).first->second;
		}
	}

	void D3D12GraphicsBuffer::CreateHWResource(void const * subres_init)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		uint32_t total_size = size_in_byte_;
		if ((access_hint_ & EAH_GPU_Write)
			&& !((access_hint_ & EAH_GPU_Structured) || (access_hint_ & EAH_GPU_Unordered)))
		{
			total_size = ((size_in_byte_ + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1)) + sizeof(uint64_t);
		}
		else if ((access_hint_ & EAH_GPU_Unordered) && (structure_byte_stride_ != 0)
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

			this->UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_COPY_DEST);
			re.FlushResourceBarriers(cmd_list);

			cmd_list->CopyResource(d3d_resource_.get(), buffer_upload.get());

			curr_states_[0] = init_state;

			re.CommitResCmd();
		}

		if ((access_hint_ & EAH_GPU_Write)
			&& !((access_hint_ & EAH_GPU_Structured) || (access_hint_ & EAH_GPU_Unordered)))
		{
			counter_offset_ = (size_in_byte_ + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1);
		}
		else if ((access_hint_ & EAH_GPU_Unordered) && (structure_byte_stride_ != 0))
		{
			if ((access_hint_ & EAH_Append) || (access_hint_ & EAH_Counter))
			{
				counter_offset_ = (size_in_byte_ + D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1)
					& ~(D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1);
			}
			else
			{
				counter_offset_ = 0;
			}
		}
	}

	void D3D12GraphicsBuffer::DeleteHWResource()
	{
		d3d_sr_views_.clear();
		d3d_rt_views_.clear();
		d3d_ua_views_.clear();
		counter_offset_ = 0;
		d3d_resource_.reset();
	}

	bool D3D12GraphicsBuffer::HWResourceReady() const
	{
		return d3d_resource_.get() ? true : false;
	}

	void* D3D12GraphicsBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(d3d_resource_);

		mapped_ba_ = ba;

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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

	void D3D12GraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
	{
		this->CopyToSubBuffer(target, 0, 0, size_in_byte_);
	}

	void D3D12GraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target,
		uint32_t dst_offset, uint32_t src_offset, uint32_t size)
	{
		BOOST_ASSERT(src_offset + size <= this->Size());
		BOOST_ASSERT(dst_offset + size <= target.Size());

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* cmd_list = re.D3DRenderCmdList();
		auto& d3d_gb = checked_cast<D3D12GraphicsBuffer&>(target);

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
		if (EAH_CPU_Read == target.AccessHint())
		{
			dst_heap_type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((target.AccessHint() & EAH_CPU_Read) || (target.AccessHint() & EAH_CPU_Write))
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
			uint8_t* dst = static_cast<uint8_t*>(d3d_gb.Map(BA_Read_Write));
			memcpy(dst + dst_offset, src + src_offset, size);
			d3d_gb.Unmap();
			this->Unmap();
		}
		else
		{
			this->UpdateResourceBarrier(cmd_list, 0,
				src_heap_type == D3D12_HEAP_TYPE_UPLOAD ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_SOURCE);
			d3d_gb.UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_COPY_DEST);
			re.FlushResourceBarriers(cmd_list);

			cmd_list->CopyBufferRegion(d3d_gb.D3DResource().get(), dst_offset, d3d_resource_.get(), src_offset, size);
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
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto cmd_list = re.D3DRenderCmdList();

			auto upload_buff = re.AllocTempBuffer(true, size);

			D3D12_RANGE read_range;
			read_range.Begin = 0;
			read_range.End = 0;

			void* p;
			TIFHR(upload_buff->Map(0, &read_range, &p));
			memcpy(p, data, size);
			upload_buff->Unmap(0, nullptr);

			this->UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_COPY_DEST);
			re.FlushResourceBarriers(cmd_list);

			cmd_list->CopyBufferRegion(d3d_resource_.get(), offset, upload_buff.get(), 0, size);

			re.RecycleTempBuffer(upload_buff, true, size);
		}
	}

	void D3D12GraphicsBuffer::ResetInitCount(uint64_t count)
	{
		if (counter_offset_ > 0)
		{
			this->UpdateSubresource(counter_offset_, sizeof(count), &count);
		}
	}
}
