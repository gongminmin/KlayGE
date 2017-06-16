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
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Resource.hpp>

namespace KlayGE
{
	D3D12Resource::D3D12Resource()
	{
	}

	D3D12Resource::~D3D12Resource()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.ForceCPUGPUSync();
		}
	}

	bool D3D12Resource::UpdateResourceBarrier(uint32_t sub_res, D3D12_RESOURCE_BARRIER& barrier, D3D12_RESOURCE_STATES target_state)
	{
		if (sub_res == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
#ifdef KLAYGE_DEBUG
			for (auto state : curr_states_)
			{
				BOOST_ASSERT(state == curr_states_[0]);
			}
#endif

			if (curr_states_[0] == target_state)
			{
				return false;
			}
			else
			{
				barrier.Transition.pResource = d3d_resource_.get();
				barrier.Transition.StateBefore = curr_states_[0];
				barrier.Transition.StateAfter = target_state;
				barrier.Transition.Subresource = sub_res;
				for (auto& state : curr_states_)
				{
					state = target_state;
				}
				return true;
			}
		}
		else
		{
			if (curr_states_[sub_res] == target_state)
			{
				return false;
			}
			else
			{
				barrier.Transition.pResource = d3d_resource_.get();
				barrier.Transition.StateBefore = curr_states_[sub_res];
				barrier.Transition.StateAfter = target_state;
				barrier.Transition.Subresource = sub_res;
				curr_states_[sub_res] = target_state;
				return true;
			}
		}
	}
}
