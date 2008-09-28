// D3D10RenderStateObject.cpp
// KlayGE D3D10渲染状态对象类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <numeric>

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10Mapping.hpp>
#include <KlayGE/D3D10/D3D10RenderStateObject.hpp>

namespace KlayGE
{
	D3D10RasterizerStateObject::D3D10RasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc)
	{
		D3D10_RASTERIZER_DESC d3d_desc;
		d3d_desc.FillMode = D3D10Mapping::Mapping(desc.polygon_mode);
		d3d_desc.CullMode = D3D10Mapping::Mapping(desc.cull_mode);
		d3d_desc.FrontCounterClockwise = D3D10Mapping::Mapping(desc.front_face_ccw);
		d3d_desc.DepthBias = static_cast<int>(desc.polygon_offset_units);
		d3d_desc.DepthBiasClamp = desc.polygon_offset_units;
		d3d_desc.SlopeScaledDepthBias = desc.polygon_offset_factor;
		d3d_desc.DepthClipEnable = false;
		d3d_desc.ScissorEnable = desc.scissor_enable;
		d3d_desc.MultisampleEnable = desc.multisample_enable;
		d3d_desc.AntialiasedLineEnable = desc.multisample_enable;

		ID3D10DevicePtr const & d3d_device = checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		ID3D10RasterizerState* rasterizer_state;
		d3d_device->CreateRasterizerState(&d3d_desc, &rasterizer_state);
		rasterizer_state_ = MakeCOMPtr(rasterizer_state);
	}

	void D3D10RasterizerStateObject::Active()
	{
		D3D10RenderEngine& re = *checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.RSSetState(rasterizer_state_);
	}

	D3D10DepthStencilStateObject::D3D10DepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
		D3D10_DEPTH_STENCIL_DESC d3d_desc;
		d3d_desc.DepthEnable = desc.depth_enable;
		d3d_desc.DepthWriteMask = D3D10Mapping::Mapping(desc.depth_write_mask);
		d3d_desc.DepthFunc = D3D10Mapping::Mapping(desc.depth_func);
		d3d_desc.StencilEnable = desc.front_stencil_enable;
		d3d_desc.StencilReadMask = static_cast<uint8_t>(desc.front_stencil_read_mask);
		d3d_desc.StencilWriteMask = static_cast<uint8_t>(desc.front_stencil_write_mask);
		d3d_desc.FrontFace.StencilFailOp = D3D10Mapping::Mapping(desc.front_stencil_fail);
		d3d_desc.FrontFace.StencilDepthFailOp = D3D10Mapping::Mapping(desc.front_stencil_depth_fail);
		d3d_desc.FrontFace.StencilPassOp = D3D10Mapping::Mapping(desc.front_stencil_pass);
		d3d_desc.FrontFace.StencilFunc = D3D10Mapping::Mapping(desc.front_stencil_func);
		d3d_desc.BackFace.StencilFailOp = D3D10Mapping::Mapping(desc.back_stencil_fail);
		d3d_desc.BackFace.StencilDepthFailOp = D3D10Mapping::Mapping(desc.back_stencil_depth_fail);
		d3d_desc.BackFace.StencilPassOp = D3D10Mapping::Mapping(desc.back_stencil_pass);
		d3d_desc.BackFace.StencilFunc = D3D10Mapping::Mapping(desc.back_stencil_func);

		ID3D10DevicePtr const & d3d_device = checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		ID3D10DepthStencilState* ds_state;
		d3d_device->CreateDepthStencilState(&d3d_desc, &ds_state);
		depth_stencil_state_ = MakeCOMPtr(ds_state);
	}

	void D3D10DepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t /*back_stencil_ref*/)
	{
		D3D10RenderEngine& re = *checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetDepthStencilState(depth_stencil_state_, front_stencil_ref);
	}

	D3D10BlendStateObject::D3D10BlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
		D3D10_BLEND_DESC d3d_desc;
		d3d_desc.AlphaToCoverageEnable = desc.alpha_to_coverage_enable;
		for (int i = 0; i < 8; ++ i)
		{
			d3d_desc.BlendEnable[i] = desc.blend_enable[i];
			d3d_desc.RenderTargetWriteMask[i] = static_cast<UINT8>(D3D10Mapping::MappingColorMask(desc.color_write_mask[i]));
		}
		d3d_desc.SrcBlend = D3D10Mapping::Mapping(desc.src_blend[0]);
		d3d_desc.DestBlend = D3D10Mapping::Mapping(desc.dest_blend[0]);
		d3d_desc.BlendOp = D3D10Mapping::Mapping(desc.blend_op[0]);
		d3d_desc.SrcBlendAlpha = D3D10Mapping::Mapping(desc.src_blend[0]);
		d3d_desc.DestBlendAlpha = D3D10Mapping::Mapping(desc.dest_blend[0]);
		d3d_desc.BlendOpAlpha = D3D10Mapping::Mapping(desc.blend_op[0]);

		ID3D10DevicePtr const & d3d_device = checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		ID3D10BlendState* blend_state;
		d3d_device->CreateBlendState(&d3d_desc, &blend_state);
		blend_state_ = MakeCOMPtr(blend_state);
	}

	void D3D10BlendStateObject::Active(Color const & blend_factor, uint32_t sample_mask)
	{
		D3D10RenderEngine& re = *checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetBlendState(blend_state_, blend_factor, sample_mask);
	}

	D3D10SamplerStateObject::D3D10SamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc)
	{
		D3D10_SAMPLER_DESC d3d_desc;
		d3d_desc.Filter = D3D10Mapping::Mapping(desc.filter);
		d3d_desc.AddressU = D3D10Mapping::Mapping(desc.addr_mode_u);
		d3d_desc.AddressV = D3D10Mapping::Mapping(desc.addr_mode_v);
		d3d_desc.AddressW = D3D10Mapping::Mapping(desc.addr_mode_w);
		d3d_desc.MipLODBias = desc.mip_map_lod_bias;
		d3d_desc.MaxAnisotropy = desc.anisotropy;
		d3d_desc.ComparisonFunc = D3D10_COMPARISON_NEVER;
		d3d_desc.BorderColor[0] = desc.border_clr.r();
		d3d_desc.BorderColor[1] = desc.border_clr.g();
		d3d_desc.BorderColor[2] = desc.border_clr.b();
		d3d_desc.BorderColor[3] = desc.border_clr.a();
		d3d_desc.MinLOD = -std::numeric_limits<float>::max();
		d3d_desc.MaxLOD = desc.max_mip_level;

		ID3D10DevicePtr const & d3d_device = checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		ID3D10SamplerState* sampler_state;
		d3d_device->CreateSamplerState(&d3d_desc, &sampler_state);
		sampler_state_ = MakeCOMPtr(sampler_state);
	}

	void D3D10SamplerStateObject::Active(uint32_t /*stage*/, TexturePtr /*texture*/)
	{
	}
}
