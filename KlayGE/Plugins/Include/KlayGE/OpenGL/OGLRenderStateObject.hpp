// OGLRenderStateObject.hpp
// KlayGE OpenGL渲染状态对象类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.7.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERSTATEOBJECT_HPP
#define _OGLRENDERSTATEOBJECT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class OGLRasterizerStateObject : public RasterizerStateObject
	{
	public:
		explicit OGLRasterizerStateObject(RasterizerStateDesc const & desc);

		void Active();
		void ForceDefaultState();

	private:
		GLenum ogl_polygon_mode_;
		GLenum ogl_shade_mode_;
		GLenum ogl_front_face_;
	};

	class OGLDepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit OGLDepthStencilStateObject(DepthStencilStateDesc const & desc);

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

	class OGLBlendStateObject : public BlendStateObject
	{
	public:
		explicit OGLBlendStateObject(BlendStateDesc const & desc);

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

	class OGLSamplerStateObject : public SamplerStateObject
	{
	public:
		explicit OGLSamplerStateObject(SamplerStateDesc const & desc);

		void Active(uint32_t stage, TexturePtr const & texture);

	private:
		GLenum ogl_addr_mode_u_;
		GLenum ogl_addr_mode_v_;
		GLenum ogl_addr_mode_w_;
		GLenum ogl_min_filter_;
		GLenum ogl_mag_filter_;
	};
}

#endif			// _OGLRENDERSTATEOBJECT_HPP
