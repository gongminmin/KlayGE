// Viewport.cpp
// KlayGE Viewport class implement file
// Ver 4.0.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 4.0.0
// First release (2011.7.24)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/Viewport.hpp>

namespace KlayGE
{
	Viewport::Viewport()
		: camera(MakeSharedPtr<Camera>())
	{
	}

	Viewport::Viewport(int _left, int _top, int _width, int _height)
		: left(_left), top(_top),
			width(_width), height(_height),
			camera(MakeSharedPtr<Camera>())
	{
	}
}
