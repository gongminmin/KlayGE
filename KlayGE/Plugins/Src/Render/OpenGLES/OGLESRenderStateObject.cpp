// OGLES2RenderStateObject.cpp
// KlayGE OpenGL ES 2渲染状态对象类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>

namespace KlayGE
{
	OGLESRasterizerStateObject::OGLESRasterizerStateObject(RasterizerStateDesc const & desc)
		: RasterizerStateObject(desc),
			ogl_front_face_(desc_.front_face_ccw ? GL_CCW : GL_CW)
	{
	}

	void OGLESRasterizerStateObject::Active()
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		RasterizerStateDesc const & cur_desc = re.CurRSObj()->GetDesc();

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
	}

	void OGLESRasterizerStateObject::ForceDefaultState()
	{
		RasterizerStateDesc desc;

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
	}


	OGLESDepthStencilStateObject::OGLESDepthStencilStateObject(DepthStencilStateDesc const & desc)
		: DepthStencilStateObject(desc),
			ogl_depth_write_mask_(desc_.depth_write_mask ? GL_TRUE : GL_FALSE),
			ogl_depth_func_(OGLESMapping::Mapping(desc_.depth_func)),
			ogl_front_stencil_func_(OGLESMapping::Mapping(desc_.front_stencil_func)),
			ogl_front_stencil_fail_(OGLESMapping::Mapping(desc_.front_stencil_fail)),
			ogl_front_stencil_depth_fail_(OGLESMapping::Mapping(desc_.front_stencil_depth_fail)),
			ogl_front_stencil_pass_(OGLESMapping::Mapping(desc_.front_stencil_pass)),
			ogl_back_stencil_func_(OGLESMapping::Mapping(desc_.back_stencil_func)),
			ogl_back_stencil_fail_(OGLESMapping::Mapping(desc_.back_stencil_fail)),
			ogl_back_stencil_depth_fail_(OGLESMapping::Mapping(desc_.back_stencil_depth_fail)),
			ogl_back_stencil_pass_(OGLESMapping::Mapping(desc_.back_stencil_pass))
	{
	}

	void OGLESDepthStencilStateObject::Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref)
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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

	void OGLESDepthStencilStateObject::ForceDefaultState()
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
		glDepthFunc(OGLESMapping::Mapping(desc.depth_func));

		glStencilFuncSeparate(GL_FRONT, OGLESMapping::Mapping(desc.front_stencil_func),
				0, desc.front_stencil_read_mask);
		glStencilOpSeparate(GL_FRONT, OGLESMapping::Mapping(desc.front_stencil_fail),
				OGLESMapping::Mapping(desc.front_stencil_depth_fail), OGLESMapping::Mapping(desc.front_stencil_pass));
		glStencilMaskSeparate(GL_FRONT, desc.front_stencil_write_mask);

		glStencilFuncSeparate(GL_BACK, OGLESMapping::Mapping(desc.back_stencil_func),
				0, desc.back_stencil_read_mask);
		glStencilOpSeparate(GL_BACK, OGLESMapping::Mapping(desc.back_stencil_fail),
				OGLESMapping::Mapping(desc.back_stencil_depth_fail), OGLESMapping::Mapping(desc.back_stencil_pass));
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


	OGLESBlendStateObject::OGLESBlendStateObject(BlendStateDesc const & desc)
		: BlendStateObject(desc),
			ogl_blend_op_(OGLESMapping::Mapping(desc_.blend_op[0])),
			ogl_blend_op_alpha_(OGLESMapping::Mapping(desc_.blend_op_alpha[0])),
			ogl_src_blend_(OGLESMapping::Mapping(desc_.src_blend[0])),
			ogl_dest_blend_(OGLESMapping::Mapping(desc_.dest_blend[0])),
			ogl_src_blend_alpha_(OGLESMapping::Mapping(desc_.src_blend_alpha[0])),
			ogl_dest_blend_alpha_(OGLESMapping::Mapping(desc_.dest_blend_alpha[0]))
	{
	}

	void OGLESBlendStateObject::Active(Color const & blend_factor, uint32_t /*sample_mask*/)
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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
		if (cur_desc.color_write_mask[0] != desc_.color_write_mask[0])
		{
			glColorMask((desc_.color_write_mask[0] & CMASK_Red) != 0,
					(desc_.color_write_mask[0] & CMASK_Green) != 0,
					(desc_.color_write_mask[0] & CMASK_Blue) != 0,
					(desc_.color_write_mask[0] & CMASK_Alpha) != 0);
		}

		if (cur_blend_factor != blend_factor)
		{
			glBlendColor(blend_factor.r(), blend_factor.g(), blend_factor.b(), blend_factor.a());
		}
	}

	void OGLESBlendStateObject::ForceDefaultState()
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
		glBlendEquationSeparate(OGLESMapping::Mapping(desc.blend_op[0]), OGLESMapping::Mapping(desc.blend_op_alpha[0]));
		glBlendFuncSeparate(OGLESMapping::Mapping(desc.src_blend[0]), OGLESMapping::Mapping(desc.dest_blend[0]),
				OGLESMapping::Mapping(desc.src_blend_alpha[0]), OGLESMapping::Mapping(desc.dest_blend_alpha[0]));
		glColorMask((desc.color_write_mask[0] & CMASK_Red) != 0,
				(desc.color_write_mask[0] & CMASK_Green) != 0,
				(desc.color_write_mask[0] & CMASK_Blue) != 0,
				(desc.color_write_mask[0] & CMASK_Alpha) != 0);

		glBlendColor(1, 1, 1, 1);
	}


	OGLESSamplerStateObject::OGLESSamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc),
			ogl_addr_mode_u_(OGLESMapping::Mapping(desc_.addr_mode_u)),
			ogl_addr_mode_v_(OGLESMapping::Mapping(desc_.addr_mode_v)),
			ogl_addr_mode_w_(OGLESMapping::Mapping(desc_.addr_mode_w))
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

	void OGLESSamplerStateObject::Active(TexturePtr const & texture)
	{
		OGLESRenderEngine const & re = *checked_cast<OGLESRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = re.DeviceCaps();

		OGLESTexture& tex = *checked_pointer_cast<OGLESTexture>(texture);

		tex.TexParameteri(GL_TEXTURE_WRAP_S, ogl_addr_mode_u_);
		tex.TexParameteri(GL_TEXTURE_WRAP_T, ogl_addr_mode_v_);
		if (glloader_GLES_VERSION_3_0())
		{
			tex.TexParameteri(GL_TEXTURE_WRAP_R, ogl_addr_mode_w_);
		}
		else if (glloader_GLES_OES_texture_3D())
		{
			tex.TexParameteri(GL_TEXTURE_WRAP_R_OES, ogl_addr_mode_w_);
		}

		tex.TexParameteri(GL_TEXTURE_MAG_FILTER, ogl_mag_filter_);
		GLenum min_filter = ogl_min_filter_;
		if (!caps.full_npot_texture_support)
		{
			// Only POT texture with full mipmap chain supports mipmap filter for now.
			uint32_t pot = 1UL << (texture->NumMipMaps() - 1);
			if ((pot != texture->Width(0)) || (pot != texture->Height(0)))
			{
				switch (ogl_min_filter_)
				{
				case GL_NEAREST_MIPMAP_NEAREST:
				case GL_NEAREST_MIPMAP_LINEAR:
					min_filter = GL_NEAREST;
					break;

				case GL_LINEAR_MIPMAP_NEAREST:
				case GL_LINEAR_MIPMAP_LINEAR:
					min_filter = GL_LINEAR;
					break;
				}
			}
		}
		tex.TexParameteri(GL_TEXTURE_MIN_FILTER, min_filter);

		if (glloader_GLES_EXT_texture_filter_anisotropic())
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
	}
}
