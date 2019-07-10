// D3D11Texture.cpp
// KlayGE D3D11纹理类 实现文件
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
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/TexCompression.hpp>
#include <KFL/Hash.hpp>

#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/SALWrapper.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

namespace KlayGE
{
	D3D11Texture::D3D11Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint)
	{
		if (access_hint & EAH_GPU_Write)
		{
			BOOST_ASSERT(!(access_hint & EAH_CPU_Read));
			BOOST_ASSERT(!(access_hint & EAH_CPU_Write));
		}

		auto const& re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_imm_ctx_ = re.D3DDeviceImmContext();
	}

	D3D11Texture::~D3D11Texture()
	{
	}

	std::wstring const & D3D11Texture::Name() const
	{
		static const std::wstring name(L"Direct3D11 Texture");
		return name;
	}

	uint32_t D3D11Texture::Width(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t D3D11Texture::Height(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t D3D11Texture::Depth(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void D3D11Texture::CopyToSubTexture1D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_width*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_width*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::CopyToSubTexture2D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::CopyToSubTexture3D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_z_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_z_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::CopyToSubTextureCube(Texture& /*target*/,
			uint32_t /*dst_array_index*/, CubeFaces /*dst_face*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, CubeFaces /*src_face*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	ID3D11ShaderResourceViewPtr const & D3D11Texture::RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Read);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_array_index);
		HashCombine(hash_val, array_size);
		HashCombine(hash_val, first_level);
		HashCombine(hash_val, num_levels);

		auto iter = d3d_sr_views_.find(hash_val);
		if (iter != d3d_sr_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillSRVDesc(pf, first_array_index, array_size, first_level, num_levels);
			ID3D11ShaderResourceView* d3d_sr_view;
			d3d_device_->CreateShaderResourceView(this->D3DResource(), &desc, &d3d_sr_view);
			return d3d_sr_views_.emplace(hash_val, MakeCOMPtr(d3d_sr_view)).first->second;
		}
	}

	ID3D11RenderTargetViewPtr const & D3D11Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(first_array_index < this->ArraySize());
		BOOST_ASSERT(first_array_index + array_size <= this->ArraySize());

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_array_index);
		HashCombine(hash_val, array_size);
		HashCombine(hash_val, level);
		HashCombine(hash_val, 0);
		HashCombine(hash_val, 0);

		auto iter = d3d_rt_views_.find(hash_val);
		if (iter != d3d_rt_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillRTVDesc(pf, first_array_index, array_size, level);
			ID3D11RenderTargetView* d3d_rt_view;
			d3d_device_->CreateRenderTargetView(this->D3DResource(), &desc, &d3d_rt_view);
			return d3d_rt_views_.emplace(hash_val, MakeCOMPtr(d3d_rt_view)).first->second;
		}
	}

	ID3D11RenderTargetViewPtr const &  D3D11Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index,
		uint32_t first_slice, uint32_t num_slices, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(0 == array_index);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, array_index);
		HashCombine(hash_val, 1);
		HashCombine(hash_val, level);
		HashCombine(hash_val, first_slice);
		HashCombine(hash_val, num_slices);

		auto iter = d3d_rt_views_.find(hash_val);
		if (iter != d3d_rt_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillRTVDesc(pf, array_index, first_slice, num_slices, level);
			ID3D11RenderTargetView* d3d_rt_view;
			d3d_device_->CreateRenderTargetView(this->D3DResource(), &desc, &d3d_rt_view);
			return d3d_rt_views_.emplace(hash_val, MakeCOMPtr(d3d_rt_view)).first->second;
		}
	}

	ID3D11RenderTargetViewPtr const &  D3D11Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index,
		Texture::CubeFaces face, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, array_index * 6 + face);
		HashCombine(hash_val, 1);
		HashCombine(hash_val, level);
		HashCombine(hash_val, 0);
		HashCombine(hash_val, 0);

		auto iter = d3d_rt_views_.find(hash_val);
		if (iter != d3d_rt_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillRTVDesc(pf, array_index, face, level);
			ID3D11RenderTargetView* d3d_rt_view;
			d3d_device_->CreateRenderTargetView(this->D3DResource(), &desc, &d3d_rt_view);
			return d3d_rt_views_.emplace(hash_val, MakeCOMPtr(d3d_rt_view)).first->second;
		}
	}

	ID3D11DepthStencilViewPtr const & D3D11Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(first_array_index < this->ArraySize());
		BOOST_ASSERT(first_array_index + array_size <= this->ArraySize());

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_array_index);
		HashCombine(hash_val, array_size);
		HashCombine(hash_val, level);
		HashCombine(hash_val, 0);
		HashCombine(hash_val, 0);

		auto iter = d3d_ds_views_.find(hash_val);
		if (iter != d3d_ds_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillDSVDesc(pf, first_array_index, array_size, level);
			ID3D11DepthStencilView* d3d_ds_view;
			d3d_device_->CreateDepthStencilView(this->D3DResource(), &desc, &d3d_ds_view);
			return d3d_ds_views_.emplace(hash_val, MakeCOMPtr(d3d_ds_view)).first->second;
		}
	}

	ID3D11DepthStencilViewPtr const & D3D11Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index,
		uint32_t first_slice, uint32_t num_slices, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(0 == array_index);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, array_index);
		HashCombine(hash_val, 1);
		HashCombine(hash_val, level);
		HashCombine(hash_val, first_slice);
		HashCombine(hash_val, num_slices);

		auto iter = d3d_ds_views_.find(hash_val);
		if (iter != d3d_ds_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillDSVDesc(pf, array_index, first_slice, num_slices, level);
			ID3D11DepthStencilView* d3d_ds_view;
			d3d_device_->CreateDepthStencilView(this->D3DResource(), &desc, &d3d_ds_view);
			return d3d_ds_views_.emplace(hash_val, MakeCOMPtr(d3d_ds_view)).first->second;
		}
	}

	ID3D11DepthStencilViewPtr const & D3D11Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index, CubeFaces face,
		uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, array_index * 6 + face);
		HashCombine(hash_val, 1);
		HashCombine(hash_val, level);
		HashCombine(hash_val, 0);
		HashCombine(hash_val, 0);

		auto iter = d3d_ds_views_.find(hash_val);
		if (iter != d3d_ds_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillDSVDesc(pf, array_index, face, level);
			ID3D11DepthStencilView* d3d_ds_view;
			d3d_device_->CreateDepthStencilView(this->D3DResource(), &desc, &d3d_ds_view);
			return d3d_ds_views_.emplace(hash_val, MakeCOMPtr(d3d_ds_view)).first->second;
		}
	}

	ID3D11UnorderedAccessViewPtr const & D3D11Texture::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Unordered);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_array_index);
		HashCombine(hash_val, array_size);
		HashCombine(hash_val, level);
		HashCombine(hash_val, 0);
		HashCombine(hash_val, 0);

		auto iter = d3d_ua_views_.find(hash_val);
		if (iter != d3d_ua_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillUAVDesc(pf, first_array_index, array_size, level);
			ID3D11UnorderedAccessView* d3d_ua_view;
			d3d_device_->CreateUnorderedAccessView(this->D3DResource(), &desc, &d3d_ua_view);
			return d3d_ua_views_.emplace(hash_val, MakeCOMPtr(d3d_ua_view)).first->second;
		}
	}

	ID3D11UnorderedAccessViewPtr const & D3D11Texture::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t array_index,
		uint32_t first_slice, uint32_t num_slices, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Unordered);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, array_index);
		HashCombine(hash_val, 1);
		HashCombine(hash_val, level);
		HashCombine(hash_val, first_slice);
		HashCombine(hash_val, num_slices);

		auto iter = d3d_ua_views_.find(hash_val);
		if (iter != d3d_ua_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillUAVDesc(pf, array_index, first_slice, num_slices, level);
			ID3D11UnorderedAccessView* d3d_ua_view;
			d3d_device_->CreateUnorderedAccessView(this->D3DResource(), &desc, &d3d_ua_view);
			return d3d_ua_views_.emplace(hash_val, MakeCOMPtr(d3d_ua_view)).first->second;
		}
	}

	ID3D11UnorderedAccessViewPtr const & D3D11Texture::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, CubeFaces first_face, uint32_t num_faces, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Unordered);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, first_array_index * 6 + first_face);
		HashCombine(hash_val, array_size * 6 + num_faces);
		HashCombine(hash_val, level);
		HashCombine(hash_val, 0);
		HashCombine(hash_val, 0);

		auto iter = d3d_ua_views_.find(hash_val);
		if (iter != d3d_ua_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillUAVDesc(pf, first_array_index, array_size, first_face, num_faces, level);
			ID3D11UnorderedAccessView* d3d_ua_view;
			d3d_device_->CreateUnorderedAccessView(this->D3DResource(), &desc, &d3d_ua_view);
			return d3d_ua_views_.emplace(hash_val, MakeCOMPtr(d3d_ua_view)).first->second;
		}
	}

	void D3D11Texture::GetD3DFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags)
	{
		if (access_hint_ & EAH_Immutable)
		{
			usage = D3D11_USAGE_IMMUTABLE;
		}
		else
		{
			if ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_)
			{
				usage = D3D11_USAGE_DYNAMIC;
			}
			else
			{
				if (EAH_CPU_Write == access_hint_)
				{
					if ((num_mip_maps_ != 1) || (TT_Cube == type_))
					{
						usage = D3D11_USAGE_STAGING;
					}
					else
					{
						usage = D3D11_USAGE_DYNAMIC;
					}
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
		}

		bind_flags = 0;
		if ((access_hint_ & EAH_GPU_Read) || (D3D11_USAGE_DYNAMIC == usage))
		{
			bind_flags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (access_hint_ & EAH_GPU_Write)
		{
			if (IsDepthFormat(format_))
			{
				bind_flags |= D3D11_BIND_DEPTH_STENCIL;
			}
			else
			{
				bind_flags |= D3D11_BIND_RENDER_TARGET;
			}
		}
		if (access_hint_ & EAH_GPU_Unordered)
		{
			bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
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

		misc_flags = 0;
		if (access_hint_ & EAH_Generate_Mips)
		{
			misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}

	void D3D11Texture::Map1D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*width*/,
			void*& /*data*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::Map2D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::Map3D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::MapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::Unmap1D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::Unmap2D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::Unmap3D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::UnmapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture::FillRTVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture::FillRTVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture::FillRTVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture::FillDSVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture::FillDSVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture::FillDSVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC D3D11Texture::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC D3D11Texture::FillUAVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC D3D11Texture::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		CubeFaces first_face, uint32_t num_faces, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(first_face);
		KFL_UNUSED(num_faces);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D11Texture::DeleteHWResource()
	{
		d3d_rt_views_.clear();
		d3d_ds_views_.clear();
		d3d_ua_views_.clear();
		d3d_texture_.reset();
	}

	bool D3D11Texture::HWResourceReady() const
	{
		return d3d_texture_.get() ? true : false;
	}

	void D3D11Texture::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		if (access_hint_ & (EAH_GPU_Read | EAH_GPU_Write))
		{
			D3D11_BOX box;
			box.left = x_offset;
			box.top = 0;
			box.front = 0;
			box.right = x_offset + width;
			box.bottom = 1;
			box.back = 1;
			uint32_t const texel_size = NumFormatBytes(format_);
			d3d_imm_ctx_->UpdateSubresource(d3d_texture_.get(), array_index * num_mip_maps_ + level, &box,
				data, width * texel_size, width * texel_size);
		}
		else if (access_hint_ & EAH_CPU_Write)
		{
			Texture::Mapper mapper(*this, array_index, level, TMA_Write_Only, x_offset, width);

			uint8_t const * src = static_cast<uint8_t const *>(data);
			uint8_t* dst = mapper.Pointer<uint8_t>();
			uint32_t const bytes_per_row = width * NumFormatBytes(format_);
			
			std::memcpy(dst, src, bytes_per_row);
		}
		else
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			ElementInitData init_data;
			init_data.data = data;
			init_data.row_pitch = width * NumFormatBytes(format_);
			init_data.slice_pitch = init_data.row_pitch;
			TexturePtr temp_tex = rf.MakeTexture1D(width, 1, 1, format_, 1, 0, EAH_CPU_Write, MakeSpan<1>(init_data));
			d3d_imm_ctx_->CopySubresourceRegion(d3d_texture_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_),
				x_offset, 0, 0, checked_cast<D3D11Texture&>(*temp_tex).d3d_texture_.get(), D3D11CalcSubresource(0, 0, 1),
				nullptr);
		}
	}

	void D3D11Texture::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		if (access_hint_ & (EAH_GPU_Read | EAH_GPU_Write))
		{
			D3D11_BOX box;
			box.left = x_offset;
			box.top = y_offset;
			box.front = 0;
			box.right = x_offset + width;
			box.bottom = y_offset + height;
			box.back = 1;
			d3d_imm_ctx_->UpdateSubresource(d3d_texture_.get(), array_index * num_mip_maps_ + level, &box,
				data, row_pitch, row_pitch);
		}
		else if (access_hint_ & EAH_CPU_Write)
		{
			Texture::Mapper mapper(*this, array_index, level, TMA_Write_Only, x_offset, y_offset, width, height);

			uint8_t const * src = static_cast<uint8_t const *>(data);
			uint8_t* dst = mapper.Pointer<uint8_t>();
			uint32_t const dst_row_pitch = mapper.RowPitch();
			uint32_t const block_width = BlockWidth(format_);
			uint32_t const block_height = BlockHeight(format_);
			uint32_t const block_bytes = BlockBytes(format_);
			uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
			for (uint32_t y = 0; y < height; y += block_height)
			{
				std::memcpy(dst, src, bytes_per_row);

				src += row_pitch;
				dst += dst_row_pitch;
			}
		}
		else
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			ElementInitData init_data;
			init_data.data = data;
			init_data.row_pitch = row_pitch;
			init_data.slice_pitch = row_pitch * height;
			TexturePtr temp_tex = rf.MakeTexture2D(width, height, 1, 1, format_, 1, 0, EAH_CPU_Write, MakeSpan<1>(init_data));
			d3d_imm_ctx_->CopySubresourceRegion(d3d_texture_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_),
				x_offset, y_offset, 0, checked_cast<D3D11Texture&>(*temp_tex).d3d_texture_.get(), D3D11CalcSubresource(0, 0, 1),
				nullptr);
		}
	}

	void D3D11Texture::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		if (access_hint_ & (EAH_GPU_Read | EAH_GPU_Write))
		{
			D3D11_BOX box;
			box.left = x_offset;
			box.top = y_offset;
			box.front = z_offset;
			box.right = x_offset + width;
			box.bottom = y_offset + height;
			box.back = z_offset + depth;
			d3d_imm_ctx_->UpdateSubresource(d3d_texture_.get(), array_index * num_mip_maps_ + level, &box,
				data, row_pitch, slice_pitch);
		}
		else if (access_hint_ & EAH_CPU_Write)
		{
			Texture::Mapper mapper(*this, array_index, level, TMA_Write_Only, x_offset, y_offset, width, height);

			uint8_t const * src0 = static_cast<uint8_t const *>(data);
			uint8_t* dst0 = mapper.Pointer<uint8_t>();
			uint32_t const dst_row_pitch = mapper.RowPitch();
			uint32_t const dst_slice_pitch = mapper.SlicePitch();
			uint32_t const block_width = BlockWidth(format_);
			uint32_t const block_height = BlockHeight(format_);
			uint32_t const block_depth = BlockDepth(format_);
			uint32_t const block_bytes = BlockBytes(format_);
			uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
			for (uint32_t z = 0; z < depth; z += block_depth)
			{
				uint8_t const * src = src0;
				uint8_t* dst = dst0;

				for (uint32_t y = 0; y < height; y += block_height)
				{
					std::memcpy(dst, src, bytes_per_row);

					src += row_pitch;
					dst += dst_row_pitch;
				}

				src0 += slice_pitch;
				dst0 += dst_slice_pitch;
			}
		}
		else
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			ElementInitData init_data;
			init_data.data = data;
			init_data.row_pitch = row_pitch;
			init_data.slice_pitch = row_pitch * height;
			TexturePtr temp_tex = rf.MakeTexture3D(width, height, depth, 1, 1, format_, 1, 0, EAH_CPU_Write, MakeSpan<1>(init_data));
			d3d_imm_ctx_->CopySubresourceRegion(d3d_texture_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_),
				x_offset, y_offset, z_offset, checked_cast<D3D11Texture&>(*temp_tex).d3d_texture_.get(),
				D3D11CalcSubresource(0, 0, 1),
				nullptr);
		}
	}

	void D3D11Texture::UpdateSubresourceCube(uint32_t array_index, Texture::CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		if (access_hint_ & (EAH_GPU_Read | EAH_GPU_Write))
		{
			D3D11_BOX box;
			box.left = x_offset;
			box.top = y_offset;
			box.front = 0;
			box.right = x_offset + width;
			box.bottom = y_offset + height;
			box.back = 1;
			d3d_imm_ctx_->UpdateSubresource(d3d_texture_.get(), (array_index * 6 + face) * num_mip_maps_ + level, &box,
				data, row_pitch, row_pitch);
		}
		else if (access_hint_ & EAH_CPU_Write)
		{
			Texture::Mapper mapper(*this, array_index * 6 + face, level, TMA_Write_Only, x_offset, y_offset, width, height);

			uint8_t const * src = static_cast<uint8_t const *>(data);
			uint8_t* dst = mapper.Pointer<uint8_t>();
			uint32_t const dst_row_pitch = mapper.RowPitch();
			uint32_t const block_width = BlockWidth(format_);
			uint32_t const block_height = BlockHeight(format_);
			uint32_t const block_bytes = BlockBytes(format_);
			uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
			for (uint32_t y = 0; y < height; y += block_height)
			{
				std::memcpy(dst, src, bytes_per_row);

				src += row_pitch;
				dst += dst_row_pitch;
			}
		}
		else
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			ElementInitData init_data;
			init_data.data = data;
			init_data.row_pitch = row_pitch;
			init_data.slice_pitch = row_pitch * height;
			TexturePtr temp_tex = rf.MakeTexture2D(width, height, 1, 1, format_, 1, 0, EAH_CPU_Write, MakeSpan<1>(init_data));
			d3d_imm_ctx_->CopySubresourceRegion(d3d_texture_.get(), D3D11CalcSubresource(level, array_index * 6 + face, num_mip_maps_),
				x_offset, y_offset, 0, checked_cast<D3D11Texture&>(*temp_tex).d3d_texture_.get(), D3D11CalcSubresource(0, 0, 1),
				nullptr);
		}
	}
}
