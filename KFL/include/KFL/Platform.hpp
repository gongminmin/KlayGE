/**
 * @file Platform.hpp
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

#ifndef KFL_PLATFORM_HPP
#define KFL_PLATFORM_HPP

// Detects supported platforms
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#define KLAYGE_PLATFORM_WINDOWS

	#define KLAYGE_HAS_DECLSPEC

	#if defined(_WIN64)
		#define KLAYGE_PLATFORM_WIN64
	#else
		#define KLAYGE_PLATFORM_WIN32
	#endif

	// Shut min/max in windows.h
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef WINDOWS_LEAN_AND_MEAN
		#define WINDOWS_LEAN_AND_MEAN
	#endif

	#if defined(__MINGW32__)
		#define KLAYGE_COMPILER_NAME mgw
		#include <_mingw.h>
	#else
		#include <sdkddkver.h>
	#endif

	#ifndef _WIN32_WINNT_WIN7
		#define _WIN32_WINNT_WIN7 0x0601
	#endif
	#ifndef _WIN32_WINNT_WINBLUE
		#define _WIN32_WINNT_WINBLUE 0x0603
	#endif
	#ifndef _WIN32_WINNT_WIN10
		#define _WIN32_WINNT_WIN10 0x0A00
	#endif

	#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
		#include <winapifamily.h>
		#if defined(WINAPI_FAMILY)
			#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
				#define KLAYGE_PLATFORM_WINDOWS_DESKTOP
			#else
				#define KLAYGE_PLATFORM_WINDOWS_STORE
			#endif
		#else
			#define KLAYGE_PLATFORM_WINDOWS_DESKTOP
		#endif
	#else
		#define KLAYGE_PLATFORM_WINDOWS_DESKTOP
	#endif
#elif defined(__ANDROID__)
	#define KLAYGE_PLATFORM_ANDROID
#elif defined(__CYGWIN__)
	#define KLAYGE_PLATFORM_CYGWIN
#elif defined(linux) || defined(__linux) || defined(__linux__)
	#define KLAYGE_PLATFORM_LINUX
#elif defined(__APPLE__)
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
		#define KLAYGE_PLATFORM_IOS
	#else
		#define KLAYGE_PLATFORM_DARWIN
	#endif
#else
	#error "Unknown platform. The supported target platforms are Windows, Android, Linux, macOS, and iOS."
#endif

#endif		// KFL_PLATFORM_HPP
