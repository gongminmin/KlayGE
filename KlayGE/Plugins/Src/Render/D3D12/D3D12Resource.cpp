/**
 * @file D3D12Resource.cpp
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
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Resource.hpp>

namespace KlayGE
{
	D3D12Resource::D3D12Resource() = default;

	D3D12Resource::~D3D12Resource()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.AddStallResource(d3d_resource_);
		}
	}

	void D3D12Resource::UpdateResourceBarrier(ID3D12GraphicsCommandList* cmd_list, uint32_t sub_res, D3D12_RESOURCE_STATES target_state)
	{
		if (!d3d_resource_)
		{
			return;
		}

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		bool state_changed = false;
		if (sub_res == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			auto const first_state = curr_states_[0];
			bool const same_state = std::all_of(
				curr_states_.begin(), curr_states_.end(), [first_state](D3D12_RESOURCE_STATES state) { return state == first_state; });
			if (same_state)
			{
				if (curr_states_[0] != target_state)
				{
					barrier.Transition.pResource = d3d_resource_.get();
					barrier.Transition.StateBefore = curr_states_[0];
					barrier.Transition.StateAfter = target_state;
					barrier.Transition.Subresource = sub_res;
					std::fill(curr_states_.begin(), curr_states_.end(), target_state);

					re.AddResourceBarrier(cmd_list, MakeSpan<1>(barrier));

					state_changed = true;
				}
			}
			else
			{
				for (uint32_t i = 0; i < curr_states_.size(); ++ i)
				{
					if (curr_states_[i] != target_state)
					{
						barrier.Transition.pResource = d3d_resource_.get();
						barrier.Transition.StateBefore = curr_states_[i];
						barrier.Transition.StateAfter = target_state;
						barrier.Transition.Subresource = i;
						curr_states_[i] = target_state;

						re.AddResourceBarrier(cmd_list, MakeSpan<1>(barrier));

						state_changed = true;
					}
				}
			}
		}
		else
		{
			if (curr_states_[sub_res] != target_state)
			{
				barrier.Transition.pResource = d3d_resource_.get();
				barrier.Transition.StateBefore = curr_states_[sub_res];
				barrier.Transition.StateAfter = target_state;
				barrier.Transition.Subresource = sub_res;
				curr_states_[sub_res] = target_state;

				re.AddResourceBarrier(cmd_list, MakeSpan<1>(barrier));

				state_changed = true;
			}
		}

		if (!state_changed && (target_state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.UAV.pResource = d3d_resource_.get();

			re.AddResourceBarrier(cmd_list, MakeSpan<1>(barrier));
		}
	}
}
