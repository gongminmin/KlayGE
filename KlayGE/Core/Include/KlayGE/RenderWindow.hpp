#ifndef _RENDERWINDOW_HPP
#define _RENDERWINDOW_HPP

#include <KlayGE/RenderTarget.hpp>

#ifdef _DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class RenderWindow : public RenderTarget
	{
	public:
		RenderWindow();
		virtual ~RenderWindow();

		virtual std::wstring const & Description() const = 0;

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
