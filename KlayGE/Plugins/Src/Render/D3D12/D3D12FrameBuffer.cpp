/**
 * @file D3D12FrameBuffer.cpp
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
#include <KFL/Math.hpp>
#include <KFL/Hash.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>

namespace KlayGE
{
	D3D12FrameBuffer::D3D12FrameBuffer()
	{
		d3d_viewport_.MinDepth = 0.0f;
		d3d_viewport_.MaxDepth = 1.0f;
	}

	D3D12FrameBuffer::~D3D12FrameBuffer()
	{
	}

	std::wstring const & D3D12FrameBuffer::Description() const
	{
		static std::wstring const desc(L"Direct3D12 Framebuffer");
		return desc;
	}

	void D3D12FrameBuffer::OnBind()
	{
		this->SetRenderTargets();

		for (size_t i = 0; i < ua_views_.size(); ++ i)
		{
			checked_pointer_cast<D3D12UnorderedAccessView>(ua_views_[i])->ResetInitCount();
		}
	}

	void D3D12FrameBuffer::OnUnbind()
	{
	}

	void D3D12FrameBuffer::SetRenderTargets()
	{
		if (views_dirty_)
		{
			this->UpdateViewPointers();

			views_dirty_ = false;
		}

		auto& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* cmd_list = re.D3DRenderCmdList();

		cmd_list->OMSetRenderTargets(static_cast<UINT>(d3d_rt_handles_.size()),
			d3d_rt_handles_.empty() ? nullptr : &d3d_rt_handles_[0], false, d3d_ds_handle_ptr_);
		re.RSSetViewports(1, &d3d_viewport_);
	}

	void D3D12FrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				if (clr_views_[i])
				{
					clr_views_[i]->ClearColor(clr);
				}
			}
		}
		if ((flags & CBM_Depth) && (flags & CBM_Stencil))
		{
			if (ds_view_)
			{
				ds_view_->ClearDepthStencil(depth, stencil);
			}
		}
		else
		{
			if (flags & CBM_Depth)
			{
				if (ds_view_)
				{
					ds_view_->ClearDepth(depth);
				}
			}

			if (flags & CBM_Stencil)
			{
				if (ds_view_)
				{
					ds_view_->ClearStencil(stencil);
				}
			}
		}
	}

	void D3D12FrameBuffer::Discard(uint32_t flags)
	{
		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				if (clr_views_[i])
				{
					clr_views_[i]->Discard();
				}
			}
		}
		if ((flags & CBM_Depth) || (flags & CBM_Stencil))
		{
			if (ds_view_)
			{
				ds_view_->Discard();
			}
		}
	}

	void D3D12FrameBuffer::BindBarrier()
	{
		if (views_dirty_)
		{
			this->UpdateViewPointers();

			views_dirty_ = false;
		}

		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		for (size_t i = 0; i < d3d_rt_src_.size(); ++ i)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			for (uint32_t j = 0; j < d3d_rt_num_subres_[i]; ++ j)
			{
				if (d3d_rt_src_[i]->UpdateResourceBarrier(d3d_rt_first_subres_[i] + j, barrier, D3D12_RESOURCE_STATE_RENDER_TARGET))
				{
					barriers.push_back(barrier);
				}
			}
		}
		if (d3d_ds_src_)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			for (uint32_t j = 0; j < d3d_ds_num_subres_; ++ j)
			{
				if (d3d_ds_src_->UpdateResourceBarrier(d3d_ds_first_subres_ + j, barrier, D3D12_RESOURCE_STATE_DEPTH_WRITE))
				{
					barriers.push_back(barrier);
				}
			}
		}
		if (!barriers.empty())
		{
			cmd_list->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
		}
	}

	void D3D12FrameBuffer::UpdateViewPointers()
	{
		d3d_rt_src_.clear();
		d3d_rt_first_subres_.clear();
		d3d_rt_num_subres_.clear();
		d3d_rt_handles_.resize(clr_views_.size());
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			if (clr_views_[i])
			{
				D3D12RenderTargetRenderView* p = checked_cast<D3D12RenderTargetRenderView*>(clr_views_[i].get());
				d3d_rt_src_.push_back(p->RTSrc().get());
				d3d_rt_first_subres_.push_back(p->RTFirstSubRes());
				d3d_rt_num_subres_.push_back(p->RTNumSubRes());

				d3d_rt_handles_[i] = p->D3DRenderTargetView()->Handle();
			}
			else
			{
#ifdef KLAYGE_CPU_X64
				d3d_rt_handles_[i].ptr = ~0ULL;
#else
				d3d_rt_handles_[i].ptr = ~0UL;
#endif
			}
		}

		if (ds_view_)
		{
			D3D12DepthStencilRenderView* p = checked_cast<D3D12DepthStencilRenderView*>(ds_view_.get());
			d3d_ds_src_ = p->DSSrc().get();
			d3d_ds_first_subres_ = p->DSFirstSubRes();
			d3d_ds_num_subres_ = p->DSNumSubRes();

			d3d_ds_handle_ = p->D3DDepthStencilView()->Handle();
			d3d_ds_handle_ptr_ = &d3d_ds_handle_;
		}
		else
		{
			d3d_ds_src_ = nullptr;
			d3d_ds_first_subres_ = 0;
			d3d_ds_num_subres_ = 0;

			d3d_ds_handle_ptr_ = nullptr;
		}

		d3d_viewport_.TopLeftX = static_cast<float>(viewport_->left);
		d3d_viewport_.TopLeftY = static_cast<float>(viewport_->top);
		d3d_viewport_.Width = static_cast<float>(viewport_->width);
		d3d_viewport_.Height = static_cast<float>(viewport_->height);

		pso_hash_value_ = 0;
		num_rts_ = 0;
		rtv_formats_.fill(DXGI_FORMAT_UNKNOWN);
		sample_count_ = 0;
		sample_quality_ = 0;
		for (size_t i = 0; i < clr_views_.size(); ++ i)
		{
			auto view = clr_views_[i].get();
			if (view)
			{
				auto fmt = view->Format();
				HashCombine(pso_hash_value_, fmt);
				rtv_formats_[i] = D3D12Mapping::MappingFormat(fmt);
				num_rts_ = static_cast<uint32_t>(i + 1);

				if (sample_count_ == 0)
				{
					sample_count_ = view->SampleCount();
					sample_quality_ = view->SampleQuality();
				}
				else
				{
					BOOST_ASSERT(sample_count_ == view->SampleCount());
					BOOST_ASSERT(sample_quality_ == view->SampleQuality());
				}
			}
		}
		{
			auto view = ds_view_.get();
			if (view)
			{
				auto fmt = view->Format();
				HashCombine(pso_hash_value_, fmt);
				dsv_format_ = D3D12Mapping::MappingFormat(fmt);

				if (sample_count_ == 0)
				{
					sample_count_ = view->SampleCount();
					sample_quality_ = view->SampleQuality();
				}
				else
				{
					BOOST_ASSERT(sample_count_ == view->SampleCount());
					BOOST_ASSERT(sample_quality_ == view->SampleQuality());
				}
			}
			else
			{
				dsv_format_ = DXGI_FORMAT_UNKNOWN;
			}
		}

		HashCombine(pso_hash_value_, sample_count_);
		HashCombine(pso_hash_value_, sample_quality_);
	}

	size_t D3D12FrameBuffer::PsoHashValue()
	{
		if (views_dirty_)
		{
			this->UpdateViewPointers();

			views_dirty_ = false;
		}

		return pso_hash_value_;
	}

	void D3D12FrameBuffer::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc)
	{
		if (views_dirty_)
		{
			this->UpdateViewPointers();

			views_dirty_ = false;
		}

		pso_desc.NumRenderTargets = num_rts_;
		for (uint32_t i = 0; i < 8; ++ i)
		{
			pso_desc.RTVFormats[i] = rtv_formats_[i];
		}
		pso_desc.DSVFormat = dsv_format_;
		pso_desc.SampleDesc.Count = sample_count_;
		pso_desc.SampleDesc.Quality = sample_quality_;
	}
}
