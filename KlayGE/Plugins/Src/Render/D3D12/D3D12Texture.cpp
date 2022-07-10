/**
 * @file D3D12Texture.cpp
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
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/TexCompression.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Hash.hpp>

#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>

namespace KlayGE
{
	D3D12Texture::D3D12Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint)
	{
		if (access_hint & EAH_GPU_Write)
		{
			BOOST_ASSERT(!(access_hint & EAH_CPU_Read));
			BOOST_ASSERT(!(access_hint & EAH_CPU_Write));
		}

		if (!IsSRGB(format_) && (type == TT_2D) && (access_hint_ & EAH_Generate_Mips))
		{
			access_hint_ |= EAH_GPU_Unordered;
		}
	}

	std::wstring const & D3D12Texture::Name() const
	{
		static const std::wstring name(L"Direct3D12 Texture");
		return name;
	}

#ifndef KLAYGE_SHIP
	void D3D12Texture::DebugName(std::wstring_view name)
	{
		d3d_resource_->SetPrivateData(WKPDID_D3DDebugObjectNameW, static_cast<uint32_t>(name.size() * sizeof(wchar_t)), name.data());
	}
#endif

	uint32_t D3D12Texture::Width(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t D3D12Texture::Height(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t D3D12Texture::Depth(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void D3D12Texture::CopyToSubTexture1D(Texture& /*target*/, uint32_t /*dst_array_index*/, uint32_t /*dst_level*/,
		uint32_t /*dst_x_offset*/, uint32_t /*dst_width*/, uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/,
		uint32_t /*src_width*/, TextureFilter /*filter*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::CopyToSubTexture2D(Texture& /*target*/, uint32_t /*dst_array_index*/, uint32_t /*dst_level*/,
		uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*src_array_index*/,
		uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/,
		TextureFilter /*filter*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::CopyToSubTexture3D(Texture& /*target*/, uint32_t /*dst_array_index*/, uint32_t /*dst_level*/,
		uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_z_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
		uint32_t /*dst_depth*/, uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/,
		uint32_t /*src_z_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/, TextureFilter /*filter*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::CopyToSubTextureCube(Texture& /*target*/, uint32_t /*dst_array_index*/, CubeFaces /*dst_face*/,
		uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
		uint32_t /*src_array_index*/, CubeFaces /*src_face*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/,
		uint32_t /*src_width*/, uint32_t /*src_height*/, TextureFilter /*filter*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	D3D12ShaderResourceViewSimulationPtr const & D3D12Texture::RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_array_index,
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
			auto sr_view = MakeSharedPtr<D3D12ShaderResourceViewSimulation>(this, desc);
			return d3d_sr_views_.emplace(hash_val, sr_view).first->second;
		}
	}

	D3D12ShaderResourceViewSimulationPtr const& D3D12Texture::RetrieveD3DShaderResourceView(
		ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Read);

		size_t hash_val = HashValue(pf);
		HashCombine(hash_val, array_index);
		HashCombine(hash_val, 1);
		HashCombine(hash_val, face);
		HashCombine(hash_val, first_level);
		HashCombine(hash_val, num_levels);

		auto iter = d3d_sr_views_.find(hash_val);
		if (iter != d3d_sr_views_.end())
		{
			return iter->second;
		}
		else
		{
			auto desc = this->FillSRVDesc(pf, array_index, face, first_level, num_levels);
			auto sr_view = MakeSharedPtr<D3D12ShaderResourceViewSimulation>(this, desc);
			return d3d_sr_views_.emplace(hash_val, sr_view).first->second;
		}
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_array_index,
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
			auto rt_view = MakeSharedPtr<D3D12RenderTargetViewSimulation>(this, desc);
			return d3d_rt_views_.emplace(hash_val, rt_view).first->second;
		}
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index,
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
			auto rt_view = MakeSharedPtr<D3D12RenderTargetViewSimulation>(this, desc);
			return d3d_rt_views_.emplace(hash_val, rt_view).first->second;
		}
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index,
		CubeFaces face, uint32_t level)
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
			auto rt_view = MakeSharedPtr<D3D12RenderTargetViewSimulation>(this, desc);
			return d3d_rt_views_.emplace(hash_val, rt_view).first->second;
		}
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t first_array_index,
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
			auto ds_view = MakeSharedPtr<D3D12DepthStencilViewSimulation>(this, desc);
			return d3d_ds_views_.emplace(hash_val, ds_view).first->second;
		}
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index,
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
			auto ds_view = MakeSharedPtr<D3D12DepthStencilViewSimulation>(this, desc);
			return d3d_ds_views_.emplace(hash_val, ds_view).first->second;
		}
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index,
		CubeFaces face, uint32_t level)
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
			auto ds_view = MakeSharedPtr<D3D12DepthStencilViewSimulation>(this, desc);
			return d3d_ds_views_.emplace(hash_val, ds_view).first->second;
		}
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_array_index,
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
			auto ua_view = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(this, desc);
			return d3d_ua_views_.emplace(hash_val, ua_view).first->second;
		}
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t array_index,
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
			auto ua_view = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(this, desc);
			return d3d_ua_views_.emplace(hash_val, ua_view).first->second;
		}
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_array_index,
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
			auto ua_view = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(this, desc);
			return d3d_ua_views_.emplace(hash_val, ua_view).first->second;
		}
	}

	void D3D12Texture::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t width,
		void*& data)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(data);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(z_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
		KFL_UNUSED(slice_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::Unmap1D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::Unmap2D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::Unmap3D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void D3D12Texture::DeleteHWResource()
	{
		d3d_sr_views_.clear();
		d3d_rt_views_.clear();
		d3d_ds_views_.clear();
		d3d_ua_views_.clear();

		d3d_resource_.reset();
		mapped_mem_block_.Reset();
	}

	bool D3D12Texture::HWResourceReady() const
	{
		return d3d_resource_.get() ? true : false;
	}

	void D3D12Texture::DoHWCopyToTexture(Texture& target)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		D3D12Texture& other = checked_cast<D3D12Texture&>(target);

		uint32_t const num_subres = array_size_ * num_mip_maps_;
		bool const need_resolve = (this->SampleCount() > 1) && (1 == target.SampleCount());

		this->UpdateResourceBarrier(cmd_list, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			need_resolve ? D3D12_RESOURCE_STATE_RESOLVE_SOURCE : D3D12_RESOURCE_STATE_GENERIC_READ);
		other.UpdateResourceBarrier(cmd_list, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			need_resolve ? D3D12_RESOURCE_STATE_RESOLVE_DEST : D3D12_RESOURCE_STATE_COPY_DEST);
		re.FlushResourceBarriers(cmd_list);

		if (need_resolve)
		{
			for (uint32_t i = 0; i < num_subres; ++ i)
			{
				cmd_list->ResolveSubresource(other.D3DResource(), i, d3d_resource_.get(), i, dxgi_fmt_);
			}
		}
		else
		{
			cmd_list->CopyResource(other.D3DResource(), d3d_resource_.get());
		}
	}

	void D3D12Texture::DoHWCopyToSubTexture(Texture& target,
			uint32_t dst_subres, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
			uint32_t src_subres, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
			uint32_t width, uint32_t height, uint32_t depth)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		D3D12Texture& other = checked_cast<D3D12Texture2D&>(target);

		this->UpdateResourceBarrier(cmd_list, src_subres, D3D12_RESOURCE_STATE_GENERIC_READ);
		other.UpdateResourceBarrier(cmd_list, dst_subres, D3D12_RESOURCE_STATE_COPY_DEST);
		re.FlushResourceBarriers(cmd_list);

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = d3d_resource_.get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = src_subres;

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = other.D3DResource();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = dst_subres;

		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);

		D3D12_BOX src_box;
		src_box.left = src_x_offset;
		src_box.top = src_y_offset;
		src_box.front = src_z_offset;
		src_box.right = src_x_offset + ((width + block_width - 1) & ~(block_width - 1));
		src_box.bottom = src_y_offset + ((height + block_height - 1) & ~(block_height - 1));
		src_box.back = src_z_offset + depth;

		cmd_list->CopyTextureRegion(&dst, dst_x_offset, dst_y_offset, dst_z_offset, &src, &src_box);
	}

	void D3D12Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim,
			uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size,
			std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		D3D12_CLEAR_VALUE clear_value;

		D3D12_RESOURCE_DESC tex_desc;
		tex_desc.Dimension = dim;
		tex_desc.Alignment = 0;
		tex_desc.Width = width;
		tex_desc.Height = height;
		switch (dim)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			tex_desc.DepthOrArraySize = static_cast<UINT16>(array_size);
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			tex_desc.DepthOrArraySize = static_cast<UINT16>(depth);
			break;

		default:
			KFL_UNREACHABLE("Invalid resource dimension");
		}
		tex_desc.MipLevels = static_cast<UINT16>(num_mip_maps_);
		tex_desc.Format = dxgi_fmt_;
		tex_desc.SampleDesc.Count = sample_count_;
		tex_desc.SampleDesc.Quality = sample_quality_;
		tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (access_hint_ & EAH_GPU_Write)
		{
			if (IsDepthFormat(format_))
			{
				tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				switch (format_)
				{
				case EF_D16:
					clear_value.Format = DXGI_FORMAT_D16_UNORM;
					break;

				case EF_D24S8:
					clear_value.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					break;

				case EF_D32F:
					clear_value.Format = DXGI_FORMAT_D32_FLOAT;
					break;

				default:
					KFL_UNREACHABLE("Invalid depth format");
				}
				if (clear_value_hint != nullptr)
				{
					clear_value.DepthStencil.Depth = (*clear_value_hint)[0];
					clear_value.DepthStencil.Stencil = static_cast<UINT8>((*clear_value_hint)[1] + 0.5f);
				}
				else
				{
					clear_value.DepthStencil.Depth = 1.0f;
					clear_value.DepthStencil.Stencil = 0;
				}
			}
			else
			{
				tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

				clear_value.Format = dxgi_fmt_;
				if (clear_value_hint != nullptr)
				{
					clear_value.Color[0] = (*clear_value_hint)[0];
					clear_value.Color[1] = (*clear_value_hint)[1];
					clear_value.Color[2] = (*clear_value_hint)[2];
					clear_value.Color[3] = (*clear_value_hint)[3];
				}
				else
				{
					clear_value.Color[0] = clear_value.Color[1] = clear_value.Color[2] = clear_value.Color[3] = 0;
				}
			}
		}
		if (access_hint_ & EAH_GPU_Unordered)
		{
			tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_STATES init_state = D3D12_RESOURCE_STATE_COMMON;
		if (IsDepthFormat(format_) && (access_hint_ & EAH_GPU_Write))
		{
			init_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			std::fill(curr_states_.begin(), curr_states_.end(), init_state);
		}

		TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&tex_desc, init_state, (access_hint_ & EAH_GPU_Write) ? &clear_value : nullptr,
			UuidOf<ID3D12Resource>(), d3d_resource_.put_void()));

		if (!init_data.empty())
		{
			uint32_t const num_subres = array_size * num_mip_maps_;
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(num_subres);
			std::vector<uint64_t> row_sizes_in_bytes(num_subres);
			std::vector<uint32_t> num_rows(num_subres);

			uint64_t required_size = 0;
			device->GetCopyableFootprints(&tex_desc, 0, num_subres, 0, &layouts[0], &num_rows[0], &row_sizes_in_bytes[0], &required_size);

			auto upload_mem_block =
				re.AllocUploadMemBlock(static_cast<uint32_t>(required_size), D3D12GpuMemoryAllocator::TextureDataAligment);
			auto const& upload_buff = upload_mem_block.Resource();
			uint32_t const upload_buff_offset = upload_mem_block.Offset();

			uint8_t* p = upload_mem_block.CpuAddress<uint8_t>();
			for (uint32_t i = 0; i < num_subres; ++ i)
			{
				D3D12_SUBRESOURCE_DATA src_data;
				src_data.pData = init_data[i].data;
				src_data.RowPitch = init_data[i].row_pitch;
				src_data.SlicePitch = init_data[i].slice_pitch;

				D3D12_MEMCPY_DEST dest_data;
				dest_data.pData = p + layouts[i].Offset;
				dest_data.RowPitch = layouts[i].Footprint.RowPitch;
				dest_data.SlicePitch = layouts[i].Footprint.RowPitch * num_rows[i];

				for (UINT z = 0; z < layouts[i].Footprint.Depth; ++ z)
				{
					uint8_t const * src_slice = static_cast<uint8_t const *>(src_data.pData) + src_data.SlicePitch * z;
					uint8_t* dest_slice = static_cast<uint8_t*>(dest_data.pData) + dest_data.SlicePitch * z;
					for (UINT y = 0; y < num_rows[i]; ++ y)
					{
						memcpy(dest_slice, src_slice, static_cast<size_t>(row_sizes_in_bytes[i]));

						src_slice += src_data.RowPitch;
						dest_slice += dest_data.RowPitch;
					}
				}
			}

			{
				re.ResetLoadCmd();
				ID3D12GraphicsCommandList* cmd_list = re.D3DLoadCmdList();

				this->UpdateResourceBarrier(cmd_list, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST);
				re.FlushResourceBarriers(cmd_list);

				for (uint32_t i = 0; i < num_subres; ++i)
				{
					layouts[i].Offset += upload_buff_offset;

					D3D12_TEXTURE_COPY_LOCATION src;
					src.pResource = upload_buff;
					src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					src.PlacedFootprint = layouts[i];

					D3D12_TEXTURE_COPY_LOCATION dst;
					dst.pResource = d3d_resource_.get();
					dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dst.SubresourceIndex = i;

					cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				}

				re.CommitLoadCmd();
			}

			re.DeallocUploadMemBlock(std::move(upload_mem_block));
		}
	}

	void D3D12Texture::DoMap(uint32_t subres, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);	
		width = (width + block_width - 1) & ~(block_width - 1);
		height = (height + block_height - 1) & ~(block_height - 1);

		mapped_tma_ = tma;
		mapped_x_offset_ = x_offset;
		mapped_y_offset_ = y_offset;
		mapped_z_offset_ = z_offset;
		mapped_width_ = width;
		mapped_height_ = height;
		mapped_depth_ = depth;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint32_t row_size_in_bytes = 0;
		uint32_t num_row = 0;
		uint32_t required_size = 0;
		this->GetCopyableFootprints(width, height, depth, layout, num_row, row_size_in_bytes, required_size);

		D3D12GpuMemoryBlock readback_mem_block;
		if ((TMA_Read_Only == tma) || (TMA_Read_Write == tma))
		{
			readback_mem_block =
				re.AllocReadbackMemBlock(static_cast<uint32_t>(required_size), D3D12GpuMemoryAllocator::TextureDataAligment);

			ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

			this->UpdateResourceBarrier(cmd_list, subres, D3D12_RESOURCE_STATE_GENERIC_READ);
			re.FlushResourceBarriers(cmd_list);

			D3D12_TEXTURE_COPY_LOCATION src;
			src.pResource = d3d_resource_.get();
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			src.SubresourceIndex = subres;

			layout.Offset += readback_mem_block.Offset();
			D3D12_TEXTURE_COPY_LOCATION dst;
			dst.pResource = readback_mem_block.Resource();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dst.PlacedFootprint = layout;

			D3D12_BOX src_box;
			src_box.left = x_offset;
			src_box.top = y_offset;
			src_box.front = z_offset;
			src_box.right = x_offset + width;
			src_box.bottom = y_offset + height;
			src_box.back = z_offset + depth;

			cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, &src_box);

			re.ForceFinish();
		}

		void* p;
		switch (tma)
		{
		case TMA_Read_Only:
			p = readback_mem_block.CpuAddress();
			mapped_mem_block_ = std::move(readback_mem_block);
			break;

		case TMA_Read_Write:
		case TMA_Write_Only:
			mapped_mem_block_ = re.AllocUploadMemBlock(static_cast<uint32_t>(required_size), D3D12GpuMemoryAllocator::TextureDataAligment);
			p = mapped_mem_block_.CpuAddress();
			if (TMA_Read_Write == tma)
			{
				memcpy(p, readback_mem_block.CpuAddress(), required_size);
				re.DeallocReadbackMemBlock(std::move(readback_mem_block));
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid buffer access mode");
		}

		data = p;
		row_pitch = layout.Footprint.RowPitch;
		slice_pitch = layout.Footprint.RowPitch * layout.Footprint.Height;
	}

	void D3D12Texture::DoUnmap(uint32_t subres)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		if ((TMA_Write_Only == mapped_tma_) || (TMA_Read_Write == mapped_tma_))
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
			uint32_t row_size_in_bytes = 0;
			uint32_t num_row = 0;
			uint32_t required_size = 0;
			this->GetCopyableFootprints(mapped_width_, mapped_height_, mapped_depth_, layout, num_row, row_size_in_bytes, required_size);

			ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

			this->UpdateResourceBarrier(cmd_list, subres, D3D12_RESOURCE_STATE_COPY_DEST);
			re.FlushResourceBarriers(cmd_list);

			layout.Offset += mapped_mem_block_.Offset();
			D3D12_TEXTURE_COPY_LOCATION src;
			src.pResource = mapped_mem_block_.Resource();
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = layout;

			D3D12_TEXTURE_COPY_LOCATION dst;
			dst.pResource = d3d_resource_.get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = subres;

			D3D12_BOX src_box;
			src_box.left = 0;
			src_box.top = 0;
			src_box.front = 0;
			src_box.right = mapped_width_;
			src_box.bottom = mapped_height_;
			src_box.back = mapped_depth_;

			cmd_list->CopyTextureRegion(&dst, mapped_x_offset_, mapped_y_offset_, mapped_z_offset_, &src, &src_box);
		}

		if (mapped_tma_ == TMA_Read_Only)
		{
			re.DeallocReadbackMemBlock(std::move(mapped_mem_block_));
		}
		else
		{
			re.DeallocUploadMemBlock(std::move(mapped_mem_block_));
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC D3D12Texture::FillSRVDesc(
		ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(first_level);
		KFL_UNUSED(num_levels);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC D3D12Texture::FillSRVDesc(
		ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t first_level, uint32_t num_levels) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(first_level);
		KFL_UNUSED(num_levels);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12Texture::FillRTVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12Texture::FillRTVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12Texture::FillRTVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12Texture::FillDSVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12Texture::FillDSVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12Texture::FillDSVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture::FillUAVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		KFL_UNUSED(pf);
		KFL_UNUSED(array_index);
		KFL_UNUSED(first_slice);
		KFL_UNUSED(num_slices);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
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
	
	void D3D12Texture::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		width = (width + block_width - 1) & ~(block_width - 1);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint32_t row_size_in_bytes = 0;
		uint32_t num_row = 0;
		uint32_t total_bytes = 0;
		this->GetCopyableFootprints(width, block_height, 1, layout, num_row, row_size_in_bytes, total_bytes);

		void* mapped_data;
		this->Map1D(array_index, level, TMA_Write_Only, x_offset, width, mapped_data);

		uint8_t const * src_data = static_cast<uint8_t const *>(data);
		uint8_t* dst_data = static_cast<uint8_t*>(mapped_data);
		memcpy(dst_data, src_data, row_size_in_bytes);

		this->Unmap1D(array_index, level);
	}

	void D3D12Texture::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		width = (width + block_width - 1) & ~(block_width - 1);
		height = (height + block_height - 1) & ~(block_height - 1);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint32_t row_size_in_bytes = 0;
		uint32_t num_row = 0;
		uint32_t total_bytes = 0;
		this->GetCopyableFootprints(width, height, 1, layout, num_row, row_size_in_bytes, total_bytes);

		void* mapped_data;
		uint32_t mapped_row_pitch;
		this->Map2D(array_index, level, TMA_Write_Only, x_offset, y_offset, width, height, mapped_data, mapped_row_pitch);

		uint8_t const * src_data = static_cast<uint8_t const *>(data);
		uint8_t* dst_data = static_cast<uint8_t*>(mapped_data);
		for (uint32_t y = 0; y < num_row; ++ y)
		{
			memcpy(dst_data, src_data, row_size_in_bytes);

			src_data += row_pitch;
			dst_data += mapped_row_pitch;
		}

		this->Unmap2D(array_index, level);
	}

	void D3D12Texture::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		width = (width + block_width - 1) & ~(block_width - 1);
		height = (height + block_height - 1) & ~(block_height - 1);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint32_t row_size_in_bytes = 0;
		uint32_t num_row = 0;
		uint32_t total_bytes = 0;
		this->GetCopyableFootprints(width, height, depth, layout, num_row, row_size_in_bytes, total_bytes);

		void* mapped_data;
		uint32_t mapped_row_pitch;
		uint32_t mapped_slice_pitch;
		this->Map3D(array_index, level, TMA_Write_Only, x_offset, y_offset, z_offset, width, height, depth,
			mapped_data, mapped_row_pitch, mapped_slice_pitch);

		for (uint32_t z = 0; z < depth; ++ z)
		{
			uint8_t const * src_data = static_cast<uint8_t const *>(data) + z * slice_pitch;
			uint8_t* dst_data = static_cast<uint8_t*>(mapped_data) + z * mapped_slice_pitch;
			for (uint32_t y = 0; y < num_row; ++ y)
			{
				memcpy(dst_data, src_data, row_size_in_bytes);

				src_data += row_pitch;
				dst_data += mapped_row_pitch;
			}
		}

		this->Unmap3D(array_index, level);
	}

	void D3D12Texture::UpdateSubresourceCube(uint32_t array_index, Texture::CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		width = (width + block_width - 1) & ~(block_width - 1);
		height = (height + block_height - 1) & ~(block_height - 1);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint32_t row_size_in_bytes = 0;
		uint32_t num_row = 0;
		uint32_t total_bytes = 0;
		this->GetCopyableFootprints(width, height, 1, layout, num_row, row_size_in_bytes, total_bytes);

		void* mapped_data;
		uint32_t mapped_row_pitch;
		this->MapCube(array_index, face, level, TMA_Write_Only, x_offset, y_offset, width, height, mapped_data, mapped_row_pitch);

		uint8_t const * src_data = static_cast<uint8_t const *>(data);
		uint8_t* dst_data = static_cast<uint8_t*>(mapped_data);
		for (uint32_t y = 0; y < num_row; ++ y)
		{
			memcpy(dst_data, src_data, row_size_in_bytes);

			src_data += row_pitch;
			dst_data += mapped_row_pitch;
		}

		this->UnmapCube(array_index, face, level);
	}

	void D3D12Texture::GetCopyableFootprints(uint32_t width, uint32_t height, uint32_t depth,
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout,
		uint32_t& num_row, uint32_t& row_size_in_bytes,
		uint32_t& total_bytes)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* device = re.D3DDevice();

		D3D12_RESOURCE_DESC tex_desc;
		tex_desc.Dimension = (type_ == Texture::TT_2D) ? D3D12_RESOURCE_DIMENSION_TEXTURE2D : D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		tex_desc.Alignment = 0;
		tex_desc.Width = width;
		tex_desc.Height = height;
		tex_desc.DepthOrArraySize = static_cast<uint16_t>(depth);
		tex_desc.MipLevels = 1;
		tex_desc.Format = dxgi_fmt_;
		tex_desc.SampleDesc.Count = 1;
		tex_desc.SampleDesc.Quality = 0;
		tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		uint64_t row_size_in_bytes_64 = 0;
		num_row = 0;
		uint64_t total_bytes_64 = 0;
		device->GetCopyableFootprints(&tex_desc, 0, 1, 0, &layout, &num_row, &row_size_in_bytes_64, &total_bytes_64);

		row_size_in_bytes = static_cast<uint32_t>(row_size_in_bytes_64);
		total_bytes = static_cast<uint32_t>(total_bytes_64);
	}
}
