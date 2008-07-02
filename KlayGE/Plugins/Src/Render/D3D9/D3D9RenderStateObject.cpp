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
#include <KlayGE/D3D9/D3D9RenderStateObject.hpp>

namespace KlayGE
{
	D3D9RasterizerStateObject::D3D9RasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc)
	{
	}

	void D3D9RasterizerStateObject::Active()
	{
		RenderEngine const & re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&re)->D3DDevice();

		d3d_device->SetRenderState(D3DRS_FILLMODE, D3D9Mapping::Mapping(desc_.polygon_mode));
		d3d_device->SetRenderState(D3DRS_SHADEMODE, D3D9Mapping::Mapping(desc_.shade_mode));
		d3d_device->SetRenderState(D3DRS_CULLMODE, D3D9Mapping::Mapping(desc_.cull_mode, desc_.front_face_ccw));
		d3d_device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, float_to_uint32(desc_.polygon_offset_factor));
		d3d_device->SetRenderState(D3DRS_DEPTHBIAS, float_to_uint32(desc_.polygon_offset_units));
		d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, desc_.scissor_enable);
		d3d_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, desc_.multisample_enable);
	}

	D3D9DepthStencilStateObject::D3D9DepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
	}

	void D3D9DepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t /*back_stencil_ref*/)
	{
		RenderEngine const & re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&re)->D3DDevice();

		d3d_device->SetRenderState(D3DRS_ZENABLE, desc_.depth_enable ? D3DZB_TRUE : D3DZB_FALSE);
		d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, desc_.depth_write_mask ? D3DZB_TRUE : D3DZB_FALSE);
		d3d_device->SetRenderState(D3DRS_ZFUNC, D3D9Mapping::Mapping(desc_.depth_func));

		if (desc_.front_stencil_enable && desc_.back_stencil_enable)
		{
			d3d_device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
		}
		else
		{
			if (desc_.front_stencil_enable)
			{
				d3d_device->SetRenderState(D3DRS_STENCILENABLE, true);
			}
			else
			{
				if (desc_.back_stencil_enable)
				{
					d3d_device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
					d3d_device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
				}
				else
				{
					d3d_device->SetRenderState(D3DRS_STENCILENABLE, false);
					d3d_device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, false);
				}
			}
		}
		
		d3d_device->SetRenderState(D3DRS_STENCILFUNC, D3D9Mapping::Mapping(desc_.front_stencil_func));
		d3d_device->SetRenderState(D3DRS_STENCILREF, front_stencil_ref);
		d3d_device->SetRenderState(D3DRS_STENCILMASK, desc_.front_stencil_read_mask);
		d3d_device->SetRenderState(D3DRS_STENCILFAIL, D3D9Mapping::Mapping(desc_.front_stencil_fail));
		d3d_device->SetRenderState(D3DRS_STENCILZFAIL, D3D9Mapping::Mapping(desc_.front_stencil_depth_fail));
		d3d_device->SetRenderState(D3DRS_STENCILPASS, D3D9Mapping::Mapping(desc_.front_stencil_pass));
		d3d_device->SetRenderState(D3DRS_STENCILWRITEMASK, desc_.front_stencil_write_mask);

		d3d_device->SetRenderState(D3DRS_CCW_STENCILFUNC, D3D9Mapping::Mapping(desc_.back_stencil_func));
		d3d_device->SetRenderState(D3DRS_STENCILMASK, desc_.back_stencil_read_mask);
		d3d_device->SetRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mapping::Mapping(desc_.back_stencil_fail));
		d3d_device->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mapping::Mapping(desc_.back_stencil_depth_fail));
		d3d_device->SetRenderState(D3DRS_CCW_STENCILPASS, D3D9Mapping::Mapping(desc_.back_stencil_pass));
		d3d_device->SetRenderState(D3DRS_STENCILWRITEMASK, desc_.back_stencil_write_mask);
	}

	D3D9BlendStateObject::D3D9BlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
	}

	void D3D9BlendStateObject::Active()
	{
		D3D9RenderEngine const & re = *checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D9DevicePtr const & d3d_device = re.D3DDevice();

		if (re.DeviceCaps().alpha_to_coverage_support)
		{
			if (desc_.alpha_to_coverage_enable)
			{
				d3d_device->SetRenderState(D3DRS_ADAPTIVETESS_Y,
					static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value));
			}
			else
			{
				d3d_device->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
			}
		}
		d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, desc_.blend_enable[0]);
		d3d_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, desc_.blend_enable[0]);
		d3d_device->SetRenderState(D3DRS_BLENDOP, D3D9Mapping::Mapping(desc_.blend_op[0]));
		d3d_device->SetRenderState(D3DRS_SRCBLEND, D3D9Mapping::Mapping(desc_.src_blend[0]));
		d3d_device->SetRenderState(D3DRS_DESTBLEND, D3D9Mapping::Mapping(desc_.dest_blend[0]));
		d3d_device->SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mapping::Mapping(desc_.blend_op_alpha[0]));
		d3d_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3D9Mapping::Mapping(desc_.src_blend_alpha[0]));
		d3d_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3D9Mapping::Mapping(desc_.dest_blend_alpha[0]));
		d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, D3D9Mapping::MappingColorMask(desc_.color_write_mask[0]));
		d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE1, D3D9Mapping::MappingColorMask(desc_.color_write_mask[1]));
		d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE2, D3D9Mapping::MappingColorMask(desc_.color_write_mask[2]));
		d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE3, D3D9Mapping::MappingColorMask(desc_.color_write_mask[3]));
	}
}
