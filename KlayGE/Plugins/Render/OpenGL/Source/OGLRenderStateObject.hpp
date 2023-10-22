// OGLRenderStateObject.hpp
// KlayGE OpenGL渲染状态对象类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.7.0
// 初次建立 (2008.7.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERSTATEOBJECT_HPP
#define _OGLRENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class OGLRenderStateObject final : public RenderStateObject
	{
	public:
		OGLRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc);

		void Active();
		void ForceDefaultState();

	private:
		GLenum ogl_polygon_mode_;
		GLenum ogl_shade_mode_;
		GLenum ogl_front_face_;

		GLboolean ogl_depth_write_mask_;
		GLenum ogl_depth_func_;
		GLenum ogl_front_stencil_func_;
		GLenum ogl_front_stencil_fail_;
		GLenum ogl_front_stencil_depth_fail_;
		GLenum ogl_front_stencil_pass_;
		GLenum ogl_back_stencil_func_;
		GLenum ogl_back_stencil_fail_;
		GLenum ogl_back_stencil_depth_fail_;
		GLenum ogl_back_stencil_pass_;

		GLenum ogl_blend_op_;
		GLenum ogl_blend_op_alpha_;
		GLenum ogl_src_blend_;
		GLenum ogl_dest_blend_;
		GLenum ogl_src_blend_alpha_;
		GLenum ogl_dest_blend_alpha_;
		GLenum ogl_logic_op_;
	};

	class OGLSamplerStateObject final : public SamplerStateObject
	{
	public:
		explicit OGLSamplerStateObject(SamplerStateDesc const & desc);
		~OGLSamplerStateObject() override;

		GLuint GLSampler() const noexcept
		{
			return sampler_;
		}

	private:
		GLuint sampler_;
	};
}

#endif			// _OGLRENDERSTATEOBJECT_HPP
