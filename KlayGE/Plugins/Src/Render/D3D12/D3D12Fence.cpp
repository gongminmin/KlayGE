/**
 * @file D3D12Fence.cpp
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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Fence.hpp>

namespace KlayGE
{
	D3D12Fence::D3D12Fence()
			: last_completed_val_(0), fence_val_(1)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		ID3D12Fence* fence;
		TIF(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, reinterpret_cast<void**>(&fence)));
		fence_ = MakeCOMPtr(fence);

		fence_event_ = ::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	}

	D3D12Fence::~D3D12Fence()
	{
		::CloseHandle(fence_event_);
	}

	uint64_t D3D12Fence::Signal(FenceType ft)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		ID3D12CommandQueuePtr cmd_queue;
		switch (ft)
		{
		case FT_Render:
			cmd_queue = re.D3DRenderCmdQueue();
			break;

		case FT_Compute:
			cmd_queue = re.D3DComputeCmdQueue();
			break;

		case FT_Copy:
			cmd_queue = re.D3DCopyCmdQueue();
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		uint64_t const id = fence_val_;
		TIF(cmd_queue->Signal(fence_.get(), id));
		++ fence_val_;
		return id;
	}

	void D3D12Fence::Wait(uint64_t id)
	{
		if (!this->Completed(id))
		{
			TIF(fence_->SetEventOnCompletion(id, fence_event_));
			::WaitForSingleObjectEx(fence_event_, INFINITE, FALSE);
		}
	}

	bool D3D12Fence::Completed(uint64_t id)
	{
		if (id > last_completed_val_)
		{
			last_completed_val_ = std::max(last_completed_val_, fence_->GetCompletedValue());
		}
		return id <= last_completed_val_;
	}
}
