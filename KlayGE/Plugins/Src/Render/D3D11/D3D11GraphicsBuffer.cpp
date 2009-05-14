// D3D11IndexBuffer.cpp
// KlayGE D3D11索引缓冲区类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Math.hpp>
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
	D3D11GraphicsBuffer::D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags, ElementInitData* init_data, ElementFormat fmt)
						: GraphicsBuffer(usage, access_hint),
							bind_flags_(bind_flags), hw_buf_size_(0), fmt_as_shader_res_(fmt)
	{
		D3D11RenderEngine const & renderEngine(*checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();
		d3d_imm_ctx_ = renderEngine.D3DDeviceImmContext();
		size_in_byte_ = 0;

		if (init_data != NULL)
		{
			D3D11_BUFFER_DESC desc;
			this->GetD3DFlags(desc.Usage, desc.CPUAccessFlags, desc.BindFlags);
			desc.ByteWidth = init_data->row_pitch;
			desc.MiscFlags = 0;

			size_in_byte_ = init_data->row_pitch;

			D3D11_SUBRESOURCE_DATA subres_init;
			subres_init.pSysMem = init_data->data;
			subres_init.SysMemPitch = init_data->row_pitch;
			subres_init.SysMemSlicePitch = init_data->slice_pitch;

			ID3D11Buffer* buffer;
			TIF(d3d_device_->CreateBuffer(&desc, &subres_init, &buffer));
			buffer_ = MakeCOMPtr(buffer);
			hw_buf_size_ = this->Size();

			if ((access_hint_ & EAH_GPU_Read) && (fmt_as_shader_res_ != EF_Unknown))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
				sr_desc.Format = D3D11Mapping::MappingFormat(fmt_as_shader_res_);
				sr_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				sr_desc.Buffer.ElementOffset = 0;
				sr_desc.Buffer.ElementWidth = size_in_byte_ / NumFormatBytes(fmt_as_shader_res_);

				ID3D11ShaderResourceView* d3d_sr_view;
				TIF(d3d_device_->CreateShaderResourceView(buffer_.get(), &sr_desc, &d3d_sr_view));
				d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
			}
		}
	}

	void D3D11GraphicsBuffer::GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags)
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

		cpu_access_flags = 0;
		if (access_hint_ & EAH_CPU_Read)
		{
			cpu_access_flags |= D3D11_CPU_ACCESS_READ;
		}
		if (access_hint_ & EAH_CPU_Write)
		{
			cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
		}

		if (D3D10_USAGE_STAGING == usage)
		{
			bind_flags = 0;
		}
		else
		{
			bind_flags = bind_flags_;
		}
		if (access_hint_ & EAH_GPU_Read)
		{
			bind_flags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (access_hint_ & EAH_GPU_Write)
		{
			bind_flags |= D3D10_BIND_STREAM_OUTPUT;
		}
	}

	void D3D11GraphicsBuffer::DoResize()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		if (this->Size() > hw_buf_size_)
		{
			D3D11_BUFFER_DESC desc;
			this->GetD3DFlags(desc.Usage, desc.CPUAccessFlags, desc.BindFlags);
			desc.ByteWidth = size_in_byte_;
			desc.MiscFlags = 0;

			ID3D11Buffer* buffer;
			TIF(d3d_device_->CreateBuffer(&desc, NULL, &buffer));
			buffer_ = MakeCOMPtr(buffer);
			hw_buf_size_ = this->Size();

			if ((access_hint_ & EAH_GPU_Read) && (fmt_as_shader_res_ != EF_Unknown))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
				sr_desc.Format = D3D11Mapping::MappingFormat(fmt_as_shader_res_);
				sr_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				sr_desc.Buffer.ElementOffset = 0;
				sr_desc.Buffer.ElementWidth = size_in_byte_ / NumFormatBytes(fmt_as_shader_res_);

				ID3D11ShaderResourceView* d3d_sr_view;
				TIF(d3d_device_->CreateShaderResourceView(buffer_.get(), &sr_desc, &d3d_sr_view));
				d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
			}
		}
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
		BOOST_ASSERT(this->Size() == rhs.Size());

		D3D11GraphicsBuffer& d3d_gb = *checked_cast<D3D11GraphicsBuffer*>(&rhs);
		d3d_imm_ctx_->CopyResource(d3d_gb.D3DBuffer().get(), buffer_.get());
	}
}
