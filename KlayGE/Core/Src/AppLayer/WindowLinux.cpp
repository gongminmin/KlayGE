/**
 * @file WindowLinux.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_LINUX

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <glloader/glloader.h>

#include <KlayGE/Window.hpp>

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity)
	{
		x_display_ = XOpenDisplay(nullptr);

		int r_size, g_size, b_size, a_size, d_size, s_size;
		switch (settings.color_fmt)
		{
		case EF_ARGB8:
		case EF_ABGR8:
			r_size = 8;
			g_size = 8;
			b_size = 8;
			a_size = 8;
			break;

		case EF_A2BGR10:
			r_size = 10;
			g_size = 10;
			b_size = 10;
			a_size = 2;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		switch (settings.depth_stencil_fmt)
		{
		case EF_D16:
			d_size = 16;
			s_size = 0;
			break;

		case EF_D24S8:
			d_size = 24;
			s_size = 8;
			break;

		case EF_D32F:
			d_size = 32;
			s_size = 0;
			break;

		default:
			d_size = 0;
			s_size = 0;
			break;
		}

		std::vector<int> visual_attr;
		//visual_attr.push_back(GLX_RENDER_TYPE);
		//visual_attr.push_back(GLX_RGBA_BIT);
		visual_attr.push_back(GLX_RGBA);
		visual_attr.push_back(GLX_RED_SIZE);
		visual_attr.push_back(r_size);
		visual_attr.push_back(GLX_GREEN_SIZE);
		visual_attr.push_back(g_size);
		visual_attr.push_back(GLX_BLUE_SIZE);
		visual_attr.push_back(b_size);
		visual_attr.push_back(GLX_ALPHA_SIZE);
		visual_attr.push_back(a_size);
		//visual_attr.push_back(GLX_DRAWABLE_TYPE);
		//visual_attr.push_back(GLX_WINDOW_BIT);
		if (d_size > 0)
		{
			visual_attr.push_back(GLX_DEPTH_SIZE);
			visual_attr.push_back(d_size);
		}
		if (s_size > 0)
		{
			visual_attr.push_back(GLX_STENCIL_SIZE);
			visual_attr.push_back(s_size);
		}
		visual_attr.push_back(GLX_DOUBLEBUFFER);
		visual_attr.push_back(True);
		if (settings.sample_count > 1)
		{
			visual_attr.push_back(GLX_SAMPLE_BUFFERS);
			visual_attr.push_back(1);
			visual_attr.push_back(GLX_BUFFER_SIZE);
			visual_attr.push_back(settings.sample_count);
		}
		visual_attr.push_back(None);				// end of list

		glXChooseVisualFUNC DynamicGlXChooseVisual = (glXChooseVisualFUNC)(glloader_get_gl_proc_address("glXChooseVisual"));
		vi_ = DynamicGlXChooseVisual(x_display_, DefaultScreen(x_display_), &visual_attr[0]);

		if (native_wnd != nullptr)
		{
			x_window_ = reinterpret_cast<::Window>(native_wnd);
		}
		else
		{
			XSetWindowAttributes attr;
			attr.colormap = XCreateColormap(x_display_, RootWindow(x_display_, vi_->screen), vi_->visual, AllocNone);
			attr.border_pixel = 0;
			attr.event_mask = ExposureMask
				| VisibilityChangeMask
				| KeyPressMask
				| KeyReleaseMask
				| ButtonPressMask
				| ButtonReleaseMask
				| PointerMotionMask
				| StructureNotifyMask
				| SubstructureNotifyMask
				| FocusChangeMask
				| ResizeRedirectMask;
			x_window_ = XCreateWindow(x_display_, RootWindow(x_display_, vi_->screen),
				settings.left, settings.top, settings.width, settings.height, 0, vi_->depth,
				InputOutput, vi_->visual, CWBorderPixel | CWColormap | CWEventMask, &attr);

		}
		XStoreName(x_display_, x_window_, name.c_str());
		XMapWindow(x_display_, x_window_);
		XFlush(x_display_);

		XWindowAttributes win_attr;
		XGetWindowAttributes(x_display_, x_window_, &win_attr);
		left_ = win_attr.x;
		top_ = win_attr.y;
		width_ = win_attr.width;
		height_ = win_attr.height;

		wm_delete_window_ = XInternAtom(x_display_, "WM_DELETE_WINDOW", false);
		XSetWMProtocols(x_display_, x_window_, &wm_delete_window_, 1);
	}

	Window::~Window()
	{
		XFree(vi_);
		XDestroyWindow(x_display_, x_window_);
		XCloseDisplay(x_display_);
	}

	void Window::MsgProc(XEvent const & event)
	{
		switch (event.type)
		{
		case FocusIn:
			active_ = true;
			ready_ = true;
			this->OnActive()(*this, true);
			break;

		case FocusOut:
			active_ = false;
			this->OnActive()(*this, false);
			break;

		case Expose:
			this->OnPaint()(*this);
			break;

		case ResizeRequest:
			{
				XResizeRequestEvent const & resize_ev = reinterpret_cast<XResizeRequestEvent const &>(event);
				if ((0 == resize_ev.width) || (0 == resize_ev.height))
				{
					active_ = false;
					this->OnSize()(*this, false);
				}
				else
				{
					active_ = true;
					ready_ = true;
					this->OnSize()(*this, true);
				}
			}
			break;

		case KeyPress:
			{
				XKeyEvent const & key_ev = reinterpret_cast<XKeyEvent const &>(event);
				this->OnKeyDown()(*this, static_cast<wchar_t>(key_ev.keycode));
			}
			break;

		case KeyRelease:
			{
				XKeyEvent const & key_ev = reinterpret_cast<XKeyEvent const &>(event);
				this->OnKeyUp()(*this, static_cast<wchar_t>(key_ev.keycode));
			}
			break;

		case ClientMessage:
			if (wm_delete_window_ == static_cast<Atom>(event.xclient.data.l[0]))
			{
				this->OnClose()(*this);
				active_ = false;
				ready_ = false;
				closed_ = true;
				XDestroyWindow(x_display_, x_window_);
				XCloseDisplay(x_display_);
				exit(0);
			}
			break;
		}
	}
}

#endif

