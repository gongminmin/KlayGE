/**
 * @file D3D12FrameBuffer.hpp
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

#ifndef _D3D12FRAMEBUFFER_HPP
#define _D3D12FRAMEBUFFER_HPP

#pragma once

#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	class D3D12Resource;

	class D3D12FrameBuffer : public FrameBuffer
	{
	public:
		D3D12FrameBuffer();
		virtual ~D3D12FrameBuffer();

		virtual std::wstring const & Description() const;

		void OnBind() override;
		void OnUnbind() override;

		void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil);
		virtual void Discard(uint32_t flags) override;

		virtual void BindBarrier();

		virtual void SetRenderTargets();

		size_t PsoHashValue();
		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc);

	private:
		void UpdateViewPointers();

	protected:
		std::vector<D3D12Resource*> d3d_rt_src_;
		std::vector<uint32_t> d3d_rt_first_subres_;
		std::vector<uint32_t> d3d_rt_num_subres_;

		D3D12Resource* d3d_ds_src_;
		uint32_t d3d_ds_first_subres_;
		uint32_t d3d_ds_num_subres_;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> d3d_rt_handles_;
		D3D12_CPU_DESCRIPTOR_HANDLE d3d_ds_handle_;
		D3D12_CPU_DESCRIPTOR_HANDLE* d3d_ds_handle_ptr_;

		D3D12_VIEWPORT d3d_viewport_;

		// For PSOs
		size_t pso_hash_value_;
		uint32_t num_rts_;
		std::array<DXGI_FORMAT, 8> rtv_formats_;
		DXGI_FORMAT dsv_format_;
		uint32_t sample_count_;
		uint32_t sample_quality_;
	};

	typedef std::shared_ptr<D3D12FrameBuffer> D3D12FrameBufferPtr;
}

#endif			// _D3D12FRAMEBUFFER_HPP
