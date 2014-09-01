#include <KlayGE/KlayGE.hpp>

#include <iostream>

#ifdef KLAYGE_PLATFORM_DARWIN

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>

// TODO

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false)
	{
		std::cout<<"test1";
		//NSLog(@"test1");
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false)
	{
		std::cout<<"test2";
		//NSLog(@"test2");
	}

	Window::~Window()
	{
		std::cout<<"test3";
		//NSLog(@"test3");
	}
}

#endif
