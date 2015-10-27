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
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>

namespace KlayGE
{
	D3D12FrameBuffer::D3D12FrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		viewport_->left	= left_;
		viewport_->top	= top_;

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

		for (size_t i = 0; i < ua_views_.size(); ++i)
		{
			checked_pointer_cast<D3D12UnorderedAccessView>(ua_views_[i])->ResetInitCount();
		}
	}

	void D3D12FrameBuffer::SetRenderTargets()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		std::vector<ID3D12Resource*> rt_src;
		std::vector<uint32_t> rt_first_subres;
		std::vector<uint32_t> rt_num_subres;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rt_handles(clr_views_.size());
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			if (clr_views_[i])
			{
				D3D12RenderTargetRenderView* p = checked_cast<D3D12RenderTargetRenderView*>(clr_views_[i].get());
				rt_src.push_back(p->RTSrc().get());
				rt_first_subres.push_back(p->RTFirstSubRes());
				rt_num_subres.push_back(p->RTNumSubRes());

				rt_handles[i] = p->D3DRenderTargetView()->Handle();
			}
			else
			{
#ifdef KLAYGE_CPU_X64
				rt_handles[i].ptr = ~0ULL;
#else
				rt_handles[i].ptr = ~0UL;
#endif
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE ds_handle;
		D3D12_CPU_DESCRIPTOR_HANDLE* ds_handle_ptr;
		if (rs_view_)
		{
			D3D12DepthStencilRenderView* p = checked_cast<D3D12DepthStencilRenderView*>(rs_view_.get());

			ds_handle = p->D3DDepthStencilView()->Handle();
			ds_handle_ptr = &ds_handle;
		}
		else
		{
			ds_handle_ptr = nullptr;
		}

		cmd_list->OMSetRenderTargets(static_cast<UINT>(rt_handles.size()),
			rt_handles.empty() ? nullptr : &rt_handles[0], false, ds_handle_ptr);

		d3d_viewport_.TopLeftX = static_cast<float>(viewport_->left);
		d3d_viewport_.TopLeftY = static_cast<float>(viewport_->top);
		d3d_viewport_.Width = static_cast<float>(viewport_->width);
		d3d_viewport_.Height = static_cast<float>(viewport_->height);
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
			if (rs_view_)
			{
				rs_view_->ClearDepthStencil(depth, stencil);
			}
		}
		else
		{
			if (flags & CBM_Depth)
			{
				if (rs_view_)
				{
					rs_view_->ClearDepth(depth);
				}
			}

			if (flags & CBM_Stencil)
			{
				if (rs_view_)
				{
					rs_view_->ClearStencil(stencil);
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
			if (rs_view_)
			{
				rs_view_->Discard();
			}
		}
	}

	void D3D12FrameBuffer::BindBarrier()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		std::vector<ID3D12Resource*> rt_src;
		std::vector<uint32_t> rt_first_subres;
		std::vector<uint32_t> rt_num_subres;
		for (uint32_t i = 0; i < clr_views_.size(); ++i)
		{
			if (clr_views_[i])
			{
				D3D12RenderTargetRenderView* p = checked_cast<D3D12RenderTargetRenderView*>(clr_views_[i].get());
				rt_src.push_back(p->RTSrc().get());
				rt_first_subres.push_back(p->RTFirstSubRes());
				rt_num_subres.push_back(p->RTNumSubRes());
			}
		}

		ID3D12Resource* ds_src;
		uint32_t ds_first_subres;
		uint32_t ds_num_subres;
		if (rs_view_)
		{
			D3D12DepthStencilRenderView* p = checked_cast<D3D12DepthStencilRenderView*>(rs_view_.get());
			ds_src = p->DSSrc().get();
			ds_first_subres = p->DSFirstSubRes();
			ds_num_subres = p->DSNumSubRes();
		}
		else
		{
			ds_src = nullptr;
			ds_first_subres = 0;
			ds_num_subres = 0;
		}

		std::vector<D3D12_RESOURCE_BARRIER> barrier_before;
		for (size_t i = 0; i < rt_src.size(); ++ i)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = rt_src[i];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			for (uint32_t j = 0; j < rt_num_subres[i]; ++ j)
			{
				barrier.Transition.Subresource = rt_first_subres[i] + j;
				barrier_before.push_back(barrier);
			}
		}
		if (ds_src)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = ds_src;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			for (uint32_t j = 0; j < ds_num_subres; ++ j)
			{
				barrier.Transition.Subresource = ds_first_subres + j;
				barrier_before.push_back(barrier);
			}
		}
		if (!barrier_before.empty())
		{
			cmd_list->ResourceBarrier(static_cast<UINT>(barrier_before.size()), &barrier_before[0]);
		}
	}

	void D3D12FrameBuffer::UnbindBarrier()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		std::vector<ID3D12Resource*> rt_src;
		std::vector<uint32_t> rt_first_subres;
		std::vector<uint32_t> rt_num_subres;
		for (uint32_t i = 0; i < clr_views_.size(); ++i)
		{
			if (clr_views_[i])
			{
				D3D12RenderTargetRenderView* p = checked_cast<D3D12RenderTargetRenderView*>(clr_views_[i].get());
				rt_src.push_back(p->RTSrc().get());
				rt_first_subres.push_back(p->RTFirstSubRes());
				rt_num_subres.push_back(p->RTNumSubRes());
			}
		}

		ID3D12Resource* ds_src;
		uint32_t ds_first_subres;
		uint32_t ds_num_subres;
		if (rs_view_)
		{
			D3D12DepthStencilRenderView* p = checked_cast<D3D12DepthStencilRenderView*>(rs_view_.get());
			ds_src = p->DSSrc().get();
			ds_first_subres = p->DSFirstSubRes();
			ds_num_subres = p->DSNumSubRes();
		}
		else
		{
			ds_src = nullptr;
			ds_first_subres = 0;
			ds_num_subres = 0;
		}

		std::vector<D3D12_RESOURCE_BARRIER> barrier_after;
		for (size_t i = 0; i < rt_src.size(); ++ i)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = rt_src[i];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			for (uint32_t j = 0; j < rt_num_subres[i]; ++ j)
			{
				barrier.Transition.Subresource = rt_first_subres[i] + j;
				barrier_after.push_back(barrier);
			}
		}
		if (ds_src)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = ds_src;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			for (uint32_t j = 0; j < ds_num_subres; ++ j)
			{
				barrier.Transition.Subresource = ds_first_subres + j;
				barrier_after.push_back(barrier);
			}
		}
		if (!barrier_after.empty())
		{
			cmd_list->ResourceBarrier(static_cast<UINT>(barrier_after.size()), &barrier_after[0]);
		}
	}
}
