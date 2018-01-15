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
#include <KFL/COMPtr.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

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
			re.ReleaseAfterSync(d3d_resource_);
		}
	}

	bool D3D12Resource::UpdateResourceBarrier(uint32_t sub_res, D3D12_RESOURCE_BARRIER& barrier, D3D12_RESOURCE_STATES target_state)
	{
		if (!d3d_resource_)
		{
			return false;
		}

		if (sub_res == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
#ifdef KLAYGE_DEBUG
			for (auto state : curr_states_)
			{
				BOOST_ASSERT(state == curr_states_[0]);
#if defined(KLAYGE_COMPILER_CLANGC2)
				KFL_UNUSED(state);
#endif
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

	ID3D12ResourcePtr D3D12Resource::CreateBuffer(uint32_t access_hint, uint32_t size_in_byte)
	{
		auto& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto device = re.D3DDevice();

		D3D12_RESOURCE_STATES init_state;
		D3D12_HEAP_PROPERTIES heap_prop;
		if (EAH_CPU_Read == access_hint)
		{
			init_state = D3D12_RESOURCE_STATE_COPY_DEST;
			heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((0 == access_hint) || (access_hint & EAH_CPU_Read) || (access_hint & EAH_CPU_Write))
		{
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			init_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		}
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = size_in_byte;
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (access_hint & EAH_GPU_Unordered)
		{
			res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		ID3D12Resource* buffer;
		TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, init_state, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&buffer)));
		return MakeCOMPtr(buffer);
	}
}
