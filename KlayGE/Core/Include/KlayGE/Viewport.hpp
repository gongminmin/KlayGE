#ifndef _VIEWPORT_HPP
#define _VIEWPORT_HPP

#include <KlayGE/Camera.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

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