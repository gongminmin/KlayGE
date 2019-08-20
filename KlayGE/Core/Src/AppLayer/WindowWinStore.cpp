/**
 * @file WindowWinStore.cpp
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

#ifdef KLAYGE_PLATFORM_WINDOWS_STORE

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>

#include <winrt/Windows.Graphics.Display.Core.h>
#include <winrt/Windows.UI.Input.Core.h>
#include <winrt/Windows.UI.ViewManagement.Core.h>

#include <KlayGE/Window.hpp>

namespace uwp
{
	using winrt::hstring;

	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Graphics::Display;
	using namespace winrt::Windows::System::Display;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::ViewManagement;
}

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity)
	{
		KFL_UNUSED(native_wnd);

		Convert(wname_, name);

		pointer_id_map_.fill(0);

		width_ = settings.width;
		height_ = settings.height;
		full_screen_ = settings.full_screen;
	}

	Window::~Window()
	{
		if (keep_screen_on_)
		{
			if (disp_request_)
			{
				disp_request_.RequestRelease();
			}
		}
	}

	void Window::SetWindow(uwp::CoreWindow const & window)
	{
		wnd_ = window;

		left_ = 0;
		top_ = 0;
		
		this->DetectsDpi();
		this->DetectsOrientation();

		auto app_view = uwp::ApplicationView::GetForCurrentView();

		app_view.Title(uwp::hstring(wname_));

		uwp::Size size;
		size.Width = static_cast<float>(width_);
		size.Height = static_cast<float>(height_);
		app_view.PreferredLaunchViewSize(size);
		app_view.PreferredLaunchWindowingMode(uwp::ApplicationViewWindowingMode::PreferredLaunchViewSize);

		auto const rc = wnd_.Bounds();
		width_ = static_cast<uint32_t>(rc.Width * dpi_scale_ + 0.5f);
		height_ = static_cast<uint32_t>(rc.Height * dpi_scale_ + 0.5f);

		ready_ = true;
	}

	void Window::DetectsDpi()
	{
		auto disp_info = uwp::DisplayInformation::GetForCurrentView();
		float dpi = disp_info.LogicalDpi();
		this->UpdateDpiScale(dpi / 96);
	}

	void Window::DetectsOrientation()
	{
		auto disp_info = uwp::DisplayInformation::GetForCurrentView();
		auto native_orientation = disp_info.NativeOrientation();
		auto curr_orientation = disp_info.CurrentOrientation();

		win_rotation_ = WR_Unspecified;

		switch (native_orientation)
		{
		case uwp::DisplayOrientations::Landscape:
			switch (curr_orientation)
			{
			case uwp::DisplayOrientations::Landscape:
				win_rotation_ = WR_Identity;
				break;

			case uwp::DisplayOrientations::Portrait:
				win_rotation_ = WR_Rotate270;
				break;

			case uwp::DisplayOrientations::LandscapeFlipped:
				win_rotation_ = WR_Rotate180;
				break;

			case uwp::DisplayOrientations::PortraitFlipped:
				win_rotation_ = WR_Rotate90;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			break;

		case uwp::DisplayOrientations::Portrait:
			switch (curr_orientation)
			{
			case uwp::DisplayOrientations::Landscape:
				win_rotation_ = WR_Rotate90;
				break;

			case uwp::DisplayOrientations::Portrait:
				win_rotation_ = WR_Identity;
				break;

			case uwp::DisplayOrientations::LandscapeFlipped:
				win_rotation_ = WR_Rotate270;
				break;

			case uwp::DisplayOrientations::PortraitFlipped:
				win_rotation_ = WR_Rotate180;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	bool Window::FullScreen(bool fs)
	{
		auto app_view = uwp::ApplicationView::GetForCurrentView();

		bool success;
		if (fs)
		{
			success = app_view.TryEnterFullScreenMode();
			if (success)
			{
				app_view.PreferredLaunchWindowingMode(uwp::ApplicationViewWindowingMode::FullScreen);
			}
		}
		else
		{
			success = true;
			app_view.ExitFullScreenMode();
			app_view.PreferredLaunchWindowingMode(uwp::ApplicationViewWindowingMode::PreferredLaunchViewSize);
		}

		return success;
	}

	void Window::OnActivated()
	{
		if (keep_screen_on_)
		{
			if (!disp_request_)
			{
				disp_request_ = uwp::DisplayRequest();
			}
			if (disp_request_)
			{
				disp_request_.RequestActive();
			}
		}

		if (full_screen_)
		{
			this->FullScreen(true);
		}
	}

	void Window::OnSizeChanged(uwp::WindowSizeChangedEventArgs const& args)
	{
		auto size = args.Size();

		active_ = true;

		width_ = static_cast<uint32_t>(size.Width * dpi_scale_ + 0.5f);
		height_ = static_cast<uint32_t>(size.Height * dpi_scale_ + 0.5f);

		this->OnSize()(*this, true);
	}

	void Window::OnVisibilityChanged(uwp::VisibilityChangedEventArgs const& args)
	{
		active_ = args.Visible();
		this->OnActive()(*this, active_);
	}

	void Window::OnClosed()
	{
		this->OnClose()(*this);

		active_ = false;
		ready_ = false;
		closed_ = true;
	}

	void Window::OnKeyDown(uwp::KeyEventArgs const& args)
	{
		uint32_t const vk = static_cast<uint32_t>(args.VirtualKey());
		if (vk < 256)
		{
			this->OnKeyDown()(*this, vk);
		}
	}

	void Window::OnKeyUp(uwp::KeyEventArgs const& args)
	{
		uint32_t const vk = static_cast<uint32_t>(args.VirtualKey());
		if (vk < 256)
		{
			this->OnKeyUp()(*this, vk);
		}
	}

	void Window::OnPointerPressed(uwp::PointerEventArgs const& args)
	{
		auto const point = args.CurrentPoint();
		uint32_t const pid = point.PointerId();

		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++i)
		{
			if (0 == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				pointer_id_map_[i] = pid;
				break;
			}
		}

		auto const position = point.Position();
		this->OnPointerDown()(*this, int2(static_cast<int>(position.X * dpi_scale_), static_cast<int>(position.Y * dpi_scale_)), conv_id);
	}

	void Window::OnPointerReleased(uwp::PointerEventArgs const& args)
	{
		auto const point = args.CurrentPoint();
		uint32_t const pid = point.PointerId();

		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++i)
		{
			if (pid == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				pointer_id_map_[i] = 0;
				break;
			}
		}

		auto const position = point.Position();
		this->OnPointerUp()(*this, int2(static_cast<int>(position.X * dpi_scale_), static_cast<int>(position.Y * dpi_scale_)), conv_id);
	}

	void Window::OnPointerMoved(uwp::PointerEventArgs const& args)
	{
		auto const point = args.CurrentPoint();
		uint32_t const pid = point.PointerId();

		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++i)
		{
			if (pid == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				break;
			}
		}

		auto const position = point.Position();
		bool const contact = point.IsInContact();

		this->OnPointerUpdate()(*this, int2(static_cast<int>(position.X * dpi_scale_), static_cast<int>(position.Y * dpi_scale_)),
			conv_id, contact);
	}

	void Window::OnPointerWheelChanged(uwp::PointerEventArgs const& args)
	{
		auto const point = args.CurrentPoint();
		uint32_t const pid = point.PointerId();

		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++i)
		{
			if (pid == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				break;
			}
		}

		auto const position = point.Position();
		auto const properties = point.Properties();
		int32_t const wheel = properties.MouseWheelDelta();
		this->OnPointerWheel()(*this, int2(static_cast<int>(position.X * dpi_scale_), static_cast<int>(position.Y * dpi_scale_)),
			conv_id, wheel);
	}

	void Window::OnDpiChanged()
	{
		uwp::Size size;
		size.Width = static_cast<float>(width_ / dpi_scale_);
		size.Height = static_cast<float>(height_ / dpi_scale_);

		this->DetectsDpi();

		auto app_view = uwp::ApplicationView::GetForCurrentView();

		bool success;
		do
		{
			success = app_view.TryResizeView(size);
		} while (!success);
	}

	void Window::OnOrientationChanged()
	{
		this->DetectsOrientation();

		this->OnSize()(*this, true);
	}

	void Window::OnDisplayContentsInvalidated()
	{
		// TODO
	}
}

#endif
