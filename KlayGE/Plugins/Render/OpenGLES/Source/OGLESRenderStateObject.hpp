// OGLESRenderStateObject.hpp
// KlayGE OpenGL ES渲染状态对象类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERSTATEOBJECT_HPP
#define _OGLESRENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class OGLESRenderStateObject final : public RenderStateObject
	{
	public:
		OGLESRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc);

		void Active();
		void ForceDefaultState();

	private:
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
	};

	class OGLESSamplerStateObject final : public SamplerStateObject
	{
	public:
		explicit OGLESSamplerStateObject(SamplerStateDesc const & desc);
		virtual ~OGLESSamplerStateObject();

		void Active(TexturePtr const & texture);

		GLuint GLSampler() const noexcept
		{
			return sampler_;
		}

	private:
		GLuint sampler_;
	};
}

#endif			// _OGLESRENDERSTATEOBJECT_HPP
