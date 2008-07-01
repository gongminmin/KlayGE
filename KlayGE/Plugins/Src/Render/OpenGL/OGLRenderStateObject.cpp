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

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>

namespace KlayGE
{
	OGLRasterizerStateObject::OGLRasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc)
	{
	}

	void OGLRasterizerStateObject::SetStates(RasterizerStateObject const & current)
	{
		RasterizerStateDesc const & cur_desc = current.GetDesc();

		if (cur_desc.polygon_mode != desc_.polygon_mode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, OGLMapping::Mapping(desc_.polygon_mode));
		}
		if (cur_desc.shade_mode != desc_.shade_mode)
		{
			glShadeModel(OGLMapping::Mapping(desc_.shade_mode));
		}
		if (cur_desc.cull_mode != desc_.cull_mode)
		{
			switch (desc_.cull_mode)
			{
			case CM_None:
				glDisable(GL_CULL_FACE);
				break;

			case CM_Clockwise:
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);
				break;

			case CM_AntiClockwise:
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CW);
				break;
			}
		}
		if ((cur_desc.polygon_offset_factor != desc_.polygon_offset_factor)
			|| (cur_desc.polygon_offset_units != desc_.polygon_offset_units))
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glEnable(GL_POLYGON_OFFSET_POINT);
			glEnable(GL_POLYGON_OFFSET_LINE);
			// Bias is in {0, 16}, scale the unit addition appropriately
			glPolygonOffset(desc_.polygon_offset_factor, desc_.polygon_offset_units);
		}
		if (cur_desc.scissor_enable != desc_.scissor_enable)
		{
			if (desc_.scissor_enable)
			{
				glEnable(GL_SCISSOR_TEST);
			}
			else
			{
				glDisable(GL_SCISSOR_TEST);
			}
		}
	}

	OGLDepthStencilStateObject::OGLDepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
	}

	void OGLDepthStencilStateObject::SetStates(DepthStencilStateObject const & current)
	{
		DepthStencilStateDesc const & cur_desc = current.GetDesc();

		if (cur_desc.depth_enable != desc_.depth_enable)
		{
			if (desc_.depth_enable)
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}
		}
		if (cur_desc.depth_write_mask != desc_.depth_write_mask)
		{
			glDepthMask(desc_.depth_write_mask ? GL_TRUE : GL_FALSE);
		}
		if (cur_desc.depth_func != desc_.depth_func)
		{
			glDepthFunc(OGLMapping::Mapping(desc_.depth_func));
		}

		if ((cur_desc.front_stencil_func != desc_.front_stencil_func)
			|| (cur_desc.front_stencil_ref != desc_.front_stencil_ref)
			|| (cur_desc.front_stencil_read_mask != desc_.front_stencil_read_mask))
		{
			glStencilFuncSeparate(GL_FRONT, OGLMapping::Mapping(desc_.front_stencil_func),
				desc_.front_stencil_ref, desc_.front_stencil_read_mask);
		}
		if ((cur_desc.front_stencil_fail != desc_.front_stencil_fail)
			|| (cur_desc.front_stencil_depth_fail != desc_.front_stencil_depth_fail)
			|| (cur_desc.front_stencil_pass != desc_.front_stencil_pass))
		{
			glStencilOpSeparate(GL_FRONT, OGLMapping::Mapping(desc_.front_stencil_fail),
				OGLMapping::Mapping(desc_.front_stencil_depth_fail), OGLMapping::Mapping(desc_.front_stencil_pass));
		}
		if (cur_desc.front_stencil_write_mask != desc_.front_stencil_write_mask)
		{
			glStencilMaskSeparate(GL_FRONT, desc_.front_stencil_write_mask);
		}

		if ((cur_desc.back_stencil_func != desc_.back_stencil_func)
			|| (cur_desc.back_stencil_ref != desc_.back_stencil_ref)
			|| (cur_desc.back_stencil_read_mask != desc_.back_stencil_read_mask))
		{
			glStencilFuncSeparate(GL_BACK, OGLMapping::Mapping(desc_.back_stencil_func),
				desc_.back_stencil_ref, desc_.back_stencil_read_mask);
		}
		if ((cur_desc.back_stencil_fail != desc_.back_stencil_fail)
			|| (cur_desc.back_stencil_depth_fail != desc_.back_stencil_depth_fail)
			|| (cur_desc.back_stencil_pass != desc_.back_stencil_pass))
		{
			glStencilOpSeparate(GL_BACK, OGLMapping::Mapping(desc_.back_stencil_fail),
				OGLMapping::Mapping(desc_.back_stencil_depth_fail), OGLMapping::Mapping(desc_.back_stencil_pass));
		}
		if (cur_desc.back_stencil_write_mask != desc_.back_stencil_write_mask)
		{
			glStencilMaskSeparate(GL_BACK, desc_.back_stencil_write_mask);
		}

		if ((cur_desc.front_stencil_enable != desc_.front_stencil_enable)
			|| (cur_desc.back_stencil_enable != desc_.back_stencil_enable))
		{
			if (desc_.front_stencil_enable || desc_.back_stencil_enable)
			{
				glEnable(GL_STENCIL_TEST);
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
		}
	}

	OGLBlendStateObject::OGLBlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
	}

	void OGLBlendStateObject::SetStates(BlendStateObject const & current)
	{
		BlendStateDesc const & cur_desc = current.GetDesc();

		if (cur_desc.alpha_to_coverage_enable != desc_.alpha_to_coverage_enable)
		{
			if (desc_.alpha_to_coverage_enable)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
			else
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
		}
		if (cur_desc.blend_enable[0] != desc_.blend_enable[0])
		{
			if (desc_.blend_enable[0])
			{
				glEnable(GL_BLEND);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}
		if ((cur_desc.blend_op[0] != desc_.blend_op[0])
			|| (cur_desc.blend_op_alpha[0] != desc_.blend_op_alpha[0]))
		{
			glBlendEquationSeparate(OGLMapping::Mapping(desc_.blend_op[0]), OGLMapping::Mapping(desc_.blend_op_alpha[0]));
		}
		if ((cur_desc.src_blend[0] != desc_.src_blend[0])
			|| (cur_desc.dest_blend[0] != desc_.dest_blend[0])
			|| (cur_desc.src_blend_alpha[0] != desc_.src_blend_alpha[0])
			|| (cur_desc.dest_blend_alpha[0] != desc_.dest_blend_alpha[0]))
		{
			glBlendFuncSeparate(OGLMapping::Mapping(desc_.src_blend[0]), OGLMapping::Mapping(desc_.dest_blend[0]),
				OGLMapping::Mapping(desc_.src_blend_alpha[0]), OGLMapping::Mapping(desc_.dest_blend_alpha[0]));
		}
		if (cur_desc.color_write_mask[0] != desc_.color_write_mask[0])
		{
			glColorMask((desc_.color_write_mask[0] & CMASK_Red) != 0,
					(desc_.color_write_mask[0] & CMASK_Green) != 0,
					(desc_.color_write_mask[0] & CMASK_Blue) != 0,
					(desc_.color_write_mask[0] & CMASK_Alpha) != 0);
		}
	}
}
