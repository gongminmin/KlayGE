/**
 * @file D3D11Fence.cpp
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

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Fence.hpp>

namespace KlayGE
{
	D3D11Fence::D3D11Fence() = default;

	uint64_t D3D11Fence::Signal(FenceType ft)
	{
		KFL_UNUSED(ft);

		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device1* d3d_device = re.D3DDevice1();
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_EVENT;
		desc.MiscFlags = 0;

		ID3D11QueryPtr query;
		d3d_device->CreateQuery(&desc, query.put());

		uint64_t const id = fence_val_;
		++ fence_val_;

		d3d_imm_ctx->End(query.get());

		fences_[id] = std::move(query);

		return id;
	}

	void D3D11Fence::Wait(uint64_t id)
	{
		auto iter = fences_.find(id);
		if (iter != fences_.end())
		{
			auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

			uint32_t ret;
			while (S_OK != d3d_imm_ctx->GetData(iter->second.get(), &ret, sizeof(ret), 0));
			fences_.erase(iter);
		}
	}

	bool D3D11Fence::Completed(uint64_t id)
	{
		auto iter = fences_.find(id);
		if (iter == fences_.end())
		{
			return true;
		}
		else
		{
			auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

			uint32_t ret;
			HRESULT hr = d3d_imm_ctx->GetData(iter->second.get(), &ret, sizeof(ret), 0);
			return (S_OK == hr);
		}
	}


	D3D11_4Fence::D3D11_4Fence()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_device = re.D3DDevice5();
		BOOST_ASSERT(d3d_device != nullptr);

		d3d_device->CreateFence(0, D3D11_FENCE_FLAG_NONE, IID_ID3D11Fence, fence_.put_void());

		fence_event_ = MakeWin32UniqueHandle(::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS));
	}

	uint64_t D3D11_4Fence::Signal(FenceType ft)
	{
		KFL_UNUSED(ft);

		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_imm_ctx = re.D3DDeviceImmContext4();
		BOOST_ASSERT(d3d_imm_ctx != nullptr);

		uint64_t const id = fence_val_;
		d3d_imm_ctx->Signal(fence_.get(), id);
		++ fence_val_;

		return id;
	}

	void D3D11_4Fence::Wait(uint64_t id)
	{
		if (!this->Completed(id))
		{
			TIFHR(fence_->SetEventOnCompletion(id, fence_event_.get()));
			::WaitForSingleObjectEx(fence_event_.get(), INFINITE, FALSE);
		}
	}

	bool D3D11_4Fence::Completed(uint64_t id)
	{
		if (id > last_completed_val_)
		{
			last_completed_val_ = std::max(last_completed_val_, fence_->GetCompletedValue());
		}
		return id <= last_completed_val_;
	}
}
