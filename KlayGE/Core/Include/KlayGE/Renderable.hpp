#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Matrix.hpp>
#include <string>

namespace KlayGE
{
	// Abstract class defining the interface all renderable objects must implement.
	class Renderable
	{
	public:
		virtual ~Renderable()
			{ }

		virtual RenderEffectPtr GetRenderEffect() const = 0;
		virtual RenderBufferPtr GetRenderBuffer() const = 0;
		virtual const std::wstring& Name() const = 0;

		virtual void OnRenderBegin()
			{ }
		virtual void OnRenderEnd()
			{ }

		virtual Matrix4 GetWorld() const
			{ return Matrix4::Identity(); }
		virtual Box GetBound() const = 0;

		virtual bool CanBeCulled() const
			{ return true; }
	};
}

#endif		//_RENDERABLE_HPP
