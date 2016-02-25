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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
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
											uint32_t size_in_byte, ElementFormat fmt)
						: GraphicsBuffer(usage, access_hint, size_in_byte),
							bind_flags_(bind_flags), fmt_as_shader_res_(fmt)
	{
		if ((access_hint_ & EAH_GPU_Unordered) && (fmt_as_shader_res_ != EF_Unknown))
		{
			bind_flags_ = 0;
		}

		D3D11RenderEngine const & renderEngine(*checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
	}

	ID3D11RenderTargetViewPtr const & D3D11GraphicsBuffer::D3DRenderTargetView() const
	{
		if (buffer_ && !d3d_rt_view_)
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc;
			desc.Format = D3D11Mapping::MappingFormat(fmt_as_shader_res_);
			desc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
			desc.Buffer.ElementOffset = 0;
			desc.Buffer.ElementWidth = this->Size() / NumFormatBytes(fmt_as_shader_res_);

			ID3D11RenderTargetView* rt_view;
			TIF(d3d_device_->CreateRenderTargetView(buffer_.get(), &desc, &rt_view));
			d3d_rt_view_ = MakeCOMPtr(rt_view);
		}
		return d3d_rt_view_;
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
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.DeviceFeatureLevel() > D3D_FEATURE_LEVEL_9_3)
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
		desc.StructureByteStride = NumFormatBytes(fmt_as_shader_res_);

		ID3D11Buffer* buffer;
		TIF(d3d_device_->CreateBuffer(&desc, p_subres, &buffer));
		buffer_ = MakeCOMPtr(buffer);

		if ((access_hint_ & EAH_GPU_Read) && (fmt_as_shader_res_ != EF_Unknown))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
			sr_desc.Format = (access_hint_ & EAH_GPU_Structured) ? DXGI_FORMAT_UNKNOWN : D3D11Mapping::MappingFormat(fmt_as_shader_res_);
			sr_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			sr_desc.Buffer.ElementOffset = 0;
			sr_desc.Buffer.ElementWidth = size_in_byte_ / desc.StructureByteStride;

			ID3D11ShaderResourceView* d3d_sr_view;
			TIF(d3d_device_->CreateShaderResourceView(buffer_.get(), &sr_desc, &d3d_sr_view));
			d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
		}

		if ((access_hint_ & EAH_GPU_Unordered) && (fmt_as_shader_res_ != EF_Unknown))
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
				uav_desc.Format = D3D11Mapping::MappingFormat(fmt_as_shader_res_);
			}
			uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uav_desc.Buffer.FirstElement = 0;
			uav_desc.Buffer.NumElements = size_in_byte_ / desc.StructureByteStride;
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

			ID3D11UnorderedAccessView* d3d_ua_view;
			TIF(d3d_device_->CreateUnorderedAccessView(buffer_.get(), &uav_desc, &d3d_ua_view));
			d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);
		}
	}

	void D3D11GraphicsBuffer::DeleteHWResource()
	{
		d3d_sr_view_.reset();
		d3d_ua_view_.reset();
		buffer_.reset();
	}

	void* D3D11GraphicsBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(buffer_);

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
			BOOST_ASSERT(false);
			type = D3D11_MAP_READ;
			break;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(buffer_.get(), 0, type, 0, &mapped));
		return mapped.pData;
	}

	void D3D11GraphicsBuffer::Unmap()
	{
		BOOST_ASSERT(buffer_);

		d3d_imm_ctx_->Unmap(buffer_.get(), 0);
	}

	void D3D11GraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		BOOST_ASSERT(this->Size() <= rhs.Size());

		D3D11GraphicsBuffer& d3d_gb = *checked_cast<D3D11GraphicsBuffer*>(&rhs);
		if (this->Size() == rhs.Size())
		{
			d3d_imm_ctx_->CopyResource(d3d_gb.D3DBuffer(), buffer_.get());
		}
		else
		{
			D3D11_BOX box;
			box.left = 0;
			box.right = this->Size();
			box.top = 0;
			box.bottom = 1;
			box.front = 0;
			box.back = 1;
			d3d_imm_ctx_->CopySubresourceRegion(d3d_gb.D3DBuffer(), 0, 0, 0, 0, buffer_.get(), 0, &box);
		}
	}
}
