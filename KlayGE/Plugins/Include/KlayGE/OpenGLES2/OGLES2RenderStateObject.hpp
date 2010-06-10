// OGLES2RenderStateObject.hpp
// KlayGE OpenGL ES 2渲染状态对象类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2RENDERSTATEOBJECT_HPP
#define _OGLES2RENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class OGLES2RasterizerStateObject : public RasterizerStateObject
	{
	public:
		explicit OGLES2RasterizerStateObject(RasterizerStateDesc const & desc);

		void Active();
		void ForceDefaultState();

	private:
		GLenum ogl_front_face_;
	};

	class OGLES2DepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit OGLES2DepthStencilStateObject(DepthStencilStateDesc const & desc);

		void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref);
		void ForceDefaultState();

	private:
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
	};

	class OGLES2BlendStateObject : public BlendStateObject
	{
	public:
		explicit OGLES2BlendStateObject(BlendStateDesc const & desc);

		void Active(Color const & blend_factor, uint32_t sample_mask);
		void ForceDefaultState();

	private:
		GLenum ogl_blend_op_;
		GLenum ogl_blend_op_alpha_;
		GLenum ogl_src_blend_;
		GLenum ogl_dest_blend_;
		GLenum ogl_src_blend_alpha_;
		GLenum ogl_dest_blend_alpha_;
	};

	class OGLES2SamplerStateObject : public SamplerStateObject
	{
	public:
		explicit OGLES2SamplerStateObject(SamplerStateDesc const & desc);

		void Active(uint32_t stage, TexturePtr const & texture);

	private:
		GLenum ogl_addr_mode_u_;
		GLenum ogl_addr_mode_v_;
		GLenum ogl_addr_mode_w_;
		GLenum ogl_min_filter_;
		GLenum ogl_mag_filter_;
	};
}

#endif			// _OGLES2RENDERSTATEOBJECT_HPP
