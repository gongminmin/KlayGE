// OGLRenderLayout.hpp
// KlayGE OpenGL渲染分布类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 支持GL_ARB_vertex_array_object (2009.2.15)
//
// 3.2.0
// 初次建立 (2005.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERLAYOUT_HPP
#define _OGLRENDERLAYOUT_HPP

#pragma once

#include <map>

#include <KlayGE/RenderLayout.hpp>

namespace KlayGE
{
	class OGLRenderLayout final : public RenderLayout
	{
	public:
		OGLRenderLayout();
		~OGLRenderLayout() override;

		void Active(ShaderObjectPtr const & so) const;

	private:
		void BindVertexStreams(ShaderObjectPtr const & so, GLuint vao) const;
		void UnbindVertexStreams(ShaderObjectPtr const & so, GLuint vao) const;

	private:
		mutable std::map<ShaderObjectPtr, GLuint> vaos_;
	};
}

#endif			// _OGLRENDERLAYOUT_HPP
