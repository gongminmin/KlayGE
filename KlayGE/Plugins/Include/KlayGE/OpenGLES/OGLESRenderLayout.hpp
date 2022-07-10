// OGLESRenderLayout.hpp
// KlayGE OpenGL ES��Ⱦ�ֲ��� ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
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
