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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Fence.hpp>

namespace KlayGE
{
	D3D12Fence::D3D12Fence()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		TIFHR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, fence_.put_void()));

		fence_event_ = MakeWin32UniqueHandle(::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS));
	}

	uint64_t D3D12Fence::Signal(FenceType ft)
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12CommandQueue* cmd_queue;
		switch (ft)
		{
		case FT_Render:
			cmd_queue = re.D3DCmdQueue();
			break;

		default:
			KFL_UNREACHABLE("Invalid fence type");
		}

		return this->Signal(cmd_queue);
	}

	uint64_t D3D12Fence::Signal(ID3D12CommandQueue* cmd_queue)
	{
		uint64_t const id = fence_val_;
		TIFHR(cmd_queue->Signal(fence_.get(), id));
		++ fence_val_;
		return id;
	}

	void D3D12Fence::Wait(uint64_t id)
	{
		if (!this->Completed(id))
		{
			TIFHR(fence_->SetEventOnCompletion(id, fence_event_.get()));
			::WaitForSingleObjectEx(fence_event_.get(), INFINITE, FALSE);
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
