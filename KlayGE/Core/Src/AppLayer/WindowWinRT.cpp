/**
 * @file WindowWinRT.cpp
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

#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>

#include <windows.graphics.display.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <KlayGE/Window.hpp>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & /*settings*/)
		: active_(false), ready_(false), closed_(false),
			dpi_scale_(1)
	{
		Convert(wname_, name);
	}

	Window::Window(std::string const & name, RenderSettings const & /*settings*/, void* /*native_wnd*/)
		: active_(false), ready_(false), closed_(false),
			dpi_scale_(1)
	{
		Convert(wname_, name);
	}

	Window::~Window()
	{
	}

	void Window::SetWindow(std::shared_ptr<ABI::Windows::UI::Core::ICoreWindow> const & window)
	{
		wnd_ = window;

		left_ = 0;
		top_ = 0;
		
		this->DetectsDPI();

		ABI::Windows::Foundation::Rect rc;
		wnd_->get_Bounds(&rc);
		width_ = static_cast<uint32_t>(rc.Width * dpi_scale_);
		height_ = static_cast<uint32_t>(rc.Height * dpi_scale_);
	}

	void Window::DetectsDPI()
	{
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		ComPtr<IDisplayInformationStatics> disp_info_stat;
		TIF(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat));

		ComPtr<IDisplayInformation> disp_info;
		TIF(disp_info_stat->GetForCurrentView(&disp_info));

		float dpi;
		TIF(disp_info->get_LogicalDpi(&dpi));
#else
		ComPtr<IDisplayPropertiesStatics> disp_prop;
		TIF(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayProperties).Get(),
			&disp_prop));

		float dpi;
		TIF(disp_prop->get_LogicalDpi(&dpi));
#endif

		dpi_scale_ = dpi / 96;
	}
}

#endif
