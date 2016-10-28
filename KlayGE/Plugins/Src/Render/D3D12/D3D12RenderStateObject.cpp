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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KFL/Hash.hpp>

#include <limits>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>

namespace KlayGE
{
	D3D12RenderStateObject::D3D12RenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc)
		: RenderStateObject(rs_desc, dss_desc, bs_desc)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = re.DeviceCaps();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC& graphics_ps_desc = ps_desc_.graphics_ps_desc;

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
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetStencilRef(dss_desc_.front_stencil_ref);
		re.OMSetBlendFactor(bs_desc_.blend_factor);
	}

	ID3D12PipelineStatePtr D3D12RenderStateObject::RetrieveGraphicsPSO(RenderLayout const & rl, ShaderObjectPtr const & so,
		FrameBufferPtr const & fb, bool has_tessellation) const
	{
		D3D12ShaderObjectPtr const & d3d12_so = checked_pointer_cast<D3D12ShaderObject>(so);
		D3D12RenderLayout const & d3d12_rl = *checked_cast<D3D12RenderLayout const *>(&rl);

		size_t hash_val = 0;
		{
			HashCombine(hash_val, 'V');
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_VertexShader);
				if (blob && !blob->empty())
				{
					HashCombine(hash_val, blob->data());
				}
				else
				{
					HashCombine(hash_val, 0);
				}
			}
			HashCombine(hash_val, 'P');
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_PixelShader);
				if (blob && !blob->empty())
				{
					HashCombine(hash_val, blob->data());
				}
				else
				{
					HashCombine(hash_val, 0);
				}
			}
			HashCombine(hash_val, 'D');
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_DomainShader);
				if (blob && !blob->empty())
				{
					HashCombine(hash_val, blob->data());
				}
				else
				{
					HashCombine(hash_val, 0);
				}
			}
			HashCombine(hash_val, 'H');
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_HullShader);
				if (blob && !blob->empty())
				{
					HashCombine(hash_val, blob->data());
				}
				else
				{
					HashCombine(hash_val, 0);
				}
			}
			HashCombine(hash_val, 'G');
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_GeometryShader);
				if (blob && !blob->empty())
				{
					HashCombine(hash_val, blob->data());
				}
				else
				{
					HashCombine(hash_val, 0);
				}
			}

			auto const & so_decls = d3d12_so->SODecl();
			std::vector<UINT> so_strides(so_decls.size());
			for (size_t i = 0; i < so_decls.size(); ++ i)
			{
				char const * p = reinterpret_cast<char const *>(&so_decls[i]);
				HashRange(hash_val, p, p + sizeof(so_decls[i]));
			}
			HashCombine(hash_val, d3d12_so->RasterizedStream());

			HashCombine(hash_val, 'I');
			auto const & input_elem_desc = d3d12_rl.InputElementDesc();
			for (size_t i = 0; i < input_elem_desc.size(); ++ i)
			{
				char const * p = reinterpret_cast<char const *>(&input_elem_desc[i]);
				HashRange(hash_val, p, p + sizeof(input_elem_desc[i]));
			}
			HashCombine(hash_val, rl.IndexStreamFormat());

			HashCombine(hash_val, rl.TopologyType());
			HashCombine(hash_val, has_tessellation);

			for (uint32_t i = 0; i < 8; ++ i)
			{
				auto const & view = fb->Attached(FrameBuffer::ATT_Color0 + i);
				if (view)
				{
					HashCombine(hash_val, view->Format());
				}
			}
			{
				auto const & view = fb->Attached(FrameBuffer::ATT_DepthStencil);
				if (view)
				{
					HashCombine(hash_val, view->Format());
				}
			}
		}

		auto iter = psos_.find(hash_val);
		if (iter == psos_.end())
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = ps_desc_.graphics_ps_desc;
			pso_desc.pRootSignature = d3d12_so->RootSignature().get();
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_VertexShader);
				if (blob && !blob->empty())
				{
					pso_desc.VS.pShaderBytecode = blob->data();
					pso_desc.VS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.VS.pShaderBytecode = nullptr;
					pso_desc.VS.BytecodeLength = 0;
				}
			}
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_PixelShader);
				if (blob && !blob->empty())
				{
					pso_desc.PS.pShaderBytecode = blob->data();
					pso_desc.PS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.PS.pShaderBytecode = nullptr;
					pso_desc.PS.BytecodeLength = 0;
				}
			}
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_DomainShader);
				if (blob && !blob->empty())
				{
					pso_desc.DS.pShaderBytecode = blob->data();
					pso_desc.DS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.DS.pShaderBytecode = nullptr;
					pso_desc.DS.BytecodeLength = 0;
				}
			}
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_HullShader);
				if (blob && !blob->empty())
				{
					pso_desc.HS.pShaderBytecode = blob->data();
					pso_desc.HS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.HS.pShaderBytecode = nullptr;
					pso_desc.HS.BytecodeLength = 0;
				}
			}
			{
				auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_GeometryShader);
				if (blob && !blob->empty())
				{
					pso_desc.GS.pShaderBytecode = blob->data();
					pso_desc.GS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.GS.pShaderBytecode = nullptr;
					pso_desc.GS.BytecodeLength = 0;
				}
			}

			auto const & so_decls = d3d12_so->SODecl();
			std::vector<UINT> so_strides(so_decls.size());
			for (size_t i = 0; i < so_decls.size(); ++ i)
			{
				so_strides[i] = so_decls[i].ComponentCount * sizeof(float);
			}
			pso_desc.StreamOutput.pSODeclaration = so_decls.empty() ? nullptr : &so_decls[0];
			pso_desc.StreamOutput.NumEntries = static_cast<UINT>(so_decls.size());
			pso_desc.StreamOutput.pBufferStrides = so_strides.empty() ? nullptr : &so_strides[0];
			pso_desc.StreamOutput.NumStrides = static_cast<UINT>(so_strides.size());
			pso_desc.StreamOutput.RasterizedStream = d3d12_so->RasterizedStream();

			pso_desc.InputLayout.pInputElementDescs = d3d12_rl.InputElementDesc().empty() ? nullptr : &d3d12_rl.InputElementDesc()[0];
			pso_desc.InputLayout.NumElements = static_cast<UINT>(d3d12_rl.InputElementDesc().size());
			pso_desc.IBStripCutValue = (EF_R16UI == rl.IndexStreamFormat())
				? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

			RenderLayout::topology_type tt = rl.TopologyType();
			if (has_tessellation)
			{
				switch (tt)
				{
				case RenderLayout::TT_PointList:
					tt = RenderLayout::TT_1_Ctrl_Pt_PatchList;
					break;

				case RenderLayout::TT_LineList:
					tt = RenderLayout::TT_2_Ctrl_Pt_PatchList;
					break;

				case RenderLayout::TT_TriangleList:
					tt = RenderLayout::TT_3_Ctrl_Pt_PatchList;
					break;

				default:
					break;
				}
			}
			pso_desc.PrimitiveTopologyType = D3D12Mapping::MappingPriTopoType(tt);

			pso_desc.NumRenderTargets = 0;
			for (int i = sizeof(pso_desc.RTVFormats) / sizeof(pso_desc.RTVFormats[0]) - 1; i >= 0; -- i)
			{
				if (fb->Attached(FrameBuffer::ATT_Color0 + i))
				{
					pso_desc.NumRenderTargets = i + 1;
					break;
				}
			}
			for (uint32_t i = 0; i < pso_desc.NumRenderTargets; ++ i)
			{
				pso_desc.RTVFormats[i] = D3D12Mapping::MappingFormat(fb->Attached(FrameBuffer::ATT_Color0 + i)->Format());
			}
			for (uint32_t i = pso_desc.NumRenderTargets; i < sizeof(pso_desc.RTVFormats) / sizeof(pso_desc.RTVFormats[0]); ++ i)
			{
				pso_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}
			if (fb->Attached(FrameBuffer::ATT_DepthStencil))
			{
				pso_desc.DSVFormat = D3D12Mapping::MappingFormat(fb->Attached(FrameBuffer::ATT_DepthStencil)->Format());
			}
			else
			{
				pso_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			}
			pso_desc.SampleDesc.Count = 1;
			pso_desc.SampleDesc.Quality = 0;
		
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto d3d_device = re.D3DDevice().get();

			ID3D12PipelineState* d3d_pso;
			TIF(d3d_device->CreateGraphicsPipelineState(&pso_desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&d3d_pso)));
			return psos_.emplace(hash_val, MakeCOMPtr(d3d_pso)).first->second;
		}
		else
		{
			return iter->second;
		}
	}

	ID3D12PipelineStatePtr D3D12RenderStateObject::RetrieveComputePSO(ShaderObjectPtr const & so) const
	{
		D3D12ShaderObjectPtr const & d3d12_so = checked_pointer_cast<D3D12ShaderObject>(so);

		size_t hash_val = 0;
		HashCombine(hash_val, 'C');
		auto const & blob = d3d12_so->ShaderBlob(ShaderObject::ST_ComputeShader);
		if (blob && !blob->empty())
		{
			HashCombine(hash_val, blob->data());
		}
		else
		{
			HashCombine(hash_val, 0);
		}

		auto iter = psos_.find(hash_val);
		if (iter == psos_.end())
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc;
			pso_desc.pRootSignature = d3d12_so->RootSignature().get();
			{
				if (blob && !blob->empty())
				{
					pso_desc.CS.pShaderBytecode = blob->data();
					pso_desc.CS.BytecodeLength = static_cast<UINT>(blob->size());
				}
				else
				{
					pso_desc.CS.pShaderBytecode = nullptr;
					pso_desc.CS.BytecodeLength = 0;
				}
			}
			pso_desc.NodeMask = 0;
			pso_desc.CachedPSO.pCachedBlob = nullptr;
			pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
			pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto d3d_device = re.D3DDevice().get();

			ID3D12PipelineState* d3d_pso;
			TIF(d3d_device->CreateComputePipelineState(&pso_desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&d3d_pso)));
			return psos_.emplace(hash_val, MakeCOMPtr(d3d_pso)).first->second;
		}
		else
		{
			return iter->second;
		}
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
