/**
 * @file D3D12RenderStateObject.hpp
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

#ifndef _D3D12RENDERSTATEOBJECT_HPP
#define _D3D12RENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class D3D12RenderStateObject final : public RenderStateObject
	{
	public:
		D3D12RenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc);

		void Active();

		ID3D12PipelineState* RetrieveGraphicsPSO(RenderLayout const & rl, ShaderObject const & so, FrameBuffer const & fb,
			bool has_tessellation) const;
		ID3D12PipelineState* RetrieveComputePSO(ShaderObject const & so) const;

		D3D12_RASTERIZER_DESC const & D3DRasterizerDesc() const
		{
			return ps_desc_.graphics_ps_desc.RasterizerState;
		}
		D3D12_DEPTH_STENCIL_DESC const & D3DDepthStencilDesc() const
		{
			return ps_desc_.graphics_ps_desc.DepthStencilState;
		}
		D3D12_BLEND_DESC const & D3DBlendDesc() const
		{
			return ps_desc_.graphics_ps_desc.BlendState;
		}

	private:
		union PipelineStateDesc
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC graphics_ps_desc;
			D3D12_COMPUTE_PIPELINE_STATE_DESC compute_ps_desc;
		};
		PipelineStateDesc ps_desc_;

		mutable std::unordered_map<size_t, ID3D12PipelineStatePtr> psos_;
	};

	class D3D12SamplerStateObject final : public SamplerStateObject
	{
	public:
		explicit D3D12SamplerStateObject(SamplerStateDesc const & desc);

		D3D12_SAMPLER_DESC const & D3DDesc() const
		{
			return sampler_desc_;
		}

	private:
		D3D12_SAMPLER_DESC sampler_desc_;
	};
}

#endif			// _D3D12RENDERSTATEOBJECT_HPP
