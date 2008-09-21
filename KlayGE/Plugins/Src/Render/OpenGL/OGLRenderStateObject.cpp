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
		: RasterizerStateObject(desc)
	{
	}

	void OGLRasterizerStateObject::Active()
	{
		glPolygonMode(GL_FRONT_AND_BACK, OGLMapping::Mapping(desc_.polygon_mode));
		glShadeModel(OGLMapping::Mapping(desc_.shade_mode));
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

		glFrontFace(desc_.front_face_ccw ? GL_CCW : GL_CW);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_POLYGON_OFFSET_POINT);
		glEnable(GL_POLYGON_OFFSET_LINE);
		// Bias is in {0, 16}, scale the unit addition appropriately
		glPolygonOffset(desc_.polygon_offset_factor, desc_.polygon_offset_units);

		if (desc_.scissor_enable)
		{
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}

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

	OGLDepthStencilStateObject::OGLDepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc)
	{
	}

	void OGLDepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref)
	{
		if (desc_.depth_enable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		glDepthMask(desc_.depth_write_mask ? GL_TRUE : GL_FALSE);
		glDepthFunc(OGLMapping::Mapping(desc_.depth_func));

		glStencilFuncSeparate(GL_FRONT, OGLMapping::Mapping(desc_.front_stencil_func),
				front_stencil_ref, desc_.front_stencil_read_mask);
		glStencilOpSeparate(GL_FRONT, OGLMapping::Mapping(desc_.front_stencil_fail),
				OGLMapping::Mapping(desc_.front_stencil_depth_fail), OGLMapping::Mapping(desc_.front_stencil_pass));
		glStencilMaskSeparate(GL_FRONT, desc_.front_stencil_write_mask);

		glStencilFuncSeparate(GL_BACK, OGLMapping::Mapping(desc_.back_stencil_func),
				back_stencil_ref, desc_.back_stencil_read_mask);
		glStencilOpSeparate(GL_BACK, OGLMapping::Mapping(desc_.back_stencil_fail),
				OGLMapping::Mapping(desc_.back_stencil_depth_fail), OGLMapping::Mapping(desc_.back_stencil_pass));
		glStencilMaskSeparate(GL_BACK, desc_.back_stencil_write_mask);

		if (desc_.front_stencil_enable || desc_.back_stencil_enable)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}

	OGLBlendStateObject::OGLBlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc)
	{
	}

	void OGLBlendStateObject::Active(Color const & blend_factor, uint32_t /*sample_mask*/)
	{
		if (desc_.alpha_to_coverage_enable)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		else
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		if (desc_.blend_enable[0])
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		glBlendEquationSeparate(OGLMapping::Mapping(desc_.blend_op[0]), OGLMapping::Mapping(desc_.blend_op_alpha[0]));
		glBlendFuncSeparate(OGLMapping::Mapping(desc_.src_blend[0]), OGLMapping::Mapping(desc_.dest_blend[0]),
				OGLMapping::Mapping(desc_.src_blend_alpha[0]), OGLMapping::Mapping(desc_.dest_blend_alpha[0]));
		glColorMask((desc_.color_write_mask[0] & CMASK_Red) != 0,
					(desc_.color_write_mask[0] & CMASK_Green) != 0,
					(desc_.color_write_mask[0] & CMASK_Blue) != 0,
					(desc_.color_write_mask[0] & CMASK_Alpha) != 0);

		glBlendColor(blend_factor.r(), blend_factor.g(), blend_factor.b(), blend_factor.a());
	}

	OGLSamplerStateObject::OGLSamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc)
	{
	}

	void OGLSamplerStateObject::Active(uint32_t stage, TexturePtr texture)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());


		glActiveTexture(GL_TEXTURE0 + stage);

		OGLTexture& gl_tex = *checked_pointer_cast<OGLTexture>(texture);
		GLenum tex_type = gl_tex.GLType();

		glBindTexture(tex_type, gl_tex.GLTexture());

		re.TexParameter(tex_type, GL_TEXTURE_WRAP_S, OGLMapping::Mapping(desc_.addr_mode_u));
		re.TexParameter(tex_type, GL_TEXTURE_WRAP_T, OGLMapping::Mapping(desc_.addr_mode_v));
		re.TexParameter(tex_type, GL_TEXTURE_WRAP_R, OGLMapping::Mapping(desc_.addr_mode_w));

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

		switch (desc_.filter)
		{
		case TFO_Point:
			re.TexParameter(tex_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			re.TexParameter(tex_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;

		case TFO_Bilinear:
			re.TexParameter(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			re.TexParameter(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;

		case TFO_Trilinear:
		case TFO_Anisotropic:
			re.TexParameter(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			re.TexParameter(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		re.TexParameter(tex_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, desc_.anisotropy);
		re.TexParameter(tex_type, GL_TEXTURE_MAX_LEVEL, desc_.max_mip_level);
		re.TexEnv(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, desc_.mip_map_lod_bias);
	}
}
