/**
 * @file D3D12RenderView.hpp
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

#ifndef _D3D12RENDERVIEW_HPP
#define _D3D12RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	class D3D12Texture1D;
	class D3D12Texture2D;
	class D3D12Texture3D;
	class D3D12TextureCube;
	class D3D12GraphicsBuffer;

	class D3D12ShaderResourceViewSimulation
	{
	public:
		D3D12ShaderResourceViewSimulation(ID3D12ResourcePtr const & res, D3D12_SHADER_RESOURCE_VIEW_DESC const & srv_desc);
		~D3D12ShaderResourceViewSimulation();

		D3D12_CPU_DESCRIPTOR_HANDLE const & Handle() const
		{
			return handle_;
		}

	private:
		ID3D12ResourcePtr res_;
		D3D12_CPU_DESCRIPTOR_HANDLE handle_;
	};
	typedef std::shared_ptr<D3D12ShaderResourceViewSimulation> D3D12ShaderResourceViewSimulationPtr;

	class D3D12RenderTargetViewSimulation
	{
	public:
		D3D12RenderTargetViewSimulation(ID3D12ResourcePtr const & res, D3D12_RENDER_TARGET_VIEW_DESC const & rtv_desc);
		~D3D12RenderTargetViewSimulation();

		D3D12_CPU_DESCRIPTOR_HANDLE const & Handle() const
		{
			return handle_;
		}

	private:
		ID3D12ResourcePtr res_;
		D3D12_CPU_DESCRIPTOR_HANDLE handle_;
	};
	typedef std::shared_ptr<D3D12RenderTargetViewSimulation> D3D12RenderTargetViewSimulationPtr;

	class D3D12DepthStencilViewSimulation
	{
	public:
		D3D12DepthStencilViewSimulation(ID3D12ResourcePtr const & res, D3D12_DEPTH_STENCIL_VIEW_DESC const & dsv_desc);
		~D3D12DepthStencilViewSimulation();

		D3D12_CPU_DESCRIPTOR_HANDLE const & Handle() const
		{
			return handle_;
		}

	private:
		ID3D12ResourcePtr res_;
		D3D12_CPU_DESCRIPTOR_HANDLE handle_;
	};
	typedef std::shared_ptr<D3D12DepthStencilViewSimulation> D3D12DepthStencilViewSimulationPtr;

	class D3D12UnorderedAccessViewSimulation
	{
	public:
		D3D12UnorderedAccessViewSimulation(ID3D12ResourcePtr const & res, D3D12_UNORDERED_ACCESS_VIEW_DESC const & uav_desc);
		~D3D12UnorderedAccessViewSimulation();

		D3D12_CPU_DESCRIPTOR_HANDLE const & Handle() const
		{
			return handle_;
		}

	private:
		ID3D12ResourcePtr res_;
		ID3D12ResourcePtr counter_;
		D3D12_CPU_DESCRIPTOR_HANDLE handle_;
		uint32_t counter_offset_;
	};
	typedef std::shared_ptr<D3D12UnorderedAccessViewSimulation> D3D12UnorderedAccessViewSimulationPtr;

	class D3D12RenderView : public RenderView
	{
	public:
		D3D12RenderView();
		virtual ~D3D12RenderView();

	protected:
		ID3D12DevicePtr d3d_device_;
		ID3D12GraphicsCommandListPtr d3d_cmd_list_;
	};
	typedef std::shared_ptr<D3D12RenderView> D3D12RenderViewPtr;

	class D3D12RenderTargetRenderView : public D3D12RenderView
	{
	public:
		D3D12RenderTargetRenderView(Texture& texture_1d_2d_cube, int first_array_index, int array_size, int level);
		D3D12RenderTargetRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D12RenderTargetRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D12RenderTargetRenderView(GraphicsBuffer& gb, uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		D3D12RenderTargetViewSimulationPtr D3DRenderTargetView() const
		{
			return rt_view_;
		}

		ID3D12ResourcePtr const & RTSrc() const
		{
			return rt_src_;
		}
		uint32_t RTFirstSubRes() const
		{
			return rt_first_subres_;
		}
		uint32_t RTNumSubRes() const
		{
			return rt_num_subres_;
		}

	private:
		D3D12RenderTargetViewSimulationPtr rt_view_;
		ID3D12ResourcePtr rt_src_;
		uint32_t rt_first_subres_;
		uint32_t rt_num_subres_;
	};
	typedef std::shared_ptr<D3D12RenderTargetRenderView> D3D12RenderTargetRenderViewPtr;

	class D3D12DepthStencilRenderView : public D3D12RenderView
	{
	public:
		D3D12DepthStencilRenderView(Texture& texture_1d_2d_cube, int first_array_index, int array_size, int level);
		D3D12DepthStencilRenderView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D12DepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D12DepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		D3D12DepthStencilViewSimulationPtr D3DDepthStencilView() const
		{
			return ds_view_;
		}

		ID3D12ResourcePtr const & DSSrc() const
		{
			return ds_src_;
		}
		uint32_t DSFirstSubRes() const
		{
			return ds_first_subres_;
		}
		uint32_t DSNumSubRes() const
		{
			return ds_num_subres_;
		}

	private:
		D3D12DepthStencilViewSimulationPtr ds_view_;
		ID3D12ResourcePtr ds_src_;
		uint32_t ds_first_subres_;
		uint32_t ds_num_subres_;
	};
	typedef std::shared_ptr<D3D12DepthStencilRenderView> D3D12DepthStencilRenderViewPtr;


	class D3D12UnorderedAccessView : public UnorderedAccessView
	{
	public:
		D3D12UnorderedAccessView(Texture& texture_1d_2d_cube, int first_array_index, int array_size, int level);
		D3D12UnorderedAccessView(Texture& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
		D3D12UnorderedAccessView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level);
		D3D12UnorderedAccessView(GraphicsBuffer& gb, ElementFormat pf);

		void Clear(float4 const & val);
		void Clear(uint4 const & val);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		D3D12UnorderedAccessViewSimulationPtr const & D3DUnorderedAccessView() const
		{
			return ua_view_;
		}

		ID3D12ResourcePtr const & UASrc() const
		{
			return ua_src_;
		}
		uint32_t UAFirstSubRes() const
		{
			return ua_first_subres_;
		}
		uint32_t UANumSubRes() const
		{
			return ua_num_subres_;
		}

		void ResetInitCount();

	private:
		ID3D12DevicePtr d3d_device_;
		ID3D12GraphicsCommandListPtr d3d_cmd_list_;

		D3D12UnorderedAccessViewSimulationPtr ua_view_;
		ID3D12ResourcePtr ua_src_;
		ID3D12ResourcePtr ua_counter_upload_src_;
		D3D12_RESOURCE_STATES ua_src_init_state_;
		uint32_t ua_first_subres_;
		uint32_t ua_num_subres_;
		uint32_t counter_offset_;

		float4 clear_f4_val_;
		uint4 clear_ui4_val_;
	};
	typedef std::shared_ptr<D3D12UnorderedAccessView> D3D12UnorderedAccessViewPtr;
}

#endif			// _D3D12RENDERVIEW_HPP
