// RenderStateObject.cpp
// KlayGE 渲染状态对象类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 把RenderStateObject拆成三部分 (2008.6.29)
//
// 3.5.0
// 初次建立 (2006.11.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
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

	void D3D9RasterizerStateObject::SetStates(RasterizerStateObject const & current)
	{
		RenderEngine const & re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr d3d_device = checked_cast<D3D9RenderEngine const *>(&re)->D3DDevice();

		RasterizerStateDesc const & cur_desc = current.GetDesc();
		
		if (cur_desc.polygon_mode != desc_.polygon_mode)
		{
			d3d_device->SetRenderState(D3DRS_FILLMODE, D3D9Mapping::Mapping(desc_.polygon_mode));
		}
		if (cur_desc.shade_mode != desc_.shade_mode)
		{
			d3d_device->SetRenderState(D3DRS_SHADEMODE, D3D9Mapping::Mapping(desc_.shade_mode));
		}
		if (cur_desc.cull_mode != desc_.cull_mode)
		{
			d3d_device->SetRenderState(D3DRS_CULLMODE, D3D9Mapping::Mapping(desc_.cull_mode));
		}
		if (cur_desc.polygon_offset_factor != desc_.polygon_offset_factor)
		{
			d3d_device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, float_to_uint32(desc_.polygon_offset_factor));
		}
		if (cur_desc.polygon_offset_units != desc_.polygon_offset_units)
		{
			d3d_device->SetRenderState(D3DRS_DEPTHBIAS, float_to_uint32(desc_.polygon_offset_units));
		}
		if (cur_desc.scissor_enable != desc_.scissor_enable)
		{
			d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, desc_.scissor_enable);
		}
	}

	D3D9DepthStencilStateObject::D3D9DepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
	}

	void D3D9DepthStencilStateObject::SetStates(DepthStencilStateObject const & current)
	{
		RenderEngine const & re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr d3d_device = checked_cast<D3D9RenderEngine const *>(&re)->D3DDevice();

		DepthStencilStateDesc const & cur_desc = current.GetDesc();

		if (cur_desc.depth_enable != desc_.depth_enable)
		{
			d3d_device->SetRenderState(D3DRS_ZENABLE, desc_.depth_enable ? D3DZB_TRUE : D3DZB_FALSE);
		}
		if (cur_desc.depth_write_mask != desc_.depth_write_mask)
		{
			d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, desc_.depth_write_mask ? D3DZB_TRUE : D3DZB_FALSE);
		}
		if (cur_desc.depth_func != desc_.depth_func)
		{
			d3d_device->SetRenderState(D3DRS_ZFUNC, D3D9Mapping::Mapping(desc_.depth_func));
		}

		if ((cur_desc.front_stencil_enable != desc_.front_stencil_enable)
			|| (cur_desc.back_stencil_enable != desc_.back_stencil_enable))
		{
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
		}
		if (cur_desc.front_stencil_func != desc_.front_stencil_func)
		{
			d3d_device->SetRenderState(D3DRS_STENCILFUNC, D3D9Mapping::Mapping(desc_.front_stencil_func));
		}
		if (cur_desc.front_stencil_ref != desc_.front_stencil_ref)
		{
			d3d_device->SetRenderState(D3DRS_STENCILREF, desc_.front_stencil_ref);
		}
		if (cur_desc.front_stencil_read_mask != desc_.front_stencil_read_mask)
		{
			d3d_device->SetRenderState(D3DRS_STENCILMASK, desc_.front_stencil_read_mask);
		}
		if (cur_desc.front_stencil_fail != desc_.front_stencil_fail)
		{
			d3d_device->SetRenderState(D3DRS_STENCILFAIL, D3D9Mapping::Mapping(desc_.front_stencil_fail));
		}
		if (cur_desc.front_stencil_depth_fail != desc_.front_stencil_depth_fail)
		{
			d3d_device->SetRenderState(D3DRS_STENCILZFAIL, D3D9Mapping::Mapping(desc_.front_stencil_depth_fail));
		}
		if (cur_desc.front_stencil_pass != desc_.front_stencil_pass)
		{
			d3d_device->SetRenderState(D3DRS_STENCILPASS, D3D9Mapping::Mapping(desc_.front_stencil_pass));
		}
		if (cur_desc.front_stencil_write_mask != desc_.front_stencil_write_mask)
		{
			d3d_device->SetRenderState(D3DRS_STENCILWRITEMASK, desc_.front_stencil_write_mask);
		}

		if (cur_desc.back_stencil_func != desc_.back_stencil_func)
		{
			d3d_device->SetRenderState(D3DRS_CCW_STENCILFUNC, D3D9Mapping::Mapping(desc_.back_stencil_func));
		}
		if (cur_desc.back_stencil_ref != desc_.back_stencil_ref)
		{
			d3d_device->SetRenderState(D3DRS_STENCILREF, desc_.back_stencil_ref);
		}
		if (cur_desc.back_stencil_read_mask != desc_.back_stencil_read_mask)
		{
			d3d_device->SetRenderState(D3DRS_STENCILMASK, desc_.back_stencil_read_mask);
		}
		if (cur_desc.back_stencil_fail != desc_.back_stencil_fail)
		{
			d3d_device->SetRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mapping::Mapping(desc_.back_stencil_fail));
		}
		if (cur_desc.back_stencil_depth_fail != desc_.back_stencil_depth_fail)
		{
			d3d_device->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mapping::Mapping(desc_.back_stencil_depth_fail));
		}
		if (cur_desc.back_stencil_pass != desc_.back_stencil_pass)
		{
			d3d_device->SetRenderState(D3DRS_CCW_STENCILPASS, D3D9Mapping::Mapping(desc_.back_stencil_pass));
		}
		if (cur_desc.back_stencil_write_mask != desc_.back_stencil_write_mask)
		{
			d3d_device->SetRenderState(D3DRS_STENCILWRITEMASK, desc_.back_stencil_write_mask);
		}
	}

	D3D9BlendStateObject::D3D9BlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
	}

	void D3D9BlendStateObject::SetStates(BlendStateObject const & current)
	{
		D3D9RenderEngine const & re = *checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D9Ptr d3d = re.D3DObject();
		ID3D9DevicePtr d3d_device = re.D3DDevice();

		BlendStateDesc const & cur_desc = current.GetDesc();

		if (cur_desc.alpha_to_coverage_enable != desc_.alpha_to_coverage_enable)
		{
			// NVIDIA's Transparency Multisampling
			if (S_OK == d3d->CheckDeviceFormat(re.ActiveAdapterNo(),
				D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
				static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value)))
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
		}
		if (cur_desc.blend_enable[0] != desc_.blend_enable[0])
		{
			d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, desc_.blend_enable[0]);
			d3d_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, desc_.blend_enable[0]);
		}
		if (cur_desc.blend_op[0] != desc_.blend_op[0])
		{
			d3d_device->SetRenderState(D3DRS_BLENDOP, D3D9Mapping::Mapping(desc_.blend_op[0]));
		}
		if (cur_desc.src_blend[0] != desc_.src_blend[0])
		{
			d3d_device->SetRenderState(D3DRS_SRCBLEND, D3D9Mapping::Mapping(desc_.src_blend[0]));
		}
		if (cur_desc.dest_blend[0] != desc_.dest_blend[0])
		{
			d3d_device->SetRenderState(D3DRS_DESTBLEND, D3D9Mapping::Mapping(desc_.dest_blend[0]));
		}
		if (cur_desc.blend_op_alpha[0] != desc_.blend_op_alpha[0])
		{
			d3d_device->SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mapping::Mapping(desc_.blend_op_alpha[0]));
		}
		if (cur_desc.src_blend_alpha[0] != desc_.src_blend_alpha[0])
		{
			d3d_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3D9Mapping::Mapping(desc_.src_blend_alpha[0]));
		}
		if (cur_desc.dest_blend_alpha[0] != desc_.dest_blend_alpha[0])
		{
			d3d_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3D9Mapping::Mapping(desc_.dest_blend_alpha[0]));
		}
		if (cur_desc.color_write_mask[0] != desc_.color_write_mask[0])
		{
			d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, D3D9Mapping::MappingColorMask(desc_.color_write_mask[0]));
		}
		if (cur_desc.color_write_mask[1] != desc_.color_write_mask[1])
		{
			d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE1, D3D9Mapping::MappingColorMask(desc_.color_write_mask[1]));
		}
		if (cur_desc.color_write_mask[2] != desc_.color_write_mask[2])
		{
			d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE2, D3D9Mapping::MappingColorMask(desc_.color_write_mask[2]));
		}
		if (cur_desc.color_write_mask[3] != desc_.color_write_mask[3])
		{
			d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE3, D3D9Mapping::MappingColorMask(desc_.color_write_mask[3]));
		}
	}
}
