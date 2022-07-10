/**
 * @file SmartPtrHelper.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#ifndef KFL_SMART_PTR_HELPER_HPP
#define KFL_SMART_PTR_HELPER_HPP

#pragma once

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <memory>

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
	class Win32HandleDeleter final
	{
	public:
		void operator()(HANDLE handle)
		{
			if (handle != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(handle);
			}
		}
	};
	typedef std::unique_ptr<void, Win32HandleDeleter> Win32UniqueHandle;
	typedef std::shared_ptr<void> Win32SharedHandle;

	inline Win32UniqueHandle MakeWin32UniqueHandle(HANDLE handle)
	{
		return Win32UniqueHandle(handle);
	}
	inline Win32SharedHandle MakeWin32SharedHandle(HANDLE handle)
	{
		return Win32SharedHandle(handle, Win32HandleDeleter());
	}


	class Win32TpWaitDeleter final
	{
	public:
		void operator()(PTP_WAIT pw)
		{
			if (pw != INVALID_HANDLE_VALUE)
			{
				::CloseThreadpoolWait(pw);
			}
		}
	};
	typedef std::unique_ptr<TP_WAIT, Win32TpWaitDeleter> Win32UniqueTpWait;
	typedef std::shared_ptr<TP_WAIT> Win32SharedTpWait;

	inline Win32UniqueTpWait MakeWin32UniquTpWait(PTP_WAIT pw)
	{
		return Win32UniqueTpWait(pw);
	}
	inline Win32SharedTpWait MakeWin32SharedHandle(PTP_WAIT pw)
	{
		return Win32SharedTpWait(pw, Win32TpWaitDeleter());
	}
#endif
} // namespace KlayGE

#endif // KFL_SMART_PTR_HELPER_HPP
