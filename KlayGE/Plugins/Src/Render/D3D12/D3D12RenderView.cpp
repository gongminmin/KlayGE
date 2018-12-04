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
#include <KFL/COMPtr.hpp>
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
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		handle_ = re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocCBVSRVUAV();
		device->CreateShaderResourceView(res_->D3DResource().get(), &srv_desc, handle_);
	}

	D3D12ShaderResourceViewSimulation::~D3D12ShaderResourceViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocCBVSRVUAV(static_cast<uint32_t>(handle_.ptr - re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12RenderTargetViewSimulation::D3D12RenderTargetViewSimulation(D3D12Resource const * res,
			D3D12_RENDER_TARGET_VIEW_DESC const & rtv_desc)
		: res_(res)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		handle_ = re.RTVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocRTV();
		device->CreateRenderTargetView(res_->D3DResource().get(), &rtv_desc, handle_);
	}

	D3D12RenderTargetViewSimulation::~D3D12RenderTargetViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocRTV(static_cast<uint32_t>(handle_.ptr - re.RTVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12DepthStencilViewSimulation::D3D12DepthStencilViewSimulation(D3D12Resource const * res,
		D3D12_DEPTH_STENCIL_VIEW_DESC const & dsv_desc)
		: res_(res)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		handle_ = re.DSVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocDSV();
		device->CreateDepthStencilView(res_->D3DResource().get(), &dsv_desc, handle_);
	}

	D3D12DepthStencilViewSimulation::~D3D12DepthStencilViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocDSV(static_cast<uint32_t>(handle_.ptr - re.DSVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12UnorderedAccessViewSimulation::D3D12UnorderedAccessViewSimulation(D3D12Resource const * res,
			D3D12_UNORDERED_ACCESS_VIEW_DESC const & uav_desc)
		: res_(res), counter_offset_(0)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		ID3D12Resource* counter = nullptr;
		if (D3D12_UAV_DIMENSION_BUFFER == uav_desc.ViewDimension)
		{
			counter_offset_ = static_cast<uint32_t>(uav_desc.Buffer.CounterOffsetInBytes);
			if (counter_offset_ != 0)
			{
				counter = res_->D3DResource().get();
			}
		}

		handle_ = re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocCBVSRVUAV();
		device->CreateUnorderedAccessView(res_->D3DResource().get(), counter, &uav_desc, handle_);
	}

	D3D12UnorderedAccessViewSimulation::~D3D12UnorderedAccessViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocCBVSRVUAV(static_cast<uint32_t>(handle_.ptr - re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12RenderTargetView::D3D12RenderTargetView(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
		: rt_src_(checked_pointer_cast<D3D12Texture>(texture)), rt_first_subres_(first_array_index * texture->NumMipMaps() + level),
			rt_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_rt_view_ = checked_cast<D3D12Texture*>(texture.get())->CreateD3DRenderTargetView(pf_, first_array_index_, array_size_, level_);
	}

	D3D12RenderTargetView::D3D12RenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: rt_src_(checked_pointer_cast<D3D12Texture>(texture_3d)),
			rt_first_subres_((array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level),
			rt_num_subres_(num_slices * texture_3d->NumMipMaps() + level)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_rt_view_ = checked_cast<D3D12Texture*>(texture_3d.get())->CreateD3DRenderTargetView(pf_, first_array_index_, first_slice_,
			num_slices_, level_);
	}

	D3D12RenderTargetView::D3D12RenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: rt_src_(checked_pointer_cast<D3D12Texture>(texture_cube)),
			rt_first_subres_((array_index * 6 + face) * texture_cube->NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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
	
		d3d_rt_view_ = checked_cast<D3D12Texture*>(texture_cube.get())->CreateD3DRenderTargetView(pf_, first_array_index_, first_face_,
			level_);
	}

	D3D12RenderTargetView::D3D12RenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem, uint32_t num_elems)
		: rt_src_(checked_pointer_cast<D3D12GraphicsBuffer>(gb)), rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(gb->AccessHint() & EAH_GPU_Write);

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_rt_view_ = checked_cast<D3D12GraphicsBuffer*>(gb.get())->CreateD3DRenderTargetView(pf, first_elem_, num_elems_);
	}

	void D3D12RenderTargetView::ClearColor(Color const & clr)
	{
		for (uint32_t i = 0; i < rt_num_subres_; ++ i)
		{
			rt_src_->UpdateResourceBarrier(d3d_cmd_list_, rt_first_subres_ + i, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		d3d_cmd_list_->ClearRenderTargetView(d3d_rt_view_->Handle(), &clr.r(), 0, nullptr);
	}

	void D3D12RenderTargetView::Discard()
	{
		for (uint32_t i = 0; i < rt_num_subres_; ++ i)
		{
			rt_src_->UpdateResourceBarrier(d3d_cmd_list_, rt_first_subres_ + i, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = rt_first_subres_;
		region.NumSubresources = rt_num_subres_;
		d3d_cmd_list_->DiscardResource(rt_src_->D3DResource().get(), &region);
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


	D3D12DepthStencilView::D3D12DepthStencilView(TexturePtr const & texture, ElementFormat pf, int first_array_index,
		int array_size, int level)
		: ds_src_(checked_pointer_cast<D3D12Texture>(texture)),
			ds_first_subres_(first_array_index * texture->NumMipMaps() + level), ds_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_ds_view_ = checked_cast<D3D12Texture*>(texture.get())->CreateD3DDepthStencilView(pf_, first_array_index_, array_size_, level_);
	}

	D3D12DepthStencilView::D3D12DepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: ds_src_(checked_pointer_cast<D3D12Texture>(texture_3d)),
			ds_first_subres_((array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level),
			ds_num_subres_(num_slices * texture_3d->NumMipMaps() + level)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_ds_view_ = checked_cast<D3D12Texture*>(texture_3d.get())->CreateD3DDepthStencilView(pf_, first_array_index_, first_slice_,
			num_slices_, level_);
	}

	D3D12DepthStencilView::D3D12DepthStencilView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: ds_src_(checked_pointer_cast<D3D12Texture>(texture_cube)),
			ds_first_subres_((array_index * 6 + face) * texture_cube->NumMipMaps() + level), ds_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_ds_view_ = checked_cast<D3D12Texture*>(texture_cube.get())->CreateD3DDepthStencilView(pf_, first_array_index_, first_face_,
			level_);
	}

	D3D12DepthStencilView::D3D12DepthStencilView(uint32_t width, uint32_t height,
											ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: ds_first_subres_(0), ds_num_subres_(1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		d3d_ds_view_ = checked_cast<D3D12Texture*>(tex_.get())->CreateD3DDepthStencilView(pf_, first_array_index_, array_size_, level_);
	}

	void D3D12DepthStencilView::ClearDepth(float depth)
	{
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list_, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		d3d_cmd_list_->ClearDepthStencilView(d3d_ds_view_->Handle(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
	}

	void D3D12DepthStencilView::ClearStencil(int32_t stencil)
	{
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list_, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		d3d_cmd_list_->ClearDepthStencilView(d3d_ds_view_->Handle(), D3D12_CLEAR_FLAG_STENCIL, 1, static_cast<uint8_t>(stencil), 0, nullptr);
	}

	void D3D12DepthStencilView::ClearDepthStencil(float depth, int32_t stencil)
	{
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list_, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		d3d_cmd_list_->ClearDepthStencilView(d3d_ds_view_->Handle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			depth, static_cast<uint8_t>(stencil), 0, nullptr);
	}

	void D3D12DepthStencilView::Discard()
	{
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			ds_src_->UpdateResourceBarrier(d3d_cmd_list_, ds_first_subres_ + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = ds_first_subres_;
		region.NumSubresources = ds_num_subres_;
		d3d_cmd_list_->DiscardResource(ds_src_->D3DResource().get(), &region);
	}

	void D3D12DepthStencilView::OnAttached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}

	void D3D12DepthStencilView::OnDetached(FrameBuffer& fb)
	{
		KFL_UNUSED(fb);
	}


	D3D12UnorderedAccessView::D3D12UnorderedAccessView(TexturePtr const & texture, ElementFormat pf, int first_array_index, int array_size,
		int level)
		: ua_first_subres_(first_array_index * texture->NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		ua_src_ = checked_pointer_cast<D3D12Texture>(texture);
		counter_offset_ = 0;

		d3d_ua_view_ = checked_cast<D3D12Texture*>(tex_.get())->CreateD3DUnorderedAccessView(pf_, first_array_index, array_size, level);
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(TexturePtr const & texture_3d, ElementFormat pf, int array_index,
		uint32_t first_slice, uint32_t num_slices, int level)
		: ua_first_subres_((array_index * texture_3d->Depth(level) + first_slice) * texture_3d->NumMipMaps() + level), ua_num_subres_(num_slices * texture_3d->NumMipMaps() + level)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		ua_src_ = checked_pointer_cast<D3D12Texture>(texture_3d);
		counter_offset_ = 0;

		d3d_ua_view_ = checked_cast<D3D12Texture*>(tex_.get())->CreateD3DUnorderedAccessView(pf_, first_array_index_, first_slice_,
			num_slices_, level_);
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: ua_first_subres_((array_index * 6 + face) * texture_cube->NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		ua_src_ = checked_pointer_cast<D3D12Texture>(texture_cube);
		counter_offset_ = 0;

		d3d_ua_view_ = checked_cast<D3D12Texture*>(tex_.get())->CreateD3DUnorderedAccessView(pf_, first_array_index_, first_face_, level_);
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
		: ua_first_subres_(0), ua_num_subres_(1)
	{
		uint32_t const access_hint = gb->AccessHint();
		BOOST_ASSERT(access_hint & EAH_GPU_Unordered);
		KFL_UNUSED(access_hint);

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

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

		D3D12GraphicsBufferPtr d3d_buff = checked_pointer_cast<D3D12GraphicsBuffer>(gb);
		ua_src_ = d3d_buff;
		ua_counter_upload_src_ = d3d_buff->D3DBufferCounterUpload();
		counter_offset_ = d3d_buff->CounterOffset();

		d3d_ua_view_ = checked_cast<D3D12GraphicsBuffer*>(buff_.get())->CreateD3DUnorderedAccessView(pf_, first_elem_, num_elems_);
	}

	void D3D12UnorderedAccessView::Clear(float4 const & val)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_heap_desc.NumDescriptors = 1;
		cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeap* csu_heap;
		TIFHR(d3d_device_->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&csu_heap)));
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap = MakeCOMPtr(csu_heap);

		ua_src_->UpdateResourceBarrier(d3d_cmd_list_, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
		d3d_device_->CopyDescriptorsSimple(1, cpu_handle, d3d_ua_view_->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		clear_f4_val_ = val;
		d3d_cmd_list_->ClearUnorderedAccessViewFloat(gpu_handle, d3d_ua_view_->Handle(),
			ua_src_->D3DResource().get(), &clear_f4_val_.x(), 0, nullptr);
	}

	void D3D12UnorderedAccessView::Clear(uint4 const & val)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_heap_desc.NumDescriptors = 1;
		cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeap* csu_heap;
		TIFHR(d3d_device_->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&csu_heap)));
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap = MakeCOMPtr(csu_heap);

		ua_src_->UpdateResourceBarrier(d3d_cmd_list_, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
		d3d_device_->CopyDescriptorsSimple(1, cpu_handle, d3d_ua_view_->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		clear_ui4_val_ = val;
		d3d_cmd_list_->ClearUnorderedAccessViewUint(gpu_handle, d3d_ua_view_->Handle(),
			ua_src_->D3DResource().get(), &clear_ui4_val_.x(), 0, nullptr);
	}

	void D3D12UnorderedAccessView::Discard()
	{
		for (uint32_t i = 0; i < ua_num_subres_; ++ i)
		{
			ua_src_->UpdateResourceBarrier(d3d_cmd_list_, ua_first_subres_ + i, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.FlushResourceBarriers(d3d_cmd_list_);

		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = ua_first_subres_;
		region.NumSubresources = ua_num_subres_;
		d3d_cmd_list_->DiscardResource(ua_src_->D3DResource().get(), &region);
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

	void D3D12UnorderedAccessView::ResetInitCount()
	{
		if (ua_counter_upload_src_)
		{
			uint32_t const count = this->InitCount();

			D3D12_RANGE read_range;
			read_range.Begin = 0;
			read_range.End = 0;

			void* mapped = nullptr;
			ua_counter_upload_src_->Map(0, &read_range, &mapped);
			memcpy(mapped, &count, sizeof(count));
			ua_counter_upload_src_->Unmap(0, nullptr);

			ua_src_->UpdateResourceBarrier(d3d_cmd_list_, 0, D3D12_RESOURCE_STATE_COPY_DEST);

			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.FlushResourceBarriers(d3d_cmd_list_);

			d3d_cmd_list_->CopyBufferRegion(ua_src_->D3DResource().get(),
				counter_offset_, ua_counter_upload_src_.get(), 0, sizeof(count));
		}
	}
}
