// D3D9RenderStateObject.cpp
// KlayGE D3D9渲染状态对象类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.7.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderStateObject.hpp>

namespace KlayGE
{
	D3D9RasterizerStateObject::D3D9RasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc),
			d3d9_polygon_mode_(D3D9Mapping::Mapping(desc.polygon_mode)),
			d3d9_shade_mode_(D3D9Mapping::Mapping(desc.shade_mode)),
			d3d9_cull_mode_(D3D9Mapping::Mapping(desc.cull_mode, desc.front_face_ccw)),
			d3d9_polygon_offset_factor_(float_to_uint32(desc.polygon_offset_factor)),
			d3d9_polygon_offset_units_(float_to_uint32(desc.polygon_offset_units))
	{
	}

	void D3D9RasterizerStateObject::Active()
	{
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		re.SetRenderState(D3DRS_FILLMODE, d3d9_polygon_mode_);
		re.SetRenderState(D3DRS_SHADEMODE, d3d9_shade_mode_);
		re.SetRenderState(D3DRS_CULLMODE, d3d9_cull_mode_);
		re.SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, d3d9_polygon_offset_factor_);
		re.SetRenderState(D3DRS_DEPTHBIAS, d3d9_polygon_offset_units_);
		re.SetRenderState(D3DRS_SCISSORTESTENABLE, desc_.scissor_enable);
		re.SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, desc_.multisample_enable);
	}

	D3D9DepthStencilStateObject::D3D9DepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc),
			d3d9_depth_func_(D3D9Mapping::Mapping(desc.depth_func)),
			d3d9_front_stencil_func_(D3D9Mapping::Mapping(desc.front_stencil_func)),
			d3d9_front_stencil_fail_(D3D9Mapping::Mapping(desc_.front_stencil_fail)),
			d3d9_front_stencil_depth_fail_(D3D9Mapping::Mapping(desc.front_stencil_depth_fail)),
			d3d9_front_stencil_pass_(D3D9Mapping::Mapping(desc.front_stencil_pass)),
			d3d9_back_stencil_func_(D3D9Mapping::Mapping(desc.back_stencil_func)),
			d3d9_back_stencil_fail_(D3D9Mapping::Mapping(desc_.back_stencil_fail)),
			d3d9_back_stencil_depth_fail_(D3D9Mapping::Mapping(desc.back_stencil_depth_fail)),
			d3d9_back_stencil_pass_(D3D9Mapping::Mapping(desc.back_stencil_pass))
	{
	}

	void D3D9DepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t /*back_stencil_ref*/)
	{
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		re.SetRenderState(D3DRS_ZENABLE, desc_.depth_enable);
		re.SetRenderState(D3DRS_ZWRITEENABLE, desc_.depth_write_mask);
		re.SetRenderState(D3DRS_ZFUNC, d3d9_depth_func_);

		if (desc_.front_stencil_enable && desc_.back_stencil_enable)
		{
			re.SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
		}
		else
		{
			if (desc_.front_stencil_enable)
			{
				re.SetRenderState(D3DRS_STENCILENABLE, true);
			}
			else
			{
				if (desc_.back_stencil_enable)
				{
					re.SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
					re.SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
				}
				else
				{
					re.SetRenderState(D3DRS_STENCILENABLE, false);
					re.SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, false);
				}
			}
		}

		re.SetRenderState(D3DRS_STENCILFUNC, d3d9_front_stencil_func_);
		re.SetRenderState(D3DRS_STENCILREF, front_stencil_ref);
		re.SetRenderState(D3DRS_STENCILMASK, desc_.front_stencil_read_mask);
		re.SetRenderState(D3DRS_STENCILFAIL, d3d9_front_stencil_fail_);
		re.SetRenderState(D3DRS_STENCILZFAIL, d3d9_front_stencil_depth_fail_);
		re.SetRenderState(D3DRS_STENCILPASS, d3d9_front_stencil_pass_);
		re.SetRenderState(D3DRS_STENCILWRITEMASK, desc_.front_stencil_write_mask);

		re.SetRenderState(D3DRS_CCW_STENCILFUNC, d3d9_back_stencil_func_);
		re.SetRenderState(D3DRS_CCW_STENCILFAIL, d3d9_back_stencil_fail_);
		re.SetRenderState(D3DRS_CCW_STENCILZFAIL, d3d9_back_stencil_depth_fail_);
		re.SetRenderState(D3DRS_CCW_STENCILPASS, d3d9_back_stencil_pass_);
	}

	D3D9BlendStateObject::D3D9BlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc),
			d3d9_blend_op_(D3D9Mapping::Mapping(desc.blend_op[0])),
			d3d9_src_blend_(D3D9Mapping::Mapping(desc.src_blend[0])),
			d3d9_dest_blend_(D3D9Mapping::Mapping(desc.dest_blend[0])),
			d3d9_blend_op_alpha_(D3D9Mapping::Mapping(desc.blend_op_alpha[0])),
			d3d9_src_blend_alpha_(D3D9Mapping::Mapping(desc.src_blend_alpha[0])),
			d3d9_dest_blend_alpha_(D3D9Mapping::Mapping(desc.dest_blend_alpha[0])),
			d3d9_color_write_mask_0_(D3D9Mapping::MappingColorMask(desc.color_write_mask[0])),
			d3d9_color_write_mask_1_(D3D9Mapping::MappingColorMask(desc.color_write_mask[1])),
			d3d9_color_write_mask_2_(D3D9Mapping::MappingColorMask(desc.color_write_mask[2])),
			d3d9_color_write_mask_3_(D3D9Mapping::MappingColorMask(desc.color_write_mask[3]))
	{
	}

	void D3D9BlendStateObject::Active(Color const & blend_factor, uint32_t /*sample_mask*/)
	{
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		if (re.DeviceCaps().alpha_to_coverage_support)
		{
			if (desc_.alpha_to_coverage_enable)
			{
				re.SetRenderState(D3DRS_ADAPTIVETESS_Y,
					static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value));
			}
			else
			{
				re.SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
			}
		}
		re.SetRenderState(D3DRS_ALPHABLENDENABLE, desc_.blend_enable[0]);
		re.SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		re.SetRenderState(D3DRS_BLENDOP, d3d9_blend_op_);
		re.SetRenderState(D3DRS_SRCBLEND, d3d9_src_blend_);
		re.SetRenderState(D3DRS_DESTBLEND, d3d9_dest_blend_);
		re.SetRenderState(D3DRS_BLENDOPALPHA, d3d9_blend_op_alpha_);
		re.SetRenderState(D3DRS_SRCBLENDALPHA, d3d9_src_blend_alpha_);
		re.SetRenderState(D3DRS_DESTBLENDALPHA, d3d9_dest_blend_alpha_);
		re.SetRenderState(D3DRS_COLORWRITEENABLE, d3d9_color_write_mask_0_);
		re.SetRenderState(D3DRS_COLORWRITEENABLE1, d3d9_color_write_mask_1_);
		re.SetRenderState(D3DRS_COLORWRITEENABLE2, d3d9_color_write_mask_2_);
		re.SetRenderState(D3DRS_COLORWRITEENABLE3, d3d9_color_write_mask_3_);

		re.SetRenderState(D3DRS_BLENDFACTOR, blend_factor.ARGB());
	}

	D3D9SamplerStateObject::D3D9SamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc),
			d3d9_border_clr_(D3D9Mapping::MappingToUInt32Color(desc.border_clr)),
			d3d9_addr_mode_u_(D3D9Mapping::Mapping(desc.addr_mode_u)),
			d3d9_addr_mode_v_(D3D9Mapping::Mapping(desc.addr_mode_v)),
			d3d9_addr_mode_w_(D3D9Mapping::Mapping(desc.addr_mode_w))
	{
	}

	void D3D9SamplerStateObject::Active(uint32_t stage, TexturePtr texture)
	{
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		re.SetTexture(stage, checked_pointer_cast<D3D9Texture>(texture)->D3DBaseTexture().get());
		re.SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, IsSRGB(texture->Format()));

		re.SetSamplerState(stage, D3DSAMP_BORDERCOLOR, d3d9_border_clr_);

		re.SetSamplerState(stage, D3DSAMP_ADDRESSU, d3d9_addr_mode_u_);
		re.SetSamplerState(stage, D3DSAMP_ADDRESSV, d3d9_addr_mode_v_);
		re.SetSamplerState(stage, D3DSAMP_ADDRESSW, d3d9_addr_mode_w_);

		switch (desc_.filter)
		{
		case TFO_Point:
			re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
			break;

		case TFO_Bilinear:
			re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
			break;

		case TFO_Trilinear:
			re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			break;

		case TFO_Anisotropic:
			re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			break;

		default:
			BOOST_ASSERT(false);
			re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
			break;
		}

		re.SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, desc_.anisotropy);
		re.SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, desc_.max_mip_level);
		re.SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, float_to_uint32(desc_.mip_map_lod_bias));
	}
}
