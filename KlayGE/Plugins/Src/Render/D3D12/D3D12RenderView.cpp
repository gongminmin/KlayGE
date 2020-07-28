/**
 * @file D3D12RenderView.cpp
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
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>

namespace KlayGE
{
	D3D12ShaderResourceViewSimulation::D3D12ShaderResourceViewSimulation(D3D12Resource const * res,
			D3D12_SHADER_RESOURCE_VIEW_DESC const & srv_desc)
		: res_(res)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		handle_ = re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocCBVSRVUAV();
		device->CreateShaderResourceView(res_->D3DResource(), &srv_desc, handle_);
	}

	D3D12ShaderResourceViewSimulation::~D3D12ShaderResourceViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocCBVSRVUAV(static_cast<uint32_t>(handle_.ptr - re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12RenderTargetViewSimulation::D3D12RenderTargetViewSimulation(D3D12Resource const * res,
			D3D12_RENDER_TARGET_VIEW_DESC const & rtv_desc)
		: res_(res)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		handle_ = re.RTVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocRTV();
		device->CreateRenderTargetView(res_->D3DResource(), &rtv_desc, handle_);
	}

	D3D12RenderTargetViewSimulation::~D3D12RenderTargetViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocRTV(static_cast<uint32_t>(handle_.ptr - re.RTVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12DepthStencilViewSimulation::D3D12DepthStencilViewSimulation(D3D12Resource const * res,
		D3D12_DEPTH_STENCIL_VIEW_DESC const & dsv_desc)
		: res_(res)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		handle_ = re.DSVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocDSV();
		device->CreateDepthStencilView(res_->D3DResource(), &dsv_desc, handle_);
	}

	D3D12DepthStencilViewSimulation::~D3D12DepthStencilViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocDSV(static_cast<uint32_t>(handle_.ptr - re.DSVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12UnorderedAccessViewSimulation::D3D12UnorderedAccessViewSimulation(D3D12Resource const * res,
			D3D12_UNORDERED_ACCESS_VIEW_DESC const & uav_desc)
		: res_(res), counter_offset_(0)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		ID3D12Resource* counter = nullptr;
		if (D3D12_UAV_DIMENSION_BUFFER == uav_desc.ViewDimension)
		{
			counter_offset_ = static_cast<uint32_t>(uav_desc.Buffer.CounterOffsetInBytes);
			if (counter_offset_ != 0)
			{
				counter = res_->D3DResource();
			}
		}

		handle_ = re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocCBVSRVUAV();
		device->CreateUnorderedAccessView(res_->D3DResource(), counter, &uav_desc, handle_);
	}

	D3D12UnorderedAccessViewSimulation::~D3D12UnorderedAccessViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocCBVSRVUAV(static_cast<uint32_t>(handle_.ptr - re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12TextureShaderResourceView::D3D12TextureShaderResourceView(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index,
		uint32_t array_size, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(texture->AccessHint() & EAH_GPU_Read);

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();

		tex_ = texture;
		pf_ = pf == EF_Unknown ? texture->Format() : pf;

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		first_level_ = first_level;
		num_levels_ = num_levels;
		first_elem_ = 0;
		num_elems_ = 0;

		sr_src_ = texture.get();
	}

	D3D12ShaderResourceViewSimulationPtr D3D12TextureShaderResourceView::RetrieveD3DShaderResourceView() const
	{
		if (!d3d_sr_view_ && tex_ && tex_->HWResourceReady())
		{
			d3d_sr_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DShaderResourceView(pf_, first_array_index_, array_size_,
				first_level_, num_levels_);
		}
		return d3d_sr_view_;
	}


	D3D12CubeTextureFaceShaderResourceView::D3D12CubeTextureFaceShaderResourceView(TexturePtr const& texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(texture_cube->AccessHint() & EAH_GPU_Read);

		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();

		tex_ = texture_cube;
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;

		first_array_index_ = array_index * 6 + face;
		array_size_ = 1;
		first_level_ = first_level;
		num_levels_ = num_levels;
		first_elem_ = 0;
		num_elems_ = 0;

		sr_src_ = texture_cube.get();
	}

	D3D12ShaderResourceViewSimulationPtr D3D12CubeTextureFaceShaderResourceView::RetrieveD3DShaderResourceView() const
	{
		if (!d3d_sr_view_ && tex_ && tex_->HWResourceReady())
		{
			uint32_t const array_index = first_array_index_ / 6;
			Texture::CubeFaces const face = static_cast<Texture::CubeFaces>(first_array_index_ - array_index * 6);
			d3d_sr_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DShaderResourceView(pf_, array_index, face,
				first_level_, num_levels_);
		}
		return d3d_sr_view_;
	}


	D3D12BufferShaderResourceView::D3D12BufferShaderResourceView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Read);

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();

		buff_ = gb;
		pf_ = pf;

		first_array_index_ = 0;
		array_size_ = 0;
		first_level_ = 0;
		num_levels_ = 0;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		sr_src_ = gb.get();
	}

	D3D12ShaderResourceViewSimulationPtr D3D12BufferShaderResourceView::RetrieveD3DShaderResourceView() const
	{
		if (!d3d_sr_view_ && buff_ && buff_->HWResourceReady())
		{
			d3d_sr_view_ = checked_cast<D3D12GraphicsBuffer&>(*buff_).RetrieveD3DShaderResourceView(pf_, first_elem_, num_elems_);
		}
		return d3d_sr_view_;
	}


	D3D12RenderTargetView::D3D12RenderTargetView(D3D12ResourcePtr const & src, uint32_t first_subres, uint32_t num_subres)
		: rt_src_(src), rt_first_subres_(first_subres), rt_num_subres_(num_subres)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
	}

	void D3D12RenderTargetView::ClearColor(Color const & clr)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < rt_num_subres_; ++ i)
		{
			rt_src_->UpdateResourceBarrier(d3d_cmd_list, rt_first_subres_ + i, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		d3d_cmd_list->ClearRenderTargetView(d3d_rt_view_->Handle(), &clr.r(), 0, nullptr);
	}

	void D3D12RenderTargetView::Discard()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < rt_num_subres_; ++ i)
		{
			rt_src_->UpdateResourceBarrier(d3d_cmd_list, rt_first_subres_ + i, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = rt_first_subres_;
		region.NumSubresources = rt_num_subres_;
		d3d_cmd_list->DiscardResource(rt_src_->D3DResource(), &region);
	}

	void D3D12RenderTargetView::OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}

	void D3D12RenderTargetView::OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}


	D3D12Texture1D2DCubeRenderTargetView::D3D12Texture1D2DCubeRenderTargetView(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
		: D3D12RenderTargetView(checked_pointer_cast<D3D12Texture>(texture), first_array_index * texture->NumMipMaps() + level, array_size)
	{
		BOOST_ASSERT(texture);

		tex_ = texture;
		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		this->RetrieveD3DRenderTargetView();
	}

	D3D12RenderTargetViewSimulation* D3D12Texture1D2DCubeRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && tex_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, array_size_, level_);
		}
		return d3d_rt_view_.get();
	}


	D3D12Texture3DRenderTargetView::D3D12Texture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: D3D12RenderTargetView(checked_pointer_cast<D3D12Texture>(texture_3d),
		(array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level,
			num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(texture_3d);

		tex_ = texture_3d;
		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = first_slice;
		num_slices_ = num_slices;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		this->RetrieveD3DRenderTargetView();
	}

	D3D12RenderTargetViewSimulation* D3D12Texture3DRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && tex_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, first_slice_,
				num_slices_, level_);
		}
		return d3d_rt_view_.get();
	}


	D3D12TextureCubeFaceRenderTargetView::D3D12TextureCubeFaceRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: D3D12RenderTargetView(checked_pointer_cast<D3D12Texture>(texture_cube),
			(array_index * 6 + face) * texture_cube->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture_cube);

		tex_ = texture_cube;
		width_ = texture_cube->Width(level);
		height_ = texture_cube->Width(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture_cube->Depth(0);
		first_face_ = face;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		this->RetrieveD3DRenderTargetView();
	}

	D3D12RenderTargetViewSimulation* D3D12TextureCubeFaceRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && tex_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, first_face_, level_);
		}
		return d3d_rt_view_.get();
	}


	D3D12BufferRenderTargetView::D3D12BufferRenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: D3D12RenderTargetView(checked_pointer_cast<D3D12GraphicsBuffer>(gb), 0, 1)
	{
		BOOST_ASSERT(gb);
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Write);

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();

		buff_ = gb;
		width_ = num_elems;
		height_ = 1;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;

		first_array_index_ = 0;
		array_size_ = 0;
		level_ = 0;
		first_slice_ = 0;
		num_slices_ = 0;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		this->RetrieveD3DRenderTargetView();
	}

	D3D12RenderTargetViewSimulation* D3D12BufferRenderTargetView::RetrieveD3DRenderTargetView() const
	{
		if (!d3d_rt_view_ && buff_->HWResourceReady())
		{
			d3d_rt_view_ = checked_cast<D3D12GraphicsBuffer&>(*buff_).RetrieveD3DRenderTargetView(pf_, first_elem_, num_elems_);
		}
		return d3d_rt_view_.get();
	}


	D3D12DepthStencilView::D3D12DepthStencilView(D3D12ResourcePtr const & src, uint32_t first_subres, uint32_t num_subres)
		: ds_src_(src), ds_first_subres_(first_subres), ds_num_subres_(num_subres)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
	}

	void D3D12DepthStencilView::ClearDepth(float depth)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		d3d_cmd_list->ClearDepthStencilView(d3d_ds_view_->Handle(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
	}

	void D3D12DepthStencilView::ClearStencil(int32_t stencil)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		d3d_cmd_list->ClearDepthStencilView(d3d_ds_view_->Handle(), D3D12_CLEAR_FLAG_STENCIL, 1, static_cast<uint8_t>(stencil), 0, nullptr);
	}

	void D3D12DepthStencilView::ClearDepthStencil(float depth, int32_t stencil)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		d3d_cmd_list->ClearDepthStencilView(d3d_ds_view_->Handle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			depth, static_cast<uint8_t>(stencil), 0, nullptr);
	}

	void D3D12DepthStencilView::Discard()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = ds_first_subres_;
		region.NumSubresources = ds_num_subres_;
		d3d_cmd_list->DiscardResource(ds_src_->D3DResource(), &region);
	}

	void D3D12DepthStencilView::OnAttached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}

	void D3D12DepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}


	D3D12Texture1D2DCubeDepthStencilView::D3D12Texture1D2DCubeDepthStencilView(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
		: D3D12DepthStencilView(checked_pointer_cast<D3D12Texture>(texture), first_array_index * texture->NumMipMaps() + level, array_size)
	{
		BOOST_ASSERT(texture);

		tex_ = texture;
		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;

		this->RetrieveD3DDepthStencilView();
	}

	D3D12Texture1D2DCubeDepthStencilView::D3D12Texture1D2DCubeDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf,
		uint32_t sample_count, uint32_t sample_quality)
		: D3D12DepthStencilView(nullptr, 0, 1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();

		auto& rf = Context::Instance().RenderFactoryInstance();
		tex_ = rf.MakeTexture2D(width, height, 1, 1, pf, sample_count, sample_quality, EAH_GPU_Write);
		ds_src_ = checked_pointer_cast<D3D12Texture>(tex_);

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		first_array_index_ = 0;
		array_size_ = 1;
		level_ = 0;
		first_slice_ = 0;
		num_slices_ = 1;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;

		this->RetrieveD3DDepthStencilView();
	}

	D3D12DepthStencilViewSimulation* D3D12Texture1D2DCubeDepthStencilView::RetrieveD3DDepthStencilView() const
	{
		if (!d3d_ds_view_ && tex_->HWResourceReady())
		{
			d3d_ds_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, array_size_, level_);
		}
		return d3d_ds_view_.get();
	}


	D3D12Texture3DDepthStencilView::D3D12Texture3DDepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: D3D12DepthStencilView(checked_pointer_cast<D3D12Texture>(texture_3d),
			(array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level,
			num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(texture_3d);

		tex_ = texture_3d;
		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = first_slice;
		num_slices_ = num_slices;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;

		this->RetrieveD3DDepthStencilView();
	}

	D3D12DepthStencilViewSimulation* D3D12Texture3DDepthStencilView::RetrieveD3DDepthStencilView() const
	{
		if (!d3d_ds_view_ && tex_->HWResourceReady())
		{
			d3d_ds_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, first_slice_,
				num_slices_, level_);
		}
		return d3d_ds_view_.get();
	}

	D3D12TextureCubeFaceDepthStencilView::D3D12TextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: D3D12DepthStencilView(checked_pointer_cast<D3D12Texture>(texture_cube),
			(array_index * 6 + face) * texture_cube->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture_cube);

		tex_ = texture_cube;
		width_ = texture_cube->Width(level);
		height_ = texture_cube->Width(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture_cube->Depth(0);
		first_face_ = face;
		num_faces_ = 1;

		this->RetrieveD3DDepthStencilView();
	}

	D3D12DepthStencilViewSimulation* D3D12TextureCubeFaceDepthStencilView::RetrieveD3DDepthStencilView() const
	{
		if (!d3d_ds_view_ && tex_->HWResourceReady())
		{
			d3d_ds_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, first_face_, level_);
		}
		return d3d_ds_view_.get();
	}


	D3D12UnorderedAccessView::D3D12UnorderedAccessView(D3D12ResourcePtr const & src, uint32_t first_subres, uint32_t num_subres)
		: ua_src_(src), ua_first_subres_(first_subres), ua_num_subres_(num_subres)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
	}

	void D3D12UnorderedAccessView::Clear(float4 const & val)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		ua_src_->UpdateResourceBarrier(d3d_cmd_list, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		re.FlushResourceBarriers(d3d_cmd_list);

		ID3D12DescriptorHeap* cbv_srv_uav_heap = re.CreateDynamicCBVSRVUAVDescriptorHeap(1);
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
		d3d_device_->CopyDescriptorsSimple(1, cpu_handle, d3d_ua_view_->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		clear_f4_val_ = val;
		d3d_cmd_list->ClearUnorderedAccessViewFloat(gpu_handle, d3d_ua_view_->Handle(),
			ua_src_->D3DResource(), &clear_f4_val_.x(), 0, nullptr);
	}

	void D3D12UnorderedAccessView::Clear(uint4 const & val)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		ua_src_->UpdateResourceBarrier(d3d_cmd_list, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		re.FlushResourceBarriers(d3d_cmd_list);

		ID3D12DescriptorHeap* cbv_srv_uav_heap = re.CreateDynamicCBVSRVUAVDescriptorHeap(1);
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
		d3d_device_->CopyDescriptorsSimple(1, cpu_handle, d3d_ua_view_->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		clear_ui4_val_ = val;
		d3d_cmd_list->ClearUnorderedAccessViewUint(gpu_handle, d3d_ua_view_->Handle(),
			ua_src_->D3DResource(), &clear_ui4_val_.x(), 0, nullptr);
	}

	void D3D12UnorderedAccessView::Discard()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_cmd_list = re.D3DRenderCmdList();

		for (uint32_t i = 0; i < ua_num_subres_; ++ i)
		{
			ua_src_->UpdateResourceBarrier(d3d_cmd_list, ua_first_subres_ + i, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		re.FlushResourceBarriers(d3d_cmd_list);

		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = ua_first_subres_;
		region.NumSubresources = ua_num_subres_;
		d3d_cmd_list->DiscardResource(ua_src_->D3DResource(), &region);
	}

	void D3D12UnorderedAccessView::OnAttached(FrameBuffer& fb, uint32_t index)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(index);
	}

	void D3D12UnorderedAccessView::OnDetached(FrameBuffer& fb, uint32_t index)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(index);
	}


	D3D12Texture1D2DCubeUnorderedAccessView::D3D12Texture1D2DCubeUnorderedAccessView(TexturePtr const & texture, ElementFormat pf,
		int first_array_index, int array_size, int level)
		: D3D12UnorderedAccessView(checked_pointer_cast<D3D12Texture>(texture), first_array_index * texture->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture);

		tex_ = texture;
		pf_ = pf == EF_Unknown ? texture->Format() : pf;

		first_array_index_ = first_array_index;
		array_size_ = array_size;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture->Depth(0);
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		this->RetrieveD3DUnorderedAccessView();
	}

	D3D12UnorderedAccessViewSimulation* D3D12Texture1D2DCubeUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && tex_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DUnorderedAccessView(pf_, first_array_index_, array_size_, level_);
		}
		return d3d_ua_view_.get();
	}


	D3D12Texture3DUnorderedAccessView::D3D12Texture3DUnorderedAccessView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: D3D12UnorderedAccessView(checked_pointer_cast<D3D12Texture>(texture_3d),
			(array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level,
			num_slices * texture_3d->NumMipMaps() + level)
	{
		BOOST_ASSERT(texture_3d);

		tex_ = texture_3d;
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = first_slice;
		num_slices_ = num_slices;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		this->RetrieveD3DUnorderedAccessView();
	}

	D3D12UnorderedAccessViewSimulation* D3D12Texture3DUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && tex_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DUnorderedAccessView(pf_, first_array_index_, first_slice_,
				num_slices_, level_);
		}
		return d3d_ua_view_.get();
	}


	D3D12TextureCubeFaceUnorderedAccessView::D3D12TextureCubeFaceUnorderedAccessView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: D3D12UnorderedAccessView(checked_pointer_cast<D3D12Texture>(texture_cube),
			(array_index * 6 + face) * texture_cube->NumMipMaps() + level, 1)
	{
		BOOST_ASSERT(texture_cube);

		tex_ = texture_cube;
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;

		first_array_index_ = array_index;
		array_size_ = 1;
		level_ = level;
		first_slice_ = 0;
		num_slices_ = texture_cube->Depth(0);
		first_face_ = face;
		num_faces_ = 1;
		first_elem_ = 0;
		num_elems_ = 0;

		this->RetrieveD3DUnorderedAccessView();
	}

	D3D12UnorderedAccessViewSimulation* D3D12TextureCubeFaceUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && tex_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D12Texture&>(*tex_).RetrieveD3DUnorderedAccessView(pf_, first_array_index_, first_face_, level_);
		}
		return d3d_ua_view_.get();
	}


	D3D12BufferUnorderedAccessView::D3D12BufferUnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: D3D12UnorderedAccessView(checked_pointer_cast<D3D12GraphicsBuffer>(gb), 0, 1)
	{
		BOOST_ASSERT(gb);
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Unordered);

		buff_ = gb;
		pf_ = pf;

		first_array_index_ = 0;
		array_size_ = 0;
		level_ = 0;
		first_slice_ = 0;
		num_slices_ = 0;
		first_face_ = Texture::CF_Positive_X;
		num_faces_ = 1;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		this->RetrieveD3DUnorderedAccessView();
	}

	D3D12UnorderedAccessViewSimulation* D3D12BufferUnorderedAccessView::RetrieveD3DUnorderedAccessView() const
	{
		if (!d3d_ua_view_ && buff_->HWResourceReady())
		{
			d3d_ua_view_ = checked_cast<D3D12GraphicsBuffer&>(*buff_).RetrieveD3DUnorderedAccessView(pf_, first_elem_, num_elems_);
		}
		return d3d_ua_view_.get();
	}

	void D3D12BufferUnorderedAccessView::ResetInitCount()
	{
		checked_pointer_cast<D3D12GraphicsBuffer>(buff_)->ResetInitCount(this->InitCount());
	}
}
