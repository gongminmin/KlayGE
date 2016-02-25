// D3D11RenderStateObject.cpp
// KlayGE D3D11渲染状态对象类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <limits>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>

namespace KlayGE
{
	D3D11RasterizerStateObject::D3D11RasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc)
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device* d3d_device = re.D3DDevice();

		D3D11_RASTERIZER_DESC d3d_desc;
		d3d_desc.FillMode = D3D11Mapping::Mapping(desc.polygon_mode);
		d3d_desc.CullMode = D3D11Mapping::Mapping(desc.cull_mode);
		d3d_desc.FrontCounterClockwise = desc.front_face_ccw;
		d3d_desc.DepthBias = static_cast<int>(desc.polygon_offset_units);
		d3d_desc.DepthBiasClamp = desc.polygon_offset_units;
		d3d_desc.SlopeScaledDepthBias = desc.polygon_offset_factor;
		d3d_desc.DepthClipEnable = re.DeviceFeatureLevel() >= D3D_FEATURE_LEVEL_10_0 ? desc.depth_clip_enable : true;
		d3d_desc.ScissorEnable = desc.scissor_enable;
		d3d_desc.MultisampleEnable = desc.multisample_enable;
		d3d_desc.AntialiasedLineEnable = false;

		if (re.D3D11RuntimeSubVer() >= 1)
		{
			ID3D11Device1* d3d_device_1 = re.D3DDevice1();
			D3D11_RASTERIZER_DESC1 d3d_desc1;
			d3d_desc1.FillMode = d3d_desc.FillMode;
			d3d_desc1.CullMode = d3d_desc.CullMode;
			d3d_desc1.FrontCounterClockwise = d3d_desc.FrontCounterClockwise;
			d3d_desc1.DepthBias = d3d_desc.DepthBias;
			d3d_desc1.DepthBiasClamp = d3d_desc.DepthBiasClamp;
			d3d_desc1.SlopeScaledDepthBias = d3d_desc.SlopeScaledDepthBias;
			d3d_desc1.DepthClipEnable = d3d_desc.DepthClipEnable;
			d3d_desc1.ScissorEnable = d3d_desc.ScissorEnable;
			d3d_desc1.MultisampleEnable = d3d_desc.MultisampleEnable;
			d3d_desc1.AntialiasedLineEnable = d3d_desc.AntialiasedLineEnable;
			d3d_desc1.ForcedSampleCount = 0;

			ID3D11RasterizerState1* rasterizer_state;
			TIF(d3d_device_1->CreateRasterizerState1(&d3d_desc1, &rasterizer_state));
			rasterizer_state_ = MakeCOMPtr(rasterizer_state);
		}
		else
		{
			ID3D11RasterizerState* rasterizer_state;
			TIF(d3d_device->CreateRasterizerState(&d3d_desc, &rasterizer_state));
			rasterizer_state_ = MakeCOMPtr(rasterizer_state);
		}
	}

	void D3D11RasterizerStateObject::Active()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.RSSetState(rasterizer_state_.get());
	}

	D3D11DepthStencilStateObject::D3D11DepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
		D3D11_DEPTH_STENCIL_DESC d3d_desc;
		d3d_desc.DepthEnable = desc.depth_enable;
		d3d_desc.DepthWriteMask = D3D11Mapping::Mapping(desc.depth_write_mask);
		d3d_desc.DepthFunc = D3D11Mapping::Mapping(desc.depth_func);
		d3d_desc.StencilEnable = desc.front_stencil_enable;
		d3d_desc.StencilReadMask = static_cast<uint8_t>(desc.front_stencil_read_mask);
		d3d_desc.StencilWriteMask = static_cast<uint8_t>(desc.front_stencil_write_mask);
		d3d_desc.FrontFace.StencilFailOp = D3D11Mapping::Mapping(desc.front_stencil_fail);
		d3d_desc.FrontFace.StencilDepthFailOp = D3D11Mapping::Mapping(desc.front_stencil_depth_fail);
		d3d_desc.FrontFace.StencilPassOp = D3D11Mapping::Mapping(desc.front_stencil_pass);
		d3d_desc.FrontFace.StencilFunc = D3D11Mapping::Mapping(desc.front_stencil_func);
		d3d_desc.BackFace.StencilFailOp = D3D11Mapping::Mapping(desc.back_stencil_fail);
		d3d_desc.BackFace.StencilDepthFailOp = D3D11Mapping::Mapping(desc.back_stencil_depth_fail);
		d3d_desc.BackFace.StencilPassOp = D3D11Mapping::Mapping(desc.back_stencil_pass);
		d3d_desc.BackFace.StencilFunc = D3D11Mapping::Mapping(desc.back_stencil_func);

		ID3D11Device* d3d_device = checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		ID3D11DepthStencilState* ds_state;
		TIF(d3d_device->CreateDepthStencilState(&d3d_desc, &ds_state));
		depth_stencil_state_ = MakeCOMPtr(ds_state);
	}

	void D3D11DepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t /*back_stencil_ref*/)
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetDepthStencilState(depth_stencil_state_.get(), front_stencil_ref);
	}

	D3D11BlendStateObject::D3D11BlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = re.DeviceCaps();

		D3D11_BLEND_DESC d3d_desc;
		d3d_desc.AlphaToCoverageEnable = desc.alpha_to_coverage_enable;
		d3d_desc.IndependentBlendEnable = caps.independent_blend_support ? desc.independent_blend_enable : false;
		for (int i = 0; i < 8; ++ i)
		{
			uint32_t const rt_index = caps.independent_blend_support ? i : 0;

			d3d_desc.RenderTarget[i].BlendEnable = desc.blend_enable[rt_index];
			d3d_desc.RenderTarget[i].SrcBlend = D3D11Mapping::Mapping(desc.src_blend[rt_index]);
			d3d_desc.RenderTarget[i].DestBlend = D3D11Mapping::Mapping(desc.dest_blend[rt_index]);
			d3d_desc.RenderTarget[i].BlendOp = D3D11Mapping::Mapping(desc.blend_op[rt_index]);
			d3d_desc.RenderTarget[i].SrcBlendAlpha = D3D11Mapping::Mapping(desc.src_blend_alpha[rt_index]);
			d3d_desc.RenderTarget[i].DestBlendAlpha = D3D11Mapping::Mapping(desc.dest_blend_alpha[rt_index]);
			d3d_desc.RenderTarget[i].BlendOpAlpha = D3D11Mapping::Mapping(desc.blend_op_alpha[rt_index]);
			d3d_desc.RenderTarget[i].RenderTargetWriteMask = static_cast<UINT8>(D3D11Mapping::MappingColorMask(desc.color_write_mask[rt_index]));
		}

		ID3D11Device* d3d_device = re.D3DDevice();

		if (re.D3D11RuntimeSubVer() >= 1)
		{
			ID3D11Device1* d3d_device_1 = re.D3DDevice1();
			D3D11_BLEND_DESC1 d3d_desc1;
			d3d_desc1.AlphaToCoverageEnable = d3d_desc.AlphaToCoverageEnable;
			d3d_desc1.IndependentBlendEnable = d3d_desc.IndependentBlendEnable;
			for (int i = 0; i < 8; ++ i)
			{
				uint32_t const rt_index = caps.independent_blend_support ? i : 0;

				d3d_desc1.RenderTarget[i].BlendEnable = d3d_desc.RenderTarget[i].BlendEnable;
				d3d_desc1.RenderTarget[i].LogicOpEnable = desc.logic_op_enable[i];
				d3d_desc1.RenderTarget[i].SrcBlend = d3d_desc.RenderTarget[i].SrcBlend;
				d3d_desc1.RenderTarget[i].DestBlend = d3d_desc.RenderTarget[i].DestBlend;
				d3d_desc1.RenderTarget[i].BlendOp = d3d_desc.RenderTarget[i].BlendOp;
				d3d_desc1.RenderTarget[i].SrcBlendAlpha = d3d_desc.RenderTarget[i].SrcBlendAlpha;
				d3d_desc1.RenderTarget[i].DestBlendAlpha = d3d_desc.RenderTarget[i].DestBlendAlpha;
				d3d_desc1.RenderTarget[i].BlendOpAlpha = d3d_desc.RenderTarget[i].BlendOpAlpha;
				d3d_desc1.RenderTarget[i].LogicOp
					= caps.logic_op_support ? D3D11Mapping::Mapping(desc.logic_op[rt_index]) : D3D11_LOGIC_OP_NOOP;
				d3d_desc1.RenderTarget[i].RenderTargetWriteMask = d3d_desc.RenderTarget[i].RenderTargetWriteMask;
			}

			ID3D11BlendState1* blend_state;
			TIF(d3d_device_1->CreateBlendState1(&d3d_desc1, &blend_state));
			blend_state_ = MakeCOMPtr(blend_state);
		}
		else
		{
			ID3D11BlendState* blend_state;
			TIF(d3d_device->CreateBlendState(&d3d_desc, &blend_state));
			blend_state_ = MakeCOMPtr(blend_state);
		}
	}

	void D3D11BlendStateObject::Active(Color const & blend_factor, uint32_t sample_mask)
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.OMSetBlendState(blend_state_.get(), blend_factor, sample_mask);
	}

	D3D11SamplerStateObject::D3D11SamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc)
	{
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device* d3d_device = render_eng.D3DDevice();
		D3D_FEATURE_LEVEL feature_level = render_eng.DeviceFeatureLevel();

		D3D11_SAMPLER_DESC d3d_desc;
		d3d_desc.Filter = D3D11Mapping::Mapping(desc.filter);
		TexAddressingMode addr_mode_u = desc.addr_mode_u;
		if ((feature_level <= D3D_FEATURE_LEVEL_9_2) && (TAM_Border == desc.addr_mode_u))
		{
			addr_mode_u = TAM_Clamp;
		}
		d3d_desc.AddressU = D3D11Mapping::Mapping(addr_mode_u);
		TexAddressingMode addr_mode_v = desc.addr_mode_v;
		if ((feature_level <= D3D_FEATURE_LEVEL_9_2) && (TAM_Border == desc.addr_mode_u))
		{
			addr_mode_v = TAM_Clamp;
		}
		d3d_desc.AddressV = D3D11Mapping::Mapping(addr_mode_v);
		TexAddressingMode addr_mode_w = desc.addr_mode_w;
		if ((feature_level <= D3D_FEATURE_LEVEL_9_2) && (TAM_Border == desc.addr_mode_u))
		{
			addr_mode_w = TAM_Clamp;
		}
		d3d_desc.AddressW = D3D11Mapping::Mapping(addr_mode_w);
		d3d_desc.MipLODBias = desc.mip_map_lod_bias;
		d3d_desc.MaxAnisotropy = (feature_level <= D3D_FEATURE_LEVEL_9_1) ? 1 : desc.max_anisotropy;
		d3d_desc.ComparisonFunc = D3D11Mapping::Mapping(desc.cmp_func);
		d3d_desc.BorderColor[0] = desc.border_clr.r();
		d3d_desc.BorderColor[1] = desc.border_clr.g();
		d3d_desc.BorderColor[2] = desc.border_clr.b();
		d3d_desc.BorderColor[3] = desc.border_clr.a();
		if (feature_level <= D3D_FEATURE_LEVEL_9_3)
		{
			d3d_desc.MinLOD = floor(desc.min_lod);
			d3d_desc.MaxLOD = std::numeric_limits<float>::max();
		}
		else
		{
			d3d_desc.MinLOD = desc.min_lod;
			d3d_desc.MaxLOD = desc.max_lod;
		}

		ID3D11SamplerState* sampler_state;
		TIF(d3d_device->CreateSamplerState(&d3d_desc, &sampler_state));
		sampler_state_ = MakeCOMPtr(sampler_state);
	}
}
