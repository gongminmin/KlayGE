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
#include <KFL/ThrowErr.hpp>
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
	D3D12ShaderResourceViewSimulation::D3D12ShaderResourceViewSimulation(ID3D12ResourcePtr const & res,
			D3D12_SHADER_RESOURCE_VIEW_DESC const & srv_desc)
		: res_(res)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		handle_ = re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocCBVSRVUAV();
		device->CreateShaderResourceView(res.get(), &srv_desc, handle_);
	}

	D3D12ShaderResourceViewSimulation::~D3D12ShaderResourceViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocCBVSRVUAV(static_cast<uint32_t>(handle_.ptr - re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12RenderTargetViewSimulation::D3D12RenderTargetViewSimulation(ID3D12ResourcePtr const & res,
			D3D12_RENDER_TARGET_VIEW_DESC const & rtv_desc)
		: res_(res)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		handle_ = re.RTVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocRTV();
		device->CreateRenderTargetView(res.get(), &rtv_desc, handle_);
	}

	D3D12RenderTargetViewSimulation::~D3D12RenderTargetViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocRTV(static_cast<uint32_t>(handle_.ptr - re.RTVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12DepthStencilViewSimulation::D3D12DepthStencilViewSimulation(ID3D12ResourcePtr const & res,
		D3D12_DEPTH_STENCIL_VIEW_DESC const & dsv_desc)
		: res_(res)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		handle_ = re.DSVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocDSV();
		device->CreateDepthStencilView(res.get(), &dsv_desc, handle_);
	}

	D3D12DepthStencilViewSimulation::~D3D12DepthStencilViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocDSV(static_cast<uint32_t>(handle_.ptr - re.DSVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12UnorderedAccessViewSimulation::D3D12UnorderedAccessViewSimulation(ID3D12ResourcePtr const & res,
			D3D12_UNORDERED_ACCESS_VIEW_DESC const & uav_desc)
		: res_(res), counter_offset_(0)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		ID3D12Resource* counter = nullptr;
		if (D3D12_UAV_DIMENSION_BUFFER == uav_desc.ViewDimension)
		{
			counter_offset_ = static_cast<uint32_t>(uav_desc.Buffer.CounterOffsetInBytes);
			if (counter_offset_ != 0)
			{
				counter = res.get();
			}
		}

		handle_ = re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart();
		handle_.ptr += re.AllocCBVSRVUAV();
		device->CreateUnorderedAccessView(res.get(), counter, &uav_desc, handle_);
	}

	D3D12UnorderedAccessViewSimulation::~D3D12UnorderedAccessViewSimulation()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocCBVSRVUAV(static_cast<uint32_t>(handle_.ptr - re.CBVSRVUAVDescHeap()->GetCPUDescriptorHandleForHeapStart().ptr));
		}
	}


	D3D12RenderView::D3D12RenderView()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();
	}

	D3D12RenderView::~D3D12RenderView()
	{
	}


	D3D12RenderTargetRenderView::D3D12RenderTargetRenderView(Texture& texture, int first_array_index, int array_size, int level)
		: rt_first_subres_(first_array_index * texture.NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture);
		rt_view_ = d3d_tex->RetriveD3DRenderTargetView(first_array_index, array_size, level);
		rt_src_ = d3d_tex->D3DResource();

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
	}

	D3D12RenderTargetRenderView::D3D12RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: rt_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), rt_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture_3d);
		rt_view_ = d3d_tex->RetriveD3DRenderTargetView(array_index, first_slice, num_slices, level);
		rt_src_ = d3d_tex->D3DResource();

		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
	}

	D3D12RenderTargetRenderView::D3D12RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: rt_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), rt_num_subres_(1)
	{
		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture_cube);
		rt_view_ = d3d_tex->RetriveD3DRenderTargetView(array_index, face, level);
		rt_src_ = d3d_tex->D3DResource();

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
	}

	D3D12RenderTargetRenderView::D3D12RenderTargetRenderView(GraphicsBuffer& gb, uint32_t width, uint32_t height, ElementFormat pf)
		: rt_first_subres_(0), rt_num_subres_(1)
	{
		BOOST_ASSERT(gb.AccessHint() & EAH_GPU_Write);

		rt_src_ = checked_cast<D3D12GraphicsBuffer*>(&gb)->D3DBuffer();

		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = std::min(width * height, gb.Size() / NumFormatBytes(pf));

		rt_view_ = MakeSharedPtr<D3D12RenderTargetViewSimulation>(rt_src_, desc);

		width_ = width * height;
		height_ = 1;
		pf_ = pf;
	}

	void D3D12RenderTargetRenderView::ClearColor(Color const & clr)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = rt_src_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		for (uint32_t i = 0; i < rt_num_subres_; ++ i)
		{
			barrier.Transition.Subresource = rt_first_subres_ + i;
			barriers.push_back(barrier);
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);

		d3d_cmd_list_->ClearRenderTargetView(rt_view_->Handle(), &clr.r(), 0, nullptr);

		for (uint32_t i = 0; i < rt_num_subres_; ++ i)
		{
			barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barriers[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
	}

	void D3D12RenderTargetRenderView::ClearDepth(float /*depth*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12RenderTargetRenderView::ClearStencil(int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12RenderTargetRenderView::ClearDepthStencil(float /*depth*/, int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D12RenderTargetRenderView::Discard()
	{
		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = rt_first_subres_;
		region.NumSubresources = rt_num_subres_;
		d3d_cmd_list_->DiscardResource(rt_src_.get(), &region);
	}

	void D3D12RenderTargetRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}

	void D3D12RenderTargetRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}


	D3D12DepthStencilRenderView::D3D12DepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
		: ds_first_subres_(first_array_index * texture.NumMipMaps() + level), ds_num_subres_(1)
	{
		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture);
		ds_view_ = d3d_tex->RetriveD3DDepthStencilView(first_array_index, array_size, level);
		ds_src_ = d3d_tex->D3DResource();

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
	}

	D3D12DepthStencilRenderView::D3D12DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: ds_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), ds_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture_3d);
		ds_view_ = d3d_tex->RetriveD3DDepthStencilView(array_index, first_slice, num_slices, level);
		ds_src_ = d3d_tex->D3DResource();

		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
	}

	D3D12DepthStencilRenderView::D3D12DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: ds_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), ds_num_subres_(1)
	{
		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture_cube);
		ds_view_ = d3d_tex->RetriveD3DDepthStencilView(array_index, face, level);
		ds_src_ = d3d_tex->D3DResource();

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
	}

	D3D12DepthStencilRenderView::D3D12DepthStencilRenderView(uint32_t width, uint32_t height,
											ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: ds_first_subres_(0), ds_num_subres_(1)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		D3D12_RESOURCE_DESC tex_desc;
		tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		tex_desc.Alignment = 0;
		tex_desc.Width = width;
		tex_desc.Height = height;
		tex_desc.DepthOrArraySize = 1;
		tex_desc.MipLevels = 1;
		tex_desc.Format = D3D12Mapping::MappingFormat(pf);
		tex_desc.SampleDesc.Count = sample_count;
		tex_desc.SampleDesc.Quality = sample_quality;
		tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		tex_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		ID3D12Resource* depth_tex;
		TIF(d3d_device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&tex_desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&depth_tex)));
		ds_src_ = MakeCOMPtr(depth_tex);

		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = tex_desc.Format;
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (sample_count > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			desc.Texture2DMSArray.FirstArraySlice = 0;
			desc.Texture2DMSArray.ArraySize = 1;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = 0;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = 1;
		}

		ds_view_ = MakeSharedPtr<D3D12DepthStencilViewSimulation>(ds_src_, desc);

		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void D3D12DepthStencilRenderView::ClearColor(Color const & clr)
	{
		KFL_UNUSED(clr);
		BOOST_ASSERT(false);
	}

	void D3D12DepthStencilRenderView::ClearDepth(float depth)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = ds_src_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			barrier.Transition.Subresource = ds_first_subres_ + i;
			barriers.push_back(barrier);
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);

		d3d_cmd_list_->ClearDepthStencilView(ds_view_->Handle(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			barriers[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
	}

	void D3D12DepthStencilRenderView::ClearStencil(int32_t stencil)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = ds_src_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			barrier.Transition.Subresource = ds_first_subres_ + i;
			barriers.push_back(barrier);
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);

		d3d_cmd_list_->ClearDepthStencilView(ds_view_->Handle(), D3D12_CLEAR_FLAG_STENCIL, 1, static_cast<uint8_t>(stencil), 0, nullptr);

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			barriers[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
	}

	void D3D12DepthStencilRenderView::ClearDepthStencil(float depth, int32_t stencil)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = ds_src_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			barrier.Transition.Subresource = ds_first_subres_ + i;
			barriers.push_back(barrier);
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);

		d3d_cmd_list_->ClearDepthStencilView(ds_view_->Handle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, static_cast<uint8_t>(stencil), 0, nullptr);

		for (uint32_t i = 0; i < ds_num_subres_; ++ i)
		{
			barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			barriers[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		}
		d3d_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
	}

	void D3D12DepthStencilRenderView::Discard()
	{
		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = ds_first_subres_;
		region.NumSubresources = ds_num_subres_;
		d3d_cmd_list_->DiscardResource(ds_src_.get(), &region);
	}

	void D3D12DepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);
	}

	void D3D12DepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);
	}


	D3D12UnorderedAccessView::D3D12UnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
		: ua_first_subres_(first_array_index * texture.NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture);
		ua_view_ = d3d_tex->RetriveD3DUnorderedAccessView(first_array_index, array_size, level);
		ua_src_ = d3d_tex->D3DResource();
		counter_offset_ = 0;

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
		: ua_first_subres_((array_index * texture_3d.Depth(level) + first_slice) * texture_3d.NumMipMaps() + level), ua_num_subres_(num_slices * texture_3d.NumMipMaps() + level)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture_3d);
		ua_view_ = d3d_tex->RetriveD3DUnorderedAccessView(array_index, first_slice, num_slices, level);
		ua_src_ = d3d_tex->D3DResource();
		counter_offset_ = 0;

		width_ = texture_3d.Width(level);
		height_ = texture_3d.Height(level);
		pf_ = texture_3d.Format();
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: ua_first_subres_((array_index * 6 + face) * texture_cube.NumMipMaps() + level), ua_num_subres_(1)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

		D3D12Texture* d3d_tex = checked_cast<D3D12Texture*>(&texture_cube);
		ua_view_ = d3d_tex->RetriveD3DUnorderedAccessView(array_index, face, level);
		ua_src_ = d3d_tex->D3DResource();
		counter_offset_ = 0;

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Width(level);
		pf_ = texture_cube.Format();
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(GraphicsBuffer& gb, ElementFormat pf)
		: ua_first_subres_(0), ua_num_subres_(1)
	{
		BOOST_ASSERT(gb.AccessHint() & EAH_GPU_Write);

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d_device_ = re.D3DDevice();
		d3d_cmd_list_ = re.D3DRenderCmdList();

		D3D12GraphicsBuffer* d3d_buff = checked_cast<D3D12GraphicsBuffer*>(&gb);
		ua_view_ = d3d_buff->D3DUnorderedAccessView();
		ua_src_ = d3d_buff->D3DBuffer();
		ua_counter_upload_src_ = d3d_buff->D3DBufferCounterUpload();
		counter_offset_ = d3d_buff->CounterOffset();

		if ((gb.AccessHint() & EAH_CPU_Read) || (gb.AccessHint() & EAH_CPU_Write))
		{
			ua_src_init_state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else
		{
			ua_src_init_state_ = D3D12_RESOURCE_STATE_COMMON;
		}

		width_ = gb.Size() / NumFormatBytes(pf);
		height_ = 1;
		pf_ = pf;
	}

	void D3D12UnorderedAccessView::Clear(float4 const & val)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_heap_desc.NumDescriptors = 1;
		cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeap* csu_heap;
		TIF(d3d_device_->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&csu_heap)));
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap = MakeCOMPtr(csu_heap);

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.pResource = ua_src_.get();
		barrier_before.Transition.StateBefore = ua_src_init_state_;
		barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier_before.Transition.Subresource = 0;
		d3d_cmd_list_->ResourceBarrier(1, &barrier_before);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
		d3d_device_->CopyDescriptorsSimple(1, cpu_handle, ua_view_->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		clear_f4_val_ = val;
		d3d_cmd_list_->ClearUnorderedAccessViewFloat(gpu_handle, ua_view_->Handle(),
			ua_src_.get(), &clear_f4_val_.x(), 0, nullptr);

		D3D12_RESOURCE_BARRIER barrier_after;
		barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after.Transition.pResource = ua_src_.get();
		barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier_after.Transition.StateAfter = ua_src_init_state_;
		barrier_after.Transition.Subresource = 0;
		d3d_cmd_list_->ResourceBarrier(1, &barrier_after);
	}

	void D3D12UnorderedAccessView::Clear(uint4 const & val)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_heap_desc.NumDescriptors = 1;
		cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeap* csu_heap;
		TIF(d3d_device_->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&csu_heap)));
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap = MakeCOMPtr(csu_heap);

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.pResource = ua_src_.get();
		barrier_before.Transition.StateBefore = ua_src_init_state_;
		barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier_before.Transition.Subresource = 0;
		d3d_cmd_list_->ResourceBarrier(1, &barrier_before);

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();
		d3d_device_->CopyDescriptorsSimple(1, cpu_handle, ua_view_->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		clear_ui4_val_ = val;
		d3d_cmd_list_->ClearUnorderedAccessViewUint(gpu_handle, ua_view_->Handle(),
			ua_src_.get(), &clear_ui4_val_.x(), 0, nullptr);

		D3D12_RESOURCE_BARRIER barrier_after;
		barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_after.Transition.pResource = ua_src_.get();
		barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier_after.Transition.StateAfter = ua_src_init_state_;
		barrier_after.Transition.Subresource = 0;
		d3d_cmd_list_->ResourceBarrier(1, &barrier_after);
	}

	void D3D12UnorderedAccessView::Discard()
	{
		D3D12_DISCARD_REGION region;
		region.NumRects = 0;
		region.pRects = nullptr;
		region.FirstSubresource = ua_first_subres_;
		region.NumSubresources = ua_num_subres_;
		d3d_cmd_list_->DiscardResource(ua_src_.get(), &region);
	}

	void D3D12UnorderedAccessView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}

	void D3D12UnorderedAccessView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
	}

	void D3D12UnorderedAccessView::ResetInitCount()
	{
		if (ua_counter_upload_src_)
		{
			uint32_t const count = this->InitCount();
			void* mapped = nullptr;
			ua_counter_upload_src_->Map(0, nullptr, &mapped);
			memcpy(mapped, &count, sizeof(count));
			ua_counter_upload_src_->Unmap(0, nullptr);

			D3D12_RESOURCE_BARRIER barrier_before;
			barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_before.Transition.pResource = ua_src_.get();
			barrier_before.Transition.StateBefore = ua_src_init_state_;
			barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier_before.Transition.Subresource = 0;
			d3d_cmd_list_->ResourceBarrier(1, &barrier_before);

			d3d_cmd_list_->CopyBufferRegion(ua_src_.get(),
				counter_offset_, ua_counter_upload_src_.get(), 0, sizeof(count));

			D3D12_RESOURCE_BARRIER barrier_after;
			barrier_after.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier_after.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier_after.Transition.pResource = ua_src_.get();
			barrier_after.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier_after.Transition.StateAfter = ua_src_init_state_;
			barrier_after.Transition.Subresource = 0;
			d3d_cmd_list_->ResourceBarrier(1, &barrier_after);
		}
	}
}
