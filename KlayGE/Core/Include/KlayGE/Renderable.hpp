// Renderable.hpp
// KlayGE 可渲染对象类 头文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 增加了Render (2005.1.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Matrix.hpp>

namespace KlayGE
{
	// Abstract class defining the interface all renderable objects must implement.
	class Renderable : public boost::enable_shared_from_this<Renderable>
	{
	public:
		virtual ~Renderable();

		virtual RenderEffectPtr GetRenderEffect() const = 0;
		virtual RenderBufferPtr GetRenderBuffer() const = 0;
		virtual std::wstring const & Name() const = 0;

		virtual void OnRenderBegin();
		virtual void OnRenderEnd();

		virtual Matrix4 GetWorld() const;
		virtual Box GetBound() const = 0;

		virtual bool CanBeCulled() const;

		virtual void Render();
	};
}

#endif		//_RENDERABLE_HPP
