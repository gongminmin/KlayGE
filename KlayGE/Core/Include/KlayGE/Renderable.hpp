#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SharePtr.hpp>

namespace KlayGE
{
	// Abstract class defining the interface all renderable objects must implement.
	class Renderable
	{
	public:
		virtual ~Renderable()
			{ }

		virtual size_t NumSubs() const = 0;
		virtual RenderEffectPtr GetRenderEffect(size_t index) = 0;
		virtual VertexBufferPtr GetVertexBuffer(size_t index) = 0;
		virtual const WString& Name() const = 0;
	};
}

#endif //__Renderable_H__
