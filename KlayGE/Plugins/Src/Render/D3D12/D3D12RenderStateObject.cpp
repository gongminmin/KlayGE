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

#include <limits>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>

namespace KlayGE
{
	D3D12RasterizerStateObject::D3D12RasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc)
	{
		rasterizer_desc_.FillMode = D3D12Mapping::Mapping(desc.polygon_mode);
		rasterizer_desc_.CullMode = D3D12Mapping::Mapping(desc.cull_mode);
		rasterizer_desc_.FrontCounterClockwise = desc.front_face_ccw;
		rasterizer_desc_.DepthBias = static_cast<int>(desc.polygon_offset_units);
		rasterizer_desc_.DepthBiasClamp = desc.polygon_offset_units;
		rasterizer_desc_.SlopeScaledDepthBias = desc.polygon_offset_factor;
		rasterizer_desc_.DepthClipEnable = desc.depth_clip_enable;
		rasterizer_desc_.MultisampleEnable = desc.multisample_enable;
		rasterizer_desc_.AntialiasedLineEnable = false;
		rasterizer_desc_.ForcedSampleCount = 0;
		rasterizer_desc_.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	void D3D12RasterizerStateObject::Active()
	{
	}

	D3D12DepthStencilStateObject::D3D12DepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
		depth_stencil_desc_.DepthEnable = desc.depth_enable;
		depth_stencil_desc_.DepthWriteMask = D3D12Mapping::Mapping(desc.depth_write_mask);
		depth_stencil_desc_.DepthFunc = D3D12Mapping::Mapping(desc.depth_func);
		depth_stencil_desc_.StencilEnable = desc.front_stencil_enable;
		depth_stencil_desc_.StencilReadMask = static_cast<uint8_t>(desc.front_stencil_read_mask);
		depth_stencil_desc_.StencilWriteMask = static_cast<uint8_t>(desc.front_stencil_write_mask);
		depth_stencil_desc_.FrontFace.StencilFailOp = D3D12Mapping::Mapping(desc.front_stencil_fail);
		depth_stencil_desc_.FrontFace.StencilDepthFailOp = D3D12Mapping::Mapping(desc.front_stencil_depth_fail);
		depth_stencil_desc_.FrontFace.StencilPassOp = D3D12Mapping::Mapping(desc.front_stencil_pass);
		depth_stencil_desc_.FrontFace.StencilFunc = D3D12Mapping::Mapping(desc.front_stencil_func);
		depth_stencil_desc_.BackFace.StencilFailOp = D3D12Mapping::Mapping(desc.back_stencil_fail);
		depth_stencil_desc_.BackFace.StencilDepthFailOp = D3D12Mapping::Mapping(desc.back_stencil_depth_fail);
		depth_stencil_desc_.BackFace.StencilPassOp = D3D12Mapping::Mapping(desc.back_stencil_pass);
		depth_stencil_desc_.BackFace.StencilFunc = D3D12Mapping::Mapping(desc.back_stencil_func);
	}

	void D3D12DepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t /*back_stencil_ref*/)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetStencilRef(front_stencil_ref);
	}

	D3D12BlendStateObject::D3D12BlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = re.DeviceCaps();

		blend_desc_.AlphaToCoverageEnable = desc.alpha_to_coverage_enable;
		blend_desc_.IndependentBlendEnable = desc.independent_blend_enable;
		for (int i = 0; i < 8; ++i)
		{
			uint32_t const rt_index = caps.independent_blend_support ? i : 0;

			blend_desc_.RenderTarget[i].BlendEnable = desc.blend_enable[rt_index];
			blend_desc_.RenderTarget[i].LogicOpEnable = desc.logic_op_enable[rt_index];
			blend_desc_.RenderTarget[i].SrcBlend = D3D12Mapping::Mapping(desc.src_blend[rt_index]);
			blend_desc_.RenderTarget[i].DestBlend = D3D12Mapping::Mapping(desc.dest_blend[rt_index]);
			blend_desc_.RenderTarget[i].BlendOp = D3D12Mapping::Mapping(desc.blend_op[rt_index]);
			blend_desc_.RenderTarget[i].SrcBlendAlpha = D3D12Mapping::Mapping(desc.src_blend_alpha[rt_index]);
			blend_desc_.RenderTarget[i].DestBlendAlpha = D3D12Mapping::Mapping(desc.dest_blend_alpha[rt_index]);
			blend_desc_.RenderTarget[i].BlendOpAlpha = D3D12Mapping::Mapping(desc.blend_op_alpha[rt_index]);
			blend_desc_.RenderTarget[i].LogicOp
				= caps.logic_op_support ? D3D12Mapping::Mapping(desc.logic_op[rt_index]) : D3D12_LOGIC_OP_NOOP;
			blend_desc_.RenderTarget[i].RenderTargetWriteMask
				= static_cast<UINT8>(D3D12Mapping::MappingColorMask(desc.color_write_mask[rt_index]));
		}
	}

	void D3D12BlendStateObject::Active(Color const & blend_factor, uint32_t sample_mask)
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetBlendState(blend_factor, sample_mask);
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
