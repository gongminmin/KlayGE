#ifndef _RENDERWINDOW_HPP
#define _RENDERWINDOW_HPP

#include <KlayGE/RenderTarget.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class RenderWindow : public RenderTarget
	{
	public:
		RenderWindow();
		virtual ~RenderWindow();

		virtual const WString& Description() const = 0;

		virtual void Destroy() = 0;

		virtual void Resize(int width, int height) = 0;
		virtual void Reposition(int left, int top) = 0;
		virtual void SwapBuffers() = 0;

		virtual bool Closed() const = 0;

		virtual void Update();

		bool FullScreen() const;

	protected:
		bool isFullScreen_;
	};
}

#endif			// _RENDERWINDOW_HPP
