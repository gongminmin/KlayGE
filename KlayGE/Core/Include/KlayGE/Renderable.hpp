#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SharedPtr.hpp>

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

		virtual SharedPtr<Box> GetBound() const
			{ return SharedPtr<Box>(); }
	};
}

#endif //__Renderable_H__
