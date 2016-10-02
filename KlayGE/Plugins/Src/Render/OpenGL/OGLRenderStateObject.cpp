// OGLRenderStateObject.cpp
// KlayGE OpenGL渲染状态对象类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2008-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持Depth Clamp (2009.8.5)
//
// 3.7.0
// 初次建立 (2007.7.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
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

		re.SetPolygonMode(GL_FRONT_AND_BACK, ogl_polygon_mode_);
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

		if (cur_desc.depth_clip_enable != desc_.depth_clip_enable)
		{
			if (glloader_GL_VERSION_3_2() || glloader_GL_ARB_depth_clamp())
			{
				if (desc_.depth_clip_enable)
				{
					glDisable(GL_DEPTH_CLAMP);
				}
				else
				{
					glEnable(GL_DEPTH_CLAMP);
				}
			}
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
			}
			else
			{
				glDisable(GL_MULTISAMPLE);
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

		if (glloader_GL_VERSION_3_2() || glloader_GL_ARB_depth_clamp())
		{
			if (desc.depth_clip_enable)
			{
				glDisable(GL_DEPTH_CLAMP);
			}
			else
			{
				glEnable(GL_DEPTH_CLAMP);
			}
		}

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
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
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
			ogl_dest_blend_alpha_(OGLMapping::Mapping(desc_.dest_blend_alpha[0])),
			ogl_logic_op_(OGLMapping::Mapping(desc_.logic_op[0]))
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
		if (desc_.independent_blend_enable)
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (cur_desc.blend_enable[i] != desc_.blend_enable[i])
				{
					if (desc_.blend_enable[i])
					{
						glEnablei(GL_BLEND, i);
					}
					else
					{
						glDisablei(GL_BLEND, i);
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

		if (cur_desc.logic_op_enable[0] != desc_.logic_op_enable[0])
		{
			if (desc_.logic_op_enable[0])
			{
				glEnable(GL_LOGIC_OP_MODE);
			}
			else
			{
				glDisable(GL_LOGIC_OP_MODE);
			}
		}

		if (glloader_GL_VERSION_4_0())
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (cur_desc.blend_op[i] != desc_.blend_op[i])
				{
					glBlendEquationSeparatei(i, ogl_blend_op_, ogl_blend_op_alpha_);
				}
				if ((cur_desc.src_blend[i] != desc_.src_blend[i])
					|| (cur_desc.dest_blend[i] != desc_.dest_blend[i])
					|| (cur_desc.src_blend_alpha[i] != desc_.src_blend_alpha[i])
					|| (cur_desc.dest_blend_alpha[i] != desc_.dest_blend_alpha[i]))
				{
					glBlendFuncSeparatei(i, ogl_src_blend_, ogl_dest_blend_,
						ogl_src_blend_alpha_, ogl_dest_blend_alpha_);
				}
			}
		}
		else if (glloader_GL_ARB_draw_buffers_blend())
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (cur_desc.blend_op[i] != desc_.blend_op[i])
				{
					glBlendEquationSeparateiARB(i, ogl_blend_op_, ogl_blend_op_alpha_);
				}
				if ((cur_desc.src_blend[i] != desc_.src_blend[i])
					|| (cur_desc.dest_blend[i] != desc_.dest_blend[i])
					|| (cur_desc.src_blend_alpha[i] != desc_.src_blend_alpha[i])
					|| (cur_desc.dest_blend_alpha[i] != desc_.dest_blend_alpha[i]))
				{
					glBlendFuncSeparateiARB(i, ogl_src_blend_, ogl_dest_blend_,
						ogl_src_blend_alpha_, ogl_dest_blend_alpha_);
				}
			}
		}
		else
		{
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
		}
		if (desc_.independent_blend_enable)
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (cur_desc.color_write_mask[i] != desc_.color_write_mask[i])
				{
					glColorMaski(i, (desc_.color_write_mask[i] & CMASK_Red) != 0,
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

		if (cur_desc.logic_op[0] != desc_.logic_op[0])
		{
			glLogicOp(ogl_logic_op_);
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
		if (desc.blend_enable[0])
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		if (desc.logic_op_enable[0])
		{
			glEnable(GL_COLOR_LOGIC_OP);
		}
		else
		{
			glDisable(GL_COLOR_LOGIC_OP);
		}
		glBlendEquationSeparate(OGLMapping::Mapping(desc.blend_op[0]), OGLMapping::Mapping(desc.blend_op_alpha[0]));
		glBlendFuncSeparate(OGLMapping::Mapping(desc.src_blend[0]), OGLMapping::Mapping(desc.dest_blend[0]),
				OGLMapping::Mapping(desc.src_blend_alpha[0]), OGLMapping::Mapping(desc.dest_blend_alpha[0]));
		glColorMask((desc.color_write_mask[0] & CMASK_Red) != 0,
				(desc.color_write_mask[0] & CMASK_Green) != 0,
				(desc.color_write_mask[0] & CMASK_Blue) != 0,
				(desc.color_write_mask[0] & CMASK_Alpha) != 0);
		glLogicOp(OGLMapping::Mapping(desc.logic_op[0]));

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
			if (desc_.filter & TFOE_Mip_Linear)
			{
				ogl_min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				ogl_min_filter_ = GL_LINEAR_MIPMAP_NEAREST;
			}
		}
		else
		{
			if (desc_.filter & TFOE_Mip_Linear)
			{
				ogl_min_filter_ = GL_NEAREST_MIPMAP_LINEAR;
			}
			else
			{
				ogl_min_filter_ = GL_NEAREST_MIPMAP_NEAREST;
			}
		}
		if (desc_.filter & TFOE_Mag_Linear)
		{
			ogl_mag_filter_ = GL_LINEAR;
		}
		else
		{
			ogl_mag_filter_ = GL_NEAREST;
		}
		if (desc_.filter & TFOE_Anisotropic)
		{
			ogl_mag_filter_ = GL_LINEAR;
			ogl_min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	void OGLSamplerStateObject::Active(TexturePtr const & texture)
	{
		OGLTexture& tex = *checked_cast<OGLTexture*>(texture.get());

		tex.TexParameteri(GL_TEXTURE_WRAP_S, ogl_addr_mode_u_);
		tex.TexParameteri(GL_TEXTURE_WRAP_T, ogl_addr_mode_v_);
		tex.TexParameteri(GL_TEXTURE_WRAP_R, ogl_addr_mode_w_);

		tex.TexParameterfv(GL_TEXTURE_BORDER_COLOR, &desc_.border_clr.r());

		tex.TexParameteri(GL_TEXTURE_MAG_FILTER, ogl_mag_filter_);
		tex.TexParameteri(GL_TEXTURE_MIN_FILTER, ogl_min_filter_);

		if (glloader_GL_EXT_texture_filter_anisotropic())
		{
			if (desc_.filter & TFOE_Anisotropic)
			{
				tex.TexParameteri(GL_TEXTURE_MAX_ANISOTROPY_EXT, desc_.max_anisotropy);
			}
			else
			{
				tex.TexParameteri(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			}
		}
		tex.TexParameterf(GL_TEXTURE_MIN_LOD, desc_.min_lod);
		tex.TexParameterf(GL_TEXTURE_MAX_LOD, desc_.max_lod);
		if (desc_.cmp_func != CF_AlwaysFail)
		{
			tex.TexParameteri(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		}
		else
		{
			tex.TexParameteri(GL_TEXTURE_COMPARE_MODE, GL_NONE);
		}
		tex.TexParameteri(GL_TEXTURE_COMPARE_FUNC, OGLMapping::Mapping(desc_.cmp_func));

		tex.TexParameterf(GL_TEXTURE_LOD_BIAS, desc_.mip_map_lod_bias);
	}
}
