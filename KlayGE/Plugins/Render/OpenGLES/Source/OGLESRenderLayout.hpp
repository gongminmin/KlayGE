// OGLESRenderLayout.hpp
// KlayGE OpenGL ES渲染分布类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERLAYOUT_HPP
#define _OGLESRENDERLAYOUT_HPP

#pragma once

#include <map>

#include <KlayGE/RenderLayout.hpp>

namespace KlayGE
{
	class OGLESRenderLayout final : public RenderLayout
	{
	public:
		OGLESRenderLayout();
		~OGLESRenderLayout() override;

		void Active(ShaderObjectPtr const & so) const;

	private:
		void BindVertexStreams(ShaderObjectPtr const & so) const;
		void UnbindVertexStreams(ShaderObjectPtr const & so) const;

	private:
		mutable std::map<ShaderObjectPtr, GLuint> vaos_;
	};
}

#endif			// _OGLESRENDERLAYOUT_HPP
