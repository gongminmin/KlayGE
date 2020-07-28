// D3D11GraphicsBuffer.cpp
// KlayGE D3D11索引缓冲区类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Hash.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <algorithm>
#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>

namespace KlayGE
{
	D3D11GraphicsBuffer::D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags,
											uint32_t size_in_byte, uint32_t structure_byte_stride)
						: GraphicsBuffer(usage, access_hint, size_in_byte, structure_byte_stride),
							bind_flags_(bind_flags)
	{
		if ((access_hint_ & EAH_GPU_Structured) && (structure_byte_stride != 0))
		{
			// Structured buffer can't be vb or ib at the same time.
			bind_flags_ = 0;
		}

		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice1();
		d3d_imm_ctx_ = re.D3DDeviceImmContext1();
	}

	ID3D11ShaderResourceViewPtr const & D3D11GraphicsBuffer::RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		BOOST_ASSERT(pf != EF_Unknown);
		BOOST_ASSERT(first_elem + num_elems <= size_in_byte_ / NumFormatBytes(pf));

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
			D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Format = (access_hint_ & EAH_GPU_Structured) ? DXGI_FORMAT_UNKNOWN : D3D11Mapping::MappingFormat(pf);
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer.ElementOffset = first_elem;
			desc.Buffer.ElementWidth = num_elems;

			ID3D11ShaderResourceViewPtr d3d_sr_view;
			TIFHR(d3d_device_->CreateShaderResourceView(d3d_buffer_.get(), &desc, d3d_sr_view.put()));
			return d3d_sr_views_.emplace(hash_val, std::move(d3d_sr_view)).first->second;
		}
	}

	ID3D11RenderTargetViewPtr const & D3D11GraphicsBuffer::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		BOOST_ASSERT(pf != EF_Unknown);
		BOOST_ASSERT(first_elem + num_elems <= size_in_byte_ / NumFormatBytes(pf));
		BOOST_ASSERT(access_hint_ & EAH_GPU_Write);

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
			D3D11_RENDER_TARGET_VIEW_DESC desc;
			desc.Format = D3D11Mapping::MappingFormat(pf);
			desc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
			desc.Buffer.ElementOffset = first_elem;
			desc.Buffer.ElementWidth = num_elems;

			ID3D11RenderTargetViewPtr d3d_rt_view;
			TIFHR(d3d_device_->CreateRenderTargetView(d3d_buffer_.get(), &desc, d3d_rt_view.put()));
			return d3d_rt_views_.emplace(hash_val, std::move(d3d_rt_view)).first->second;
		}
	}

	ID3D11UnorderedAccessViewPtr const & D3D11GraphicsBuffer::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		BOOST_ASSERT(pf != EF_Unknown);
		BOOST_ASSERT(first_elem + num_elems <= size_in_byte_ / NumFormatBytes(pf));
		BOOST_ASSERT(access_hint_ & EAH_GPU_Unordered);

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
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
			if (access_hint_ & EAH_Raw)
			{
				uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			}
			else if (access_hint_ & EAH_GPU_Structured)
			{
				uav_desc.Format = DXGI_FORMAT_UNKNOWN;
			}
			else
			{
				uav_desc.Format = D3D11Mapping::MappingFormat(pf);
			}
			uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uav_desc.Buffer.FirstElement = first_elem;
			uav_desc.Buffer.NumElements = num_elems;
			uav_desc.Buffer.Flags = 0;
			if (access_hint_ & EAH_Raw)
			{
				uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
			}
			if (access_hint_ & EAH_Append)
			{
				uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
			}
			if (access_hint_ & EAH_Counter)
			{
				uav_desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
			}

			ID3D11UnorderedAccessViewPtr d3d_ua_view;
			TIFHR(d3d_device_->CreateUnorderedAccessView(d3d_buffer_.get(), &uav_desc, d3d_ua_view.put()));
			return d3d_ua_views_.emplace(hash_val, std::move(d3d_ua_view)).first->second;
		}
	}

	void D3D11GraphicsBuffer::GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags)
	{
		if (access_hint_ & EAH_Immutable)
		{
			usage = D3D11_USAGE_IMMUTABLE;
		}
		else
		{
			if ((EAH_CPU_Write == access_hint_) || ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_))
			{
				usage = D3D11_USAGE_DYNAMIC;
			}
			else
			{
				if (!(access_hint_ & EAH_CPU_Read) && !(access_hint_ & EAH_CPU_Write))
				{
					usage = D3D11_USAGE_DEFAULT;
				}
				else
				{
					usage = D3D11_USAGE_STAGING;
				}
			}
		}

		cpu_access_flags = 0;
		if (access_hint_ & EAH_CPU_Read)
		{
			cpu_access_flags |= D3D11_CPU_ACCESS_READ;
		}
		if (access_hint_ & EAH_CPU_Write)
		{
			cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
		}

		if (D3D11_USAGE_STAGING == usage)
		{
			bind_flags = 0;
		}
		else
		{
			bind_flags = bind_flags_;
		}
		if (bind_flags != D3D11_BIND_CONSTANT_BUFFER)
		{
			if ((access_hint_ & EAH_GPU_Read) && !(access_hint_ & EAH_CPU_Write))
			{
				bind_flags |= D3D11_BIND_SHADER_RESOURCE;
			}
			if (access_hint_ & EAH_GPU_Write)
			{
				if (!((access_hint_ & EAH_GPU_Structured) || (access_hint_ & EAH_GPU_Unordered)))
				{
					bind_flags |= D3D11_BIND_STREAM_OUTPUT;
				}
			}
			if (access_hint_ & EAH_GPU_Unordered)
			{
				bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
			}
		}

		misc_flags = 0;
		if (access_hint_ & EAH_GPU_Unordered)
		{
			misc_flags |= (access_hint_ & EAH_GPU_Structured)
				? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		}
		if (access_hint_ & EAH_DrawIndirectArgs)
		{
			misc_flags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}
	}

	void D3D11GraphicsBuffer::CreateHWResource(void const * init_data)
	{
		D3D11_SUBRESOURCE_DATA subres_init;
		D3D11_SUBRESOURCE_DATA* p_subres = nullptr;
		if (init_data != nullptr)
		{
			subres_init.pSysMem = init_data;
			subres_init.SysMemPitch = size_in_byte_;
			subres_init.SysMemSlicePitch = size_in_byte_;

			p_subres = &subres_init;
		}

		D3D11_BUFFER_DESC desc = {};
		this->GetD3DFlags(desc.Usage, desc.CPUAccessFlags, desc.BindFlags, desc.MiscFlags);
		desc.ByteWidth = size_in_byte_;
		desc.StructureByteStride = structure_byte_stride_;

		TIFHR(d3d_device_->CreateBuffer(&desc, p_subres, d3d_buffer_.put()));
	}

	void D3D11GraphicsBuffer::DeleteHWResource()
	{
		d3d_sr_views_.clear();
		d3d_rt_views_.clear();
		d3d_ua_views_.clear();
		d3d_buffer_.reset();
	}

	bool D3D11GraphicsBuffer::HWResourceReady() const
	{
		return d3d_buffer_.get() ? true : false;
	}

	void* D3D11GraphicsBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(d3d_buffer_);

		D3D11_MAP type;
		switch (ba)
		{
		case BA_Read_Only:
			type = D3D11_MAP_READ;
			break;

		case BA_Write_Only:
			if ((EAH_CPU_Write == access_hint_) || ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_))
			{
				type = D3D11_MAP_WRITE_DISCARD;
			}
			else
			{
				type = D3D11_MAP_WRITE;
			}
			break;

		case BA_Read_Write:
			type = D3D11_MAP_READ_WRITE;
			break;

		case BA_Write_No_Overwrite:
			type = D3D11_MAP_WRITE_NO_OVERWRITE;
			break;

		default:
			KFL_UNREACHABLE("Invalid buffer access mode");
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		TIFHR(d3d_imm_ctx_->Map(d3d_buffer_.get(), 0, type, 0, &mapped));
		return mapped.pData;
	}

	void D3D11GraphicsBuffer::Unmap()
	{
		BOOST_ASSERT(d3d_buffer_);

		d3d_imm_ctx_->Unmap(d3d_buffer_.get(), 0);
	}

	void D3D11GraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
	{
		this->CopyToSubBuffer(target, 0, 0, size_in_byte_);
	}

	void D3D11GraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target,
		uint32_t dst_offset, uint32_t src_offset, uint32_t size)
	{
		BOOST_ASSERT(src_offset + size <= this->Size());
		BOOST_ASSERT(dst_offset + size <= target.Size());

		auto& d3d_gb = checked_cast<D3D11GraphicsBuffer&>(target);
		if ((src_offset == 0) && (dst_offset == 0) && (size == this->Size()) && (size == target.Size()))
		{
			d3d_imm_ctx_->CopyResource(d3d_gb.D3DBuffer(), d3d_buffer_.get());
		}
		else
		{
			D3D11_BOX box;
			box.left = src_offset;
			box.right = src_offset + size;
			box.front = 0;
			box.top = 0;
			box.bottom = 1;
			box.back = 1;
			d3d_imm_ctx_->CopySubresourceRegion(d3d_gb.D3DBuffer(), 0, dst_offset, 0, 0, d3d_buffer_.get(), 0, &box);
		}
	}

	void D3D11GraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const * data)
	{
		D3D11_BOX* p = nullptr;
		D3D11_BOX box;
		if (!(bind_flags_ & D3D11_BIND_CONSTANT_BUFFER))
		{
			p = &box;
			box.left = offset;
			box.top = 0;
			box.front = 0;
			box.right = offset + size;
			box.bottom = 1;
			box.back = 1;
		}
		d3d_imm_ctx_->UpdateSubresource(d3d_buffer_.get(), 0, p, data, size, size);
	}
}
