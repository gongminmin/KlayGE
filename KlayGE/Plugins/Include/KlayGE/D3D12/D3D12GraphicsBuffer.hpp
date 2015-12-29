/**
 * @file D3D12GraphicsBuffer.hpp
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

#ifndef _D3D12GRAPHICSBUFFER_HPP
#define _D3D12GRAPHICSBUFFER_HPP

#pragma once

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>

namespace KlayGE
{
	class D3D12GraphicsBuffer : public GraphicsBuffer
	{
	public:
		D3D12GraphicsBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt);
		virtual ~D3D12GraphicsBuffer();

		ID3D12ResourcePtr const & D3DBuffer() const
		{
			return buffer_;
		}
		ID3D12ResourcePtr const & D3DBufferCounterUpload() const
		{
			return buffer_counter_upload_;
		}

		D3D12ShaderResourceViewSimulationPtr const & D3DShaderResourceView() const
		{
			return d3d_sr_view_;
		}

		D3D12UnorderedAccessViewSimulationPtr const & D3DUnorderedAccessView() const
		{
			return d3d_ua_view_;
		}

		void CopyToBuffer(GraphicsBuffer& rhs);

		virtual void CreateHWResource(void const * init_data) override;
		virtual void DeleteHWResource() override;

		uint32_t CounterOffset() const
		{
			return counter_offset_;
		}

		bool UpdateResourceBarrier(D3D12_RESOURCE_BARRIER& barrier, D3D12_RESOURCE_STATES target_state);

	private:

		void* Map(BufferAccess ba);
		void Unmap();

	private:
		ID3D12ResourcePtr buffer_;
		ID3D12ResourcePtr buffer_counter_upload_;
		D3D12ShaderResourceViewSimulationPtr d3d_sr_view_;
		D3D12UnorderedAccessViewSimulationPtr d3d_ua_view_;
		uint32_t counter_offset_;

		ElementFormat fmt_as_shader_res_;

		D3D12_RESOURCE_STATES curr_state_;
	};
	typedef std::shared_ptr<D3D12GraphicsBuffer> D3D12GraphicsBufferPtr;
}

#endif			// _D3D12GRAPHICSBUFFER_HPP
