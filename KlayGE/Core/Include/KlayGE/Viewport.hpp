#ifndef _VIEWPORT_HPP
#define _VIEWPORT_HPP

#include <KlayGE/Camera.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	struct Viewport
	{
		Viewport()
			{ }
		Viewport(int _left, int _top, int _width, int _height)
			: left(_left), top(_top),
				width(_width), height(_height)
			{ }

		int left;
		int top;
		int width;
		int height;

		Camera camera;
	};
}

#endif			// _VIEWPORT_HPP