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
	};

	class OGLDepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit OGLDepthStencilStateObject(DepthStencilStateDesc const & desc);

		void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref);
	};

	class OGLBlendStateObject : public BlendStateObject
	{
	public:
		explicit OGLBlendStateObject(BlendStateDesc const & desc);

		void Active();
	};
}

#endif			// _OGLRENDERSTATEOBJECT_HPP
