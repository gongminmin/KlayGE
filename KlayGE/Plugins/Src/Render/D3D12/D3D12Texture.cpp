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
#include <KFL/COMPtr.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>
#include <boost/assert.hpp>
#include <boost/functional/hash.hpp>

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
	}

	D3D12Texture::~D3D12Texture()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.ForceCPUGPUSync();
		}
	}

	std::wstring const & D3D12Texture::Name() const
	{
		static const std::wstring name(L"Direct3D12 Texture");
		return name;
	}

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

	void D3D12Texture::CopyToSubTexture1D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_width*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_width*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::CopyToSubTexture2D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::CopyToSubTexture3D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_z_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_z_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::CopyToSubTextureCube(Texture& /*target*/,
			uint32_t /*dst_array_index*/, CubeFaces /*dst_face*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, CubeFaces /*src_face*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	D3D12ShaderResourceViewSimulationPtr const & D3D12Texture::RetriveD3DShaderResourceView(uint32_t /*first_array_index*/, uint32_t /*num_items*/, uint32_t /*first_level*/, uint32_t /*num_levels*/)
	{
		BOOST_ASSERT(false);
		static D3D12ShaderResourceViewSimulationPtr ret;
		return ret;
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetriveD3DUnorderedAccessView(uint32_t /*first_array_index*/, uint32_t /*num_items*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12UnorderedAccessViewSimulationPtr ret;
		return ret;
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetriveD3DUnorderedAccessView(uint32_t /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12UnorderedAccessViewSimulationPtr ret;
		return ret;
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetriveD3DUnorderedAccessView(uint32_t /*first_array_index*/, uint32_t /*num_items*/, CubeFaces /*first_face*/, uint32_t /*num_faces*/,
		uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12UnorderedAccessViewSimulationPtr ret;
		return ret;
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetriveD3DRenderTargetView(uint32_t /*first_array_index*/, uint32_t /*array_size*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12RenderTargetViewSimulationPtr ret;
		return ret;
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetriveD3DRenderTargetView(uint32_t /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12RenderTargetViewSimulationPtr ret;
		return ret;
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetriveD3DRenderTargetView(uint32_t /*array_index*/, Texture::CubeFaces /*face*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12RenderTargetViewSimulationPtr ret;
		return ret;
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetriveD3DDepthStencilView(uint32_t /*first_array_index*/, uint32_t /*array_size*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12DepthStencilViewSimulationPtr ret;
		return ret;
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetriveD3DDepthStencilView(uint32_t /*array_index*/, uint32_t /*first_slice*/, uint32_t /*num_slices*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12DepthStencilViewSimulationPtr ret;
		return ret;
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetriveD3DDepthStencilView(uint32_t /*array_index*/, Texture::CubeFaces /*face*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
		static D3D12DepthStencilViewSimulationPtr ret;
		return ret;
	}

	void D3D12Texture::Map1D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*width*/,
			void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::Map2D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::Map3D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::MapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::Unmap1D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::Unmap2D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::Unmap3D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::UnmapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12Texture::DeleteHWResource()
	{
		d3d_texture_.reset();
		d3d_texture_upload_heaps_.reset();
		d3d_texture_readback_heaps_.reset();
	}

	bool D3D12Texture::HWResourceReady() const
	{
		return d3d_texture_.get() ? true : false;
	}

	void D3D12Texture::DoHWCopyToTexture(Texture& target)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		if ((access_hint_ & EAH_CPU_Write) && (target.AccessHint() & EAH_GPU_Read))
		{
			re.ForceCPUGPUSync();
		}

		D3D12Texture& other = *checked_cast<D3D12Texture*>(&target);

		uint32_t const num_subres = array_size_ * num_mip_maps_;

		D3D12_RESOURCE_BARRIER barrier_before[2];
		barrier_before[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[0].Transition.pResource = d3d_texture_.get();
		barrier_before[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier_before[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier_before[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[1].Transition.pResource = other.D3DResource().get();
		barrier_before[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_before[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		cmd_list->ResourceBarrier(2, barrier_before);

		if ((this->SampleCount() > 1) && (1 == target.SampleCount()))
		{
			for (uint32_t i = 0; i < num_subres; ++ i)
			{
				cmd_list->ResolveSubresource(other.D3DResource().get(), i, d3d_texture_.get(), i, dxgi_fmt_);
			}
		}
		else
		{
			cmd_list->CopyResource(other.D3DResource().get(), d3d_texture_.get());
		}

		D3D12_RESOURCE_BARRIER barrier_after[2];
		barrier_after[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after[0].Transition.pResource = d3d_texture_.get();
		barrier_after[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier_after[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_after[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier_after[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after[1].Transition.pResource = other.D3DResource().get();
		barrier_after[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_after[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_after[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		cmd_list->ResourceBarrier(2, barrier_after);
	}

	void D3D12Texture::DoHWCopyToSubTexture(Texture& target,
			uint32_t dst_subres, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
			uint32_t src_subres, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
			uint32_t width, uint32_t height, uint32_t depth)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		if ((access_hint_ & EAH_CPU_Write) && (target.AccessHint() & EAH_GPU_Read))
		{
			re.ForceCPUGPUSync();
		}

		D3D12Texture& other = *checked_cast<D3D12Texture2D*>(&target);

		D3D12_RESOURCE_BARRIER barrier_before[2];
		barrier_before[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[0].Transition.pResource = d3d_texture_.get();
		barrier_before[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier_before[0].Transition.Subresource = src_subres;
		barrier_before[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[1].Transition.pResource = other.D3DResource().get();
		barrier_before[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_before[1].Transition.Subresource = dst_subres;

		cmd_list->ResourceBarrier(2, barrier_before);

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = d3d_texture_.get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = src_subres;

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = other.D3DResource().get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = dst_subres;

		D3D12_BOX src_box;
		src_box.left = src_x_offset;
		src_box.top = src_y_offset;
		src_box.front = src_z_offset;
		src_box.right = src_x_offset + width;
		src_box.bottom = src_y_offset + height;
		src_box.back = src_z_offset + depth;

		cmd_list->CopyTextureRegion(&dst, dst_x_offset, dst_y_offset, dst_z_offset, &src, &src_box);

		D3D12_RESOURCE_BARRIER barrier_after[2];
		barrier_after[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after[0].Transition.pResource = d3d_texture_.get();
		barrier_after[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier_after[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_after[0].Transition.Subresource = src_subres;
		barrier_after[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after[1].Transition.pResource = other.D3DResource().get();
		barrier_after[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_after[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_after[1].Transition.Subresource = dst_subres;

		cmd_list->ResourceBarrier(2, barrier_after);
	}

	void D3D12Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim,
			uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size,
			ElementInitData const * init_data)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

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
			BOOST_ASSERT(false);
			tex_desc.DepthOrArraySize = 0;
			break;
		}
		tex_desc.MipLevels = static_cast<UINT16>(num_mip_maps_);
		tex_desc.Format = dxgi_fmt_;
		tex_desc.SampleDesc.Count = 1;
		tex_desc.SampleDesc.Quality = 0;
		tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (access_hint_ & EAH_GPU_Write)
		{
			if (IsDepthFormat(format_))
			{
				tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			else
			{
				tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
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

		ID3D12Resource* d3d_texture;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&tex_desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&d3d_texture)));
		d3d_texture_ = MakeCOMPtr(d3d_texture);

		uint32_t const num_subres = array_size * num_mip_maps_;
		uint64_t upload_buffer_size = 0;
		device->GetCopyableFootprints(&tex_desc, 0, num_subres, 0, nullptr, nullptr, nullptr, &upload_buffer_size);

		D3D12_HEAP_PROPERTIES upload_heap_prop;
		upload_heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		upload_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		upload_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		upload_heap_prop.CreationNodeMask = 0;
		upload_heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC buff_desc;
		buff_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		buff_desc.Alignment = 0;
		buff_desc.Width = upload_buffer_size;
		buff_desc.Height = 1;
		buff_desc.DepthOrArraySize = 1;
		buff_desc.MipLevels = 1;
		buff_desc.Format = DXGI_FORMAT_UNKNOWN;
		buff_desc.SampleDesc.Count = 1;
		buff_desc.SampleDesc.Quality = 0;
		buff_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		buff_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* d3d_texture_upload_heaps;
		TIF(device->CreateCommittedResource(&upload_heap_prop, D3D12_HEAP_FLAG_NONE, &buff_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&d3d_texture_upload_heaps)));
		d3d_texture_upload_heaps_ = MakeCOMPtr(d3d_texture_upload_heaps);

		D3D12_HEAP_PROPERTIES readback_heap_prop;
		readback_heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		readback_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		readback_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		readback_heap_prop.CreationNodeMask = 0;
		readback_heap_prop.VisibleNodeMask = 0;

		ID3D12Resource* d3d_texture_readback_heaps;
		TIF(device->CreateCommittedResource(&readback_heap_prop, D3D12_HEAP_FLAG_NONE, &buff_desc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&d3d_texture_readback_heaps)));
		d3d_texture_readback_heaps_ = MakeCOMPtr(d3d_texture_readback_heaps);

		if (init_data != nullptr)
		{
			ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DResCmdList();
			std::lock_guard<std::mutex> lock(re.D3DResCmdListMutex());

			D3D12_RESOURCE_BARRIER barrier_before;
			barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_before.Transition.pResource = d3d_texture_.get();
			barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmd_list->ResourceBarrier(1, &barrier_before);

			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(num_subres);
			std::vector<uint64_t> row_sizes_in_bytes(num_subres);
			std::vector<uint32_t> num_rows(num_subres);

			uint64_t required_size = 0;
			device->GetCopyableFootprints(&tex_desc, 0, num_subres, 0, &layouts[0], &num_rows[0], &row_sizes_in_bytes[0], &required_size);

			uint8_t* p;
			d3d_texture_upload_heaps_->Map(0, nullptr, reinterpret_cast<void**>(&p));
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
					uint8_t const * src_slice
						= reinterpret_cast<uint8_t const *>(src_data.pData) + src_data.SlicePitch * z;
					uint8_t* dest_slice = reinterpret_cast<uint8_t*>(dest_data.pData) + dest_data.SlicePitch * z;
					for (UINT y = 0; y < num_rows[i]; ++ y)
					{
						memcpy(dest_slice + dest_data.RowPitch * y, src_slice + src_data.RowPitch * y,
							static_cast<size_t>(row_sizes_in_bytes[i]));
					}
				}
			}
			d3d_texture_upload_heaps_->Unmap(0, nullptr);

			for (uint32_t i = 0; i < num_subres; ++ i)
			{
				D3D12_TEXTURE_COPY_LOCATION src;
				src.pResource = d3d_texture_upload_heaps_.get();
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src.PlacedFootprint = layouts[i];

				D3D12_TEXTURE_COPY_LOCATION dst;
				dst.pResource = d3d_texture_.get();
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = i;

				cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
			}

			D3D12_RESOURCE_BARRIER barrier_after;
			barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after.Transition.pResource = d3d_texture_.get();
			barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier_after.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			barrier_after.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmd_list->ResourceBarrier(1, &barrier_after);

			re.CommitResCmd();
		}
	}

	void D3D12Texture::DoMap(uint32_t subres, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);

		last_tma_ = tma;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint64_t row_sizes_in_bytes;
		uint32_t num_rows;

		D3D12_RESOURCE_DESC const tex_desc = d3d_texture_->GetDesc();
		uint64_t required_size = 0;
		device->GetCopyableFootprints(&tex_desc, subres, 1, 0, &layout, &num_rows, &row_sizes_in_bytes, &required_size);

		if ((TMA_Read_Only == tma) || (TMA_Read_Write == tma))
		{
			ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

			D3D12_RESOURCE_BARRIER barrier_before;
			barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_before.Transition.pResource = d3d_texture_.get();
			barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier_before.Transition.Subresource = subres;

			cmd_list->ResourceBarrier(1, &barrier_before);

			D3D12_TEXTURE_COPY_LOCATION src;
			src.pResource = d3d_texture_.get();
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			src.SubresourceIndex = subres;

			D3D12_TEXTURE_COPY_LOCATION dst;
			dst.pResource = d3d_texture_readback_heaps_.get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dst.PlacedFootprint = layout;

			cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

			D3D12_RESOURCE_BARRIER barrier_after;
			barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after.Transition.pResource = d3d_texture_.get();
			barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier_after.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			barrier_after.Transition.Subresource = subres;

			cmd_list->ResourceBarrier(1, &barrier_after);

			re.ForceCPUGPUSync();
		}

		uint8_t* p;
		d3d_texture_upload_heaps_->Map(0, nullptr, reinterpret_cast<void**>(&p));

		data = p + layout.Offset + (z_offset * layout.Footprint.Height + y_offset) * layout.Footprint.RowPitch
			+ x_offset * NumFormatBytes(format_);
		row_pitch = layout.Footprint.RowPitch;
		slice_pitch = layout.Footprint.RowPitch * layout.Footprint.Height;

		if ((TMA_Read_Only == tma) || (TMA_Read_Write == tma))
		{
			d3d_texture_readback_heaps_->Map(0, nullptr, reinterpret_cast<void**>(&p));
			uint8_t* src_p = p + layout.Offset + (z_offset * layout.Footprint.Height + y_offset) * layout.Footprint.RowPitch
				+ x_offset * NumFormatBytes(format_);
			uint8_t* dst_p = static_cast<uint8_t*>(data);
			for (uint32_t z = 0; z < depth; ++ z)
			{
				memcpy(dst_p + z * slice_pitch, src_p + z * slice_pitch, row_pitch * height);
			}
			d3d_texture_readback_heaps_->Unmap(0, nullptr);
		}
	}

	void D3D12Texture::DoUnmap(uint32_t subres)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		d3d_texture_upload_heaps_->Unmap(0, nullptr);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		uint64_t row_sizes_in_bytes;
		uint32_t num_rows;

		D3D12_RESOURCE_DESC const tex_desc = d3d_texture_->GetDesc();
		uint64_t required_size = 0;
		device->GetCopyableFootprints(&tex_desc, subres, 1, 0, &layout, &num_rows, &row_sizes_in_bytes, &required_size);

		if ((TMA_Write_Only == last_tma_) || (TMA_Read_Write == last_tma_))
		{
			ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

			re.ForceCPUGPUSync();

			D3D12_RESOURCE_BARRIER barrier_before;
			barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_before.Transition.pResource = d3d_texture_.get();
			barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier_before.Transition.Subresource = subres;

			cmd_list->ResourceBarrier(1, &barrier_before);

			D3D12_TEXTURE_COPY_LOCATION src;
			src.pResource = d3d_texture_upload_heaps_.get();
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = layout;

			D3D12_TEXTURE_COPY_LOCATION dst;
			dst.pResource = d3d_texture_.get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = subres;

			cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

			D3D12_RESOURCE_BARRIER barrier_after;
			barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after.Transition.pResource = d3d_texture_.get();
			barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier_after.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			barrier_after.Transition.Subresource = subres;

			cmd_list->ResourceBarrier(1, &barrier_after);
		}
	}

	D3D12ShaderResourceViewSimulationPtr const & D3D12Texture::RetriveD3DSRV(D3D12_SHADER_RESOURCE_VIEW_DESC const & desc)
	{
		if (this->HWResourceReady())
		{
			char const * p = reinterpret_cast<char const *>(&desc);
			size_t hash_val = 0;
			boost::hash_range(hash_val, p, p + sizeof(desc));

			auto iter = d3d_sr_views_.find(hash_val);
			if (iter != d3d_sr_views_.end())
			{
				return iter->second;
			}

			D3D12ShaderResourceViewSimulationPtr sr_view = MakeSharedPtr<D3D12ShaderResourceViewSimulation>(d3d_texture_, desc);
			return d3d_sr_views_.emplace(hash_val, sr_view).first->second;
		}
		else
		{
			static D3D12ShaderResourceViewSimulationPtr view;
			return view;
		}
	}

	D3D12UnorderedAccessViewSimulationPtr const & D3D12Texture::RetriveD3DUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC const & desc)
	{
		if (this->HWResourceReady())
		{
			char const * p = reinterpret_cast<char const *>(&desc);
			size_t hash_val = 0;
			boost::hash_range(hash_val, p, p + sizeof(desc));

			auto iter = d3d_ua_views_.find(hash_val);
			if (iter != d3d_ua_views_.end())
			{
				return iter->second;
			}

			D3D12UnorderedAccessViewSimulationPtr ua_view = MakeSharedPtr<D3D12UnorderedAccessViewSimulation>(d3d_texture_, desc);
			return d3d_ua_views_.emplace(hash_val, ua_view).first->second;
		}
		else
		{
			static D3D12UnorderedAccessViewSimulationPtr view;
			return view;
		}
	}

	D3D12RenderTargetViewSimulationPtr const & D3D12Texture::RetriveD3DRTV(D3D12_RENDER_TARGET_VIEW_DESC const & desc)
	{
		if (this->HWResourceReady())
		{
			char const * p = reinterpret_cast<char const *>(&desc);
			size_t hash_val = 0;
			boost::hash_range(hash_val, p, p + sizeof(desc));

			auto iter = d3d_rt_views_.find(hash_val);
			if (iter != d3d_rt_views_.end())
			{
				return iter->second;
			}

			D3D12RenderTargetViewSimulationPtr rt_view = MakeSharedPtr<D3D12RenderTargetViewSimulation>(d3d_texture_, desc);
			return d3d_rt_views_.emplace(hash_val, rt_view).first->second;
		}
		else
		{
			static D3D12RenderTargetViewSimulationPtr view;
			return view;
		}
	}

	D3D12DepthStencilViewSimulationPtr const & D3D12Texture::RetriveD3DDSV(D3D12_DEPTH_STENCIL_VIEW_DESC const & desc)
	{
		if (this->HWResourceReady())
		{
			char const * p = reinterpret_cast<char const *>(&desc);
			size_t hash_val = 0;
			boost::hash_range(hash_val, p, p + sizeof(desc));

			auto iter = d3d_ds_views_.find(hash_val);
			if (iter != d3d_ds_views_.end())
			{
				return iter->second;
			}

			D3D12DepthStencilViewSimulationPtr ds_view = MakeSharedPtr<D3D12DepthStencilViewSimulation>(d3d_texture_, desc);
			return d3d_ds_views_.emplace(hash_val, ds_view).first->second;
		}
		else
		{
			static D3D12DepthStencilViewSimulationPtr view;
			return view;
		}
	}
}
