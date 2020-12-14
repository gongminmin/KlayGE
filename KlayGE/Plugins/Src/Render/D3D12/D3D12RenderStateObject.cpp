/**
 * @file D3D12RenderStateObject.cpp
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
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KFL/Hash.hpp>

#include <iterator>
#include <limits>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>

namespace KlayGE
{
	D3D12RenderStateObject::D3D12RenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc)
		: RenderStateObject(rs_desc, dss_desc, bs_desc)
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = re.DeviceCaps();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC& graphics_ps_desc = std::get<D3D12_GRAPHICS_PIPELINE_STATE_DESC>(ps_desc_);

		graphics_ps_desc.RasterizerState.FillMode = D3D12Mapping::Mapping(rs_desc.polygon_mode);
		graphics_ps_desc.RasterizerState.CullMode = D3D12Mapping::Mapping(rs_desc.cull_mode);
		graphics_ps_desc.RasterizerState.FrontCounterClockwise = rs_desc.front_face_ccw;
		graphics_ps_desc.RasterizerState.DepthBias = static_cast<int>(rs_desc.polygon_offset_units);
		graphics_ps_desc.RasterizerState.DepthBiasClamp = rs_desc.polygon_offset_units;
		graphics_ps_desc.RasterizerState.SlopeScaledDepthBias = rs_desc.polygon_offset_factor;
		graphics_ps_desc.RasterizerState.DepthClipEnable = rs_desc.depth_clip_enable;
		graphics_ps_desc.RasterizerState.MultisampleEnable = rs_desc.multisample_enable;
		graphics_ps_desc.RasterizerState.AntialiasedLineEnable = false;
		graphics_ps_desc.RasterizerState.ForcedSampleCount = 0;
		graphics_ps_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		graphics_ps_desc.DepthStencilState.DepthEnable = dss_desc.depth_enable;
		graphics_ps_desc.DepthStencilState.DepthWriteMask = D3D12Mapping::Mapping(dss_desc.depth_write_mask);
		graphics_ps_desc.DepthStencilState.DepthFunc = D3D12Mapping::Mapping(dss_desc.depth_func);
		graphics_ps_desc.DepthStencilState.StencilEnable = dss_desc.front_stencil_enable;
		graphics_ps_desc.DepthStencilState.StencilReadMask = static_cast<uint8_t>(dss_desc.front_stencil_read_mask);
		graphics_ps_desc.DepthStencilState.StencilWriteMask = static_cast<uint8_t>(dss_desc.front_stencil_write_mask);
		graphics_ps_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12Mapping::Mapping(dss_desc.front_stencil_fail);
		graphics_ps_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12Mapping::Mapping(dss_desc.front_stencil_depth_fail);
		graphics_ps_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12Mapping::Mapping(dss_desc.front_stencil_pass);
		graphics_ps_desc.DepthStencilState.FrontFace.StencilFunc = D3D12Mapping::Mapping(dss_desc.front_stencil_func);
		graphics_ps_desc.DepthStencilState.BackFace.StencilFailOp = D3D12Mapping::Mapping(dss_desc.back_stencil_fail);
		graphics_ps_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12Mapping::Mapping(dss_desc.back_stencil_depth_fail);
		graphics_ps_desc.DepthStencilState.BackFace.StencilPassOp = D3D12Mapping::Mapping(dss_desc.back_stencil_pass);
		graphics_ps_desc.DepthStencilState.BackFace.StencilFunc = D3D12Mapping::Mapping(dss_desc.back_stencil_func);

		graphics_ps_desc.BlendState.AlphaToCoverageEnable = bs_desc.alpha_to_coverage_enable;
		graphics_ps_desc.BlendState.IndependentBlendEnable = bs_desc.independent_blend_enable;
		for (int i = 0; i < 8; ++ i)
		{
			uint32_t const rt_index = caps.independent_blend_support ? i : 0;

			graphics_ps_desc.BlendState.RenderTarget[i].BlendEnable = bs_desc.blend_enable[rt_index];
			graphics_ps_desc.BlendState.RenderTarget[i].LogicOpEnable = bs_desc.logic_op_enable[rt_index];
			graphics_ps_desc.BlendState.RenderTarget[i].SrcBlend = D3D12Mapping::Mapping(bs_desc.src_blend[rt_index]);
			graphics_ps_desc.BlendState.RenderTarget[i].DestBlend = D3D12Mapping::Mapping(bs_desc.dest_blend[rt_index]);
			graphics_ps_desc.BlendState.RenderTarget[i].BlendOp = D3D12Mapping::Mapping(bs_desc.blend_op[rt_index]);
			graphics_ps_desc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12Mapping::Mapping(bs_desc.src_blend_alpha[rt_index]);
			graphics_ps_desc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12Mapping::Mapping(bs_desc.dest_blend_alpha[rt_index]);
			graphics_ps_desc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12Mapping::Mapping(bs_desc.blend_op_alpha[rt_index]);
			graphics_ps_desc.BlendState.RenderTarget[i].LogicOp
				= caps.logic_op_support ? D3D12Mapping::Mapping(bs_desc.logic_op[rt_index]) : D3D12_LOGIC_OP_NOOP;
			graphics_ps_desc.BlendState.RenderTarget[i].RenderTargetWriteMask
				= static_cast<UINT8>(D3D12Mapping::MappingColorMask(bs_desc.color_write_mask[rt_index]));
		}
		graphics_ps_desc.SampleMask = bs_desc.sample_mask;

		graphics_ps_desc.NodeMask = 0;
		graphics_ps_desc.CachedPSO.pCachedBlob = nullptr;
		graphics_ps_desc.CachedPSO.CachedBlobSizeInBytes = 0;
		graphics_ps_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}

	void D3D12RenderStateObject::Active()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* cmd_list = re.D3DRenderCmdList();
		re.OMSetStencilRef(cmd_list, dss_desc_.front_stencil_ref);
		re.OMSetBlendFactor(cmd_list, bs_desc_.blend_factor);
	}

	ID3D12PipelineState* D3D12RenderStateObject::RetrieveGraphicsPSO(RenderLayout const & rl, ShaderObject const & so,
		FrameBuffer const & fb, bool has_tessellation) const
	{
		auto& d3d12_so = checked_cast<D3D12ShaderObject&>(const_cast<ShaderObject&>(so));
		auto& d3d12_rl = checked_cast<D3D12RenderLayout&>(const_cast<RenderLayout&>(rl));
		auto& d3d12_fb = checked_cast<D3D12FrameBuffer&>(const_cast<FrameBuffer&>(fb));

		size_t hash_val = 0;
		HashCombine(hash_val, d3d12_rl.PsoHashValue());
		HashCombine(hash_val, d3d12_so.GetD3D12ShaderObjectTemplate());
		HashCombine(hash_val, d3d12_fb.PsoHashValue());
		HashCombine(hash_val, has_tessellation);

		auto iter = psos_.find(hash_val);
		if (iter == psos_.end())
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = std::get<D3D12_GRAPHICS_PIPELINE_STATE_DESC>(ps_desc_);

			d3d12_rl.UpdatePsoDesc(pso_desc, has_tessellation);
			d3d12_so.UpdatePsoDesc(pso_desc);
			d3d12_fb.UpdatePsoDesc(pso_desc);
		
			auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto d3d_device = d3d12_re.D3DDevice();

			com_ptr<ID3D12PipelineState> d3d_pso;
			TIFHR(d3d_device->CreateGraphicsPipelineState(&pso_desc, IID_ID3D12PipelineState, d3d_pso.put_void()));
			iter = psos_.emplace(hash_val, std::move(d3d_pso)).first;
		}

		return iter->second.get();
	}

	ID3D12PipelineState* D3D12RenderStateObject::RetrieveComputePSO(ShaderObject const & so) const
	{
		auto& d3d12_so = checked_cast<D3D12ShaderObject&>(const_cast<ShaderObject&>(so));

		size_t hash_val = 0;
		HashCombine(hash_val, d3d12_so.GetD3D12ShaderObjectTemplate());

		auto iter = psos_.find(hash_val);
		if (iter == psos_.end())
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc;
			d3d12_so.UpdatePsoDesc(pso_desc);
			pso_desc.NodeMask = 0;
			pso_desc.CachedPSO.pCachedBlob = nullptr;
			pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
			pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		
			auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto d3d_device = d3d12_re.D3DDevice();

			com_ptr<ID3D12PipelineState> d3d_pso;
			TIFHR(d3d_device->CreateComputePipelineState(&pso_desc, IID_ID3D12PipelineState, d3d_pso.put_void()));
			iter = psos_.emplace(hash_val, std::move(d3d_pso)).first;
		}

		return iter->second.get();
	}


	D3D12SamplerStateObject::D3D12SamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc)
	{
		sampler_desc_.Filter = D3D12Mapping::Mapping(desc.filter);
		sampler_desc_.AddressU = D3D12Mapping::Mapping(desc.addr_mode_u);
		sampler_desc_.AddressV = D3D12Mapping::Mapping(desc.addr_mode_v);
		sampler_desc_.AddressW = D3D12Mapping::Mapping(desc.addr_mode_w);
		sampler_desc_.MipLODBias = desc.mip_map_lod_bias;
		sampler_desc_.MaxAnisotropy = desc.max_anisotropy;
		sampler_desc_.ComparisonFunc = D3D12Mapping::Mapping(desc.cmp_func);
		sampler_desc_.BorderColor[0] = desc.border_clr.r();
		sampler_desc_.BorderColor[1] = desc.border_clr.g();
		sampler_desc_.BorderColor[2] = desc.border_clr.b();
		sampler_desc_.BorderColor[3] = desc.border_clr.a();
		sampler_desc_.MinLOD = desc.min_lod;
		sampler_desc_.MaxLOD = desc.max_lod;
	}
}
