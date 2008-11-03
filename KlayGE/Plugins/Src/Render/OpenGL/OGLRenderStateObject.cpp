// OGLRenderStateObject.cpp
// KlayGE OpenGL渲染状态对象类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2007.7.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>

namespace KlayGE
{
	OGLRasterizerStateObject::OGLRasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc),
			ogl_polygon_mode_(OGLMapping::Mapping(desc_.polygon_mode)),
			ogl_shade_mode_(OGLMapping::Mapping(desc_.shade_mode)),
			ogl_front_face_(desc_.front_face_ccw ? GL_CCW : GL_CW)
	{
	}

	void OGLRasterizerStateObject::Active()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		RasterizerStateDesc const & cur_desc = re.CurRSObj()->GetDesc();

		if (cur_desc.polygon_mode != desc_.polygon_mode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, ogl_polygon_mode_);
		}
		if (cur_desc.shade_mode != desc_.shade_mode)
		{
			glShadeModel(ogl_shade_mode_);
		}
		if (cur_desc.cull_mode != desc_.cull_mode)
		{
			switch (desc_.cull_mode)
			{
			case CM_None:
				glDisable(GL_CULL_FACE);
				break;

			case CM_Front:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;

			case CM_Back:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			}
		}

		if (cur_desc.front_face_ccw != desc_.front_face_ccw)
		{
			glFrontFace(ogl_front_face_);
		}

		if ((cur_desc.polygon_offset_factor != desc_.polygon_offset_factor)
			|| (cur_desc.polygon_offset_units != desc_.polygon_offset_units))
		{
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

		if (cur_desc.multisample_enable != desc_.multisample_enable)
		{
			if (desc_.multisample_enable)
			{
				glEnable(GL_MULTISAMPLE);
				glEnable(GL_SAMPLE_COVERAGE);
			}
			else
			{
				glDisable(GL_MULTISAMPLE);
				glDisable(GL_SAMPLE_COVERAGE);
			}
		}
	}

	void OGLRasterizerStateObject::ForceDefaultState()
	{
		RasterizerStateDesc desc;

		glPolygonMode(GL_FRONT_AND_BACK, OGLMapping::Mapping(desc.polygon_mode));
		glShadeModel(OGLMapping::Mapping(desc.shade_mode));
		switch (desc.cull_mode)
		{
		case CM_None:
			glDisable(GL_CULL_FACE);
			break;

		case CM_Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;

		case CM_Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		}

		glFrontFace(desc.front_face_ccw ? GL_CCW : GL_CW);

		glPolygonOffset(desc.polygon_offset_factor, desc.polygon_offset_units);

		if (desc.scissor_enable)
		{
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}

		if (desc.multisample_enable)
		{
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_SAMPLE_COVERAGE);
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
			glDisable(GL_SAMPLE_COVERAGE);
		}
	}


	OGLDepthStencilStateObject::OGLDepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc),
			ogl_depth_write_mask_(desc_.depth_write_mask ? GL_TRUE : GL_FALSE),
			ogl_depth_func_(OGLMapping::Mapping(desc_.depth_func)),
			ogl_front_stencil_func_(OGLMapping::Mapping(desc_.front_stencil_func)),
			ogl_front_stencil_fail_(OGLMapping::Mapping(desc_.front_stencil_fail)),
			ogl_front_stencil_depth_fail_(OGLMapping::Mapping(desc_.front_stencil_depth_fail)),
			ogl_front_stencil_pass_(OGLMapping::Mapping(desc_.front_stencil_pass)),
			ogl_back_stencil_func_(OGLMapping::Mapping(desc_.back_stencil_func)),
			ogl_back_stencil_fail_(OGLMapping::Mapping(desc_.back_stencil_fail)),
			ogl_back_stencil_depth_fail_(OGLMapping::Mapping(desc_.back_stencil_depth_fail)),
			ogl_back_stencil_pass_(OGLMapping::Mapping(desc_.back_stencil_pass))
	{
	}

	void OGLDepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		DepthStencilStateDesc const & cur_desc = re.CurDSSObj()->GetDesc();
		uint16_t const cur_front_stencil_ref = re.CurFrontStencilRef();
		uint16_t const cur_back_stencil_ref = re.CurBackStencilRef();

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
			glDepthMask(ogl_depth_write_mask_);
		}
		if (cur_desc.depth_func != desc_.depth_func)
		{
			glDepthFunc(ogl_depth_func_);
		}

		if ((cur_desc.front_stencil_func != desc_.front_stencil_func)
			|| (cur_front_stencil_ref != front_stencil_ref)
			|| (cur_desc.front_stencil_read_mask != desc_.front_stencil_read_mask))
		{
			glStencilFuncSeparate(GL_FRONT, ogl_front_stencil_func_,
					front_stencil_ref, desc_.front_stencil_read_mask);
		}
		if ((cur_desc.front_stencil_fail != desc_.front_stencil_fail)
			|| (cur_desc.front_stencil_depth_fail != desc_.front_stencil_depth_fail)
			|| (cur_desc.front_stencil_pass != desc_.front_stencil_pass))
		{
			glStencilOpSeparate(GL_FRONT, ogl_front_stencil_fail_,
					ogl_front_stencil_depth_fail_, ogl_front_stencil_pass_);
		}
		if (cur_desc.front_stencil_write_mask != desc_.front_stencil_write_mask)
		{
			glStencilMaskSeparate(GL_FRONT, desc_.front_stencil_write_mask);
		}

		if ((cur_desc.back_stencil_func != desc_.back_stencil_func)
			|| (cur_back_stencil_ref != back_stencil_ref)
			|| (cur_desc.back_stencil_read_mask != desc_.back_stencil_read_mask))
		{
			glStencilFuncSeparate(GL_BACK, ogl_back_stencil_func_,
					back_stencil_ref, desc_.back_stencil_read_mask);
		}
		if ((cur_desc.back_stencil_fail != desc_.back_stencil_fail)
			|| (cur_desc.back_stencil_depth_fail != desc_.back_stencil_depth_fail)
			|| (cur_desc.back_stencil_pass != desc_.back_stencil_pass))
		{
			glStencilOpSeparate(GL_BACK, ogl_back_stencil_fail_,
					ogl_back_stencil_depth_fail_, ogl_back_stencil_pass_);
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

	void OGLDepthStencilStateObject::ForceDefaultState()
	{
		DepthStencilStateDesc desc;

		if (desc.depth_enable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		glDepthMask(desc.depth_write_mask ? GL_TRUE : GL_FALSE);
		glDepthFunc(OGLMapping::Mapping(desc.depth_func));

		glStencilFuncSeparate(GL_FRONT, OGLMapping::Mapping(desc.front_stencil_func),
				0, desc.front_stencil_read_mask);
		glStencilOpSeparate(GL_FRONT, OGLMapping::Mapping(desc.front_stencil_fail),
				OGLMapping::Mapping(desc.front_stencil_depth_fail), OGLMapping::Mapping(desc.front_stencil_pass));
		glStencilMaskSeparate(GL_FRONT, desc.front_stencil_write_mask);

		glStencilFuncSeparate(GL_BACK, OGLMapping::Mapping(desc.back_stencil_func),
				0, desc.back_stencil_read_mask);
		glStencilOpSeparate(GL_BACK, OGLMapping::Mapping(desc.back_stencil_fail),
				OGLMapping::Mapping(desc.back_stencil_depth_fail), OGLMapping::Mapping(desc.back_stencil_pass));
		glStencilMaskSeparate(GL_BACK, desc.back_stencil_write_mask);

		if (desc.front_stencil_enable || desc.back_stencil_enable)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}


	OGLBlendStateObject::OGLBlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc),
			ogl_blend_op_(OGLMapping::Mapping(desc_.blend_op[0])),
			ogl_blend_op_alpha_(OGLMapping::Mapping(desc_.blend_op_alpha[0])),
			ogl_src_blend_(OGLMapping::Mapping(desc_.src_blend[0])),
			ogl_dest_blend_(OGLMapping::Mapping(desc_.dest_blend[0])),
			ogl_src_blend_alpha_(OGLMapping::Mapping(desc_.src_blend_alpha[0])),
			ogl_dest_blend_alpha_(OGLMapping::Mapping(desc_.dest_blend_alpha[0]))
	{
	}

	void OGLBlendStateObject::Active(Color const & blend_factor, uint32_t /*sample_mask*/)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		BlendStateDesc const & cur_desc = re.CurBSObj()->GetDesc();
		Color const & cur_blend_factor = re.CurBlendFactor();

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
		if (glloader_GL_EXT_draw_buffers2())
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (cur_desc.blend_enable[i] != desc_.blend_enable[i])
				{
					if (desc_.blend_enable[i])
					{
						glEnableIndexedEXT(GL_BLEND, i);
					}
					else
					{
						glDisableIndexedEXT(GL_BLEND, i);
					}
				}
			}
		}
		else
		{
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
		}

		if (cur_desc.blend_op[0] != desc_.blend_op[0])
		{
			glBlendEquationSeparate(ogl_blend_op_, ogl_blend_op_alpha_);
		}
		if ((cur_desc.src_blend[0] != desc_.src_blend[0])
			|| (cur_desc.dest_blend[0] != desc_.dest_blend[0])
			|| (cur_desc.src_blend_alpha[0] != desc_.src_blend_alpha[0])
			|| (cur_desc.dest_blend_alpha[0] != desc_.dest_blend_alpha[0]))
		{
			glBlendFuncSeparate(ogl_src_blend_, ogl_dest_blend_,
					ogl_src_blend_alpha_, ogl_dest_blend_alpha_);
		}
		if (glloader_GL_EXT_draw_buffers2())
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (cur_desc.color_write_mask[i] != desc_.color_write_mask[i])
				{
					glColorMaskIndexedEXT(i, (desc_.color_write_mask[i] & CMASK_Red) != 0,
							(desc_.color_write_mask[i] & CMASK_Green) != 0,
							(desc_.color_write_mask[i] & CMASK_Blue) != 0,
							(desc_.color_write_mask[i] & CMASK_Alpha) != 0);
				}
			}
		}
		else
		{
			if (cur_desc.color_write_mask[0] != desc_.color_write_mask[0])
			{
				glColorMask((desc_.color_write_mask[0] & CMASK_Red) != 0,
						(desc_.color_write_mask[0] & CMASK_Green) != 0,
						(desc_.color_write_mask[0] & CMASK_Blue) != 0,
						(desc_.color_write_mask[0] & CMASK_Alpha) != 0);
			}
		}

		if (cur_blend_factor != blend_factor)
		{
			glBlendColor(blend_factor.r(), blend_factor.g(), blend_factor.b(), blend_factor.a());
		}
	}

	void OGLBlendStateObject::ForceDefaultState()
	{
		BlendStateDesc desc;

		if (desc.alpha_to_coverage_enable)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		else
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		if (glloader_GL_EXT_draw_buffers2())
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (desc.blend_enable[i])
				{
					glEnableIndexedEXT(GL_BLEND, i);
				}
				else
				{
					glDisableIndexedEXT(GL_BLEND, i);
				}
			}
		}
		else
		{
			if (desc.blend_enable[0])
			{
				glEnable(GL_BLEND);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}
		glBlendEquationSeparate(OGLMapping::Mapping(desc.blend_op[0]), OGLMapping::Mapping(desc.blend_op_alpha[0]));
		glBlendFuncSeparate(OGLMapping::Mapping(desc.src_blend[0]), OGLMapping::Mapping(desc.dest_blend[0]),
				OGLMapping::Mapping(desc.src_blend_alpha[0]), OGLMapping::Mapping(desc.dest_blend_alpha[0]));
		if (glloader_GL_EXT_draw_buffers2())
		{
			for (int i = 0; i < 8; ++ i)
			{
				glColorMaskIndexedEXT(i, (desc_.color_write_mask[i] & CMASK_Red) != 0,
						(desc_.color_write_mask[i] & CMASK_Green) != 0,
						(desc_.color_write_mask[i] & CMASK_Blue) != 0,
						(desc_.color_write_mask[i] & CMASK_Alpha) != 0);
			}
		}
		else
		{
			glColorMask((desc.color_write_mask[0] & CMASK_Red) != 0,
					(desc.color_write_mask[0] & CMASK_Green) != 0,
					(desc.color_write_mask[0] & CMASK_Blue) != 0,
					(desc.color_write_mask[0] & CMASK_Alpha) != 0);
		}

		glBlendColor(1, 1, 1, 1);
	}


	OGLSamplerStateObject::OGLSamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc),
			ogl_addr_mode_u_(OGLMapping::Mapping(desc_.addr_mode_u)),
			ogl_addr_mode_v_(OGLMapping::Mapping(desc_.addr_mode_v)),
			ogl_addr_mode_w_(OGLMapping::Mapping(desc_.addr_mode_w))
	{
		if (desc_.filter & TFOE_Min_Linear)
		{
			ogl_min_filter_ = GL_LINEAR;
		}
		else
		{
			ogl_min_filter_ = GL_NEAREST;
		}
		if (desc_.filter & TFOE_Mag_Linear)
		{
			if (desc_.filter & TFOE_Mip_Linear)
			{
				ogl_mag_filter_ = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				ogl_mag_filter_ = GL_LINEAR_MIPMAP_NEAREST;
			}
		}
		else
		{
			if (desc_.filter & TFOE_Mip_Linear)
			{
				ogl_mag_filter_ = GL_NEAREST_MIPMAP_LINEAR;
			}
			else
			{
				ogl_mag_filter_ = GL_NEAREST_MIPMAP_NEAREST;
			}
		}
	}

	void OGLSamplerStateObject::Active(uint32_t stage, TexturePtr const & texture)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		OGLTexture& tex = *checked_pointer_cast<OGLTexture>(texture);
		GLuint const gl_tex = tex.GLTexture();
		GLenum tex_type = tex.GLType();

		if (!glloader_GL_EXT_direct_state_access())
		{
			glBindTexture(tex_type, gl_tex);
		}

		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_WRAP_S, ogl_addr_mode_u_);
		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_WRAP_T, ogl_addr_mode_v_);
		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_WRAP_R, ogl_addr_mode_w_);

		if (glloader_GL_EXT_direct_state_access())
		{
			float tmp[4];
			glGetTextureParameterfvEXT(gl_tex, tex_type, GL_TEXTURE_BORDER_COLOR, tmp);
			if ((tmp[0] != desc_.border_clr.r())
				|| (tmp[1] != desc_.border_clr.g())
				|| (tmp[2] != desc_.border_clr.b())
				|| (tmp[3] != desc_.border_clr.a()))
			{
				glTextureParameterfvEXT(gl_tex, tex_type, GL_TEXTURE_BORDER_COLOR, &desc_.border_clr.r());
			}
		}
		else
		{
			float tmp[4];
			glGetTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, tmp);
			if ((tmp[0] != desc_.border_clr.r())
				|| (tmp[1] != desc_.border_clr.g())
				|| (tmp[2] != desc_.border_clr.b())
				|| (tmp[3] != desc_.border_clr.a()))
			{
				glTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, &desc_.border_clr.r());
			}
		}

		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_MAG_FILTER, ogl_min_filter_);
		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_MIN_FILTER, ogl_mag_filter_);

		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, desc_.anisotropy);
		re.TexParameter(gl_tex, tex_type, GL_TEXTURE_MAX_LEVEL, desc_.max_mip_level);
		re.TexEnv(GL_TEXTURE0 + stage, GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, desc_.mip_map_lod_bias);
	}
}
