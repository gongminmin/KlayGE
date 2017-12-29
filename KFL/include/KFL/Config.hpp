/**
 * @file Config.hpp
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

#ifndef _KFL_CONFIG_HPP
#define _KFL_CONFIG_HPP

#if !defined(__cplusplus)
	#error C++ compiler required.
#endif

#if defined(DEBUG) | defined(_DEBUG)
	#define KLAYGE_DEBUG
#endif

#define KFL_STRINGIZE(X) KFL_DO_STRINGIZE(X)
#define KFL_DO_STRINGIZE(X) #X

#define KFL_JOIN(X, Y) KFL_DO_JOIN(X, Y)
#define KFL_DO_JOIN(X, Y) KFL_DO_JOIN2(X, Y)
#define KFL_DO_JOIN2(X, Y) X##Y

// KlayGE requires vc 12.0+, g++ 5.1+, clang 3.6+, with C++14 or C++17 option on.

// Defines supported compilers
#if defined(__clang__)
	// Clang++

	#if __cplusplus < 201402L
		#error "-std=c++14 must be turned on."
	#endif

	#define KLAYGE_COMPILER_CLANG
	#define KLAYGE_COMPILER_NAME clang

	#define CLANG_VERSION KFL_JOIN(__clang_major__, __clang_minor__)

	#if __cplusplus > 201402L
		#define KLAYGE_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT
	#endif

	#if defined(__APPLE__)
		#if CLANG_VERSION >= 61
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install Apple clang++ 6.1 or up."
		#endif

		#define KLAYGE_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
		#if CLANG_VERSION >= 90
			#define KLAYGE_CXX17_LIBRARY_STRING_VIEW_SUPPORT
		#endif

		#define KLAYGE_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define KLAYGE_SYMBOL_IMPORT
	#elif defined(__ANDROID__)
		#if CLANG_VERSION >= 36
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang++ 3.6 or up."
		#endif

		#define KLAYGE_TS_LIBRARY_OPTIONAL_SUPPORT

		#define KLAYGE_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define KLAYGE_SYMBOL_IMPORT
	#elif defined(__c2__)
		#if CLANG_VERSION >= 36
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang++ 3.6 or up."
		#endif

		#define KLAYGE_COMPILER_CLANGC2

		#define KLAYGE_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
		#define KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT

		#define KLAYGE_SYMBOL_EXPORT __declspec(dllexport)
		#define KLAYGE_SYMBOL_IMPORT __declspec(dllimport)

		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _SCL_SECURE_NO_DEPRECATE
			#define _SCL_SECURE_NO_DEPRECATE
		#endif
	#else
		#error "Clang++ on an unknown platform. Only Apple and Windows are supported."
	#endif

	#define KLAYGE_ATTRIBUTE_NORETURN __attribute__((noreturn))
	#define KLAYGE_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(__GNUC__)
	// GNU C++

	#define KLAYGE_COMPILER_GCC

	#include <bits/c++config.h>
	#ifdef _GLIBCXX_USE_FLOAT128
		#undef _GLIBCXX_USE_FLOAT128
	#endif
	#ifdef _GLIBCXX_USE_INT128
		#undef _GLIBCXX_USE_INT128
	#endif

	#define GCC_VERSION KFL_JOIN(__GNUC__, __GNUC_MINOR__)

	#if GCC_VERSION >= 51
		#define KLAYGE_COMPILER_VERSION GCC_VERSION
	#else
		#error "Unsupported compiler version. Please install g++ 5.1 or up."
	#endif

	#if __cplusplus < 201402L
		#error "-std=c++14 must be turned on."
	#endif
	#if !defined(_GLIBCXX_HAS_GTHREADS)
		#error "_GLIBCXX_HAS_GTHREADS must be turned on."
	#endif

	#if KLAYGE_COMPILER_VERSION >= 71
		#define KLAYGE_CXX17_LIBRARY_ANY_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_OPTIONAL_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_STRING_VIEW_SUPPORT
	#else
		#define KLAYGE_TS_LIBRARY_ANY_SUPPORT
		#define KLAYGE_TS_LIBRARY_OPTIONAL_SUPPORT
	#endif

	#if KLAYGE_COMPILER_VERSION >= 61
		#define KLAYGE_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
		#define KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT
	#endif
	#if KLAYGE_COMPILER_VERSION >= 60
		#if __cplusplus > 201402L
			#define KLAYGE_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT
		#endif
	#endif

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
		#define KLAYGE_SYMBOL_EXPORT __attribute__((__dllexport__))
		#define KLAYGE_SYMBOL_IMPORT __attribute__((__dllimport__))
	#else
		#define KLAYGE_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define KLAYGE_SYMBOL_IMPORT
	#endif

	#define KLAYGE_ATTRIBUTE_NORETURN __attribute__((noreturn))
	#define KLAYGE_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
	#define KLAYGE_COMPILER_MSVC
	#define KLAYGE_COMPILER_NAME vc

	#define KLAYGE_HAS_DECLSPEC
	#define KLAYGE_SYMBOL_EXPORT __declspec(dllexport)
	#define KLAYGE_SYMBOL_IMPORT __declspec(dllimport)

	#if _MSC_VER >= 1910
		#define KLAYGE_COMPILER_VERSION 141
	#elif _MSC_VER >= 1900
		#define KLAYGE_COMPILER_VERSION 140
	#else
		#error "Unsupported compiler version. Please install vc14 or up."
	#endif

	#if _MSVC_LANG > 201402
		#if _MSC_VER >= 1910
			#define KLAYGE_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT
		#endif
		#define KLAYGE_CXX17_LIBRARY_ANY_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_OPTIONAL_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_STRING_VIEW_SUPPORT
	#endif

	#define KLAYGE_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
	#define KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT

	#pragma warning(disable: 4251) // STL classes are not dllexport.
	#pragma warning(disable: 4275) // Derived from non dllexport classes.
	#pragma warning(disable: 4503) // Some decorated name in boost are very long.
	#pragma warning(disable: 4819) // Allow non-ANSI characters.

	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif
	#ifndef _SCL_SECURE_NO_DEPRECATE
		#define _SCL_SECURE_NO_DEPRECATE
	#endif

	#define KLAYGE_ATTRIBUTE_NORETURN __declspec(noreturn)
	#define KLAYGE_BUILTIN_UNREACHABLE __assume(false)
#else
	#error "Unknown compiler. Please install vc, g++ or clang."
#endif

// Defines supported platforms
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

	// Forces all boost's libraries to be linked as dll
	#ifndef BOOST_ALL_DYN_LINK
		#define BOOST_ALL_DYN_LINK
	#endif
	// Skips boost's #pragma lib
	#ifndef BOOST_ALL_NO_LIB
		#define BOOST_ALL_NO_LIB
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
	#error "Unknown platform. The supported target platforms are Windows, Android, Linux, OSX, and iOS."
#endif

// Defines supported CPUs
#if defined(KLAYGE_COMPILER_MSVC)
	#if defined(_M_X64)
		#define KLAYGE_CPU_X64
		#define KLAYGE_COMPILER_TARGET x64
	#elif defined(_M_ARM64)
		#define KLAYGE_CPU_ARM64
		#define KLAYGE_COMPILER_TARGET arm64
	#elif defined(_M_ARM)
		#define KLAYGE_CPU_ARM
		#define KLAYGE_COMPILER_TARGET arm
	#else
		#error "Unknown CPU type. In msvc, x64, arm, and arm64 are supported."
	#endif
#elif defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
	#if defined(__x86_64__)
		#define KLAYGE_CPU_X64
		#define KLAYGE_COMPILER_TARGET x64
	#elif defined(__i386__)
		#define KLAYGE_CPU_X86
		#define KLAYGE_COMPILER_TARGET x86
	#elif defined(__arm__)
		#define KLAYGE_CPU_ARM
		#define KLAYGE_COMPILER_TARGET arm
	#elif defined(__aarch64__)
		#define KLAYGE_CPU_ARM64
		#define KLAYGE_COMPILER_TARGET arm64
	#else
		#error "Unknown CPU type. In g++/clang, x86, x64, arm and arm64 are supported."
	#endif
#endif

// Defines the native endian
#if defined(KLAYGE_CPU_ARM) || defined(KLAYGE_CPU_ARM64)
	#if defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__)
		#define KLAYGE_BIG_ENDIAN
	#else
		#define KLAYGE_LITTLE_ENDIAN
	#endif
#elif defined(KLAYGE_CPU_X86) || defined(KLAYGE_CPU_X64) || defined(KLAYGE_PLATFORM_WINDOWS)
	#define KLAYGE_LITTLE_ENDIAN
#else
	#error "Unknown CPU endian."
#endif

// Defines some MACROs from compile options
#ifdef KLAYGE_CPU_X64
	#define KLAYGE_SSE_SUPPORT
	#define KLAYGE_SSE2_SUPPORT
	#define KLAYGE_X64_SUPPORT
	#if defined(KLAYGE_COMPILER_MSVC)
		#ifdef __AVX__
			#define KLAYGE_AVX_SUPPORT
		#endif
		#ifdef __AVX2__
			#define KLAYGE_AVX2_SUPPORT
		#endif	
	#elif defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
		#ifdef __SSE3__
			#define KLAYGE_SSE3_SUPPORT
		#endif
		#ifdef __SSSE3__
			#define KLAYGE_SSSE3_SUPPORT
		#endif
		#ifdef __SSE4_1__
			#define KLAYGE_SSE4_1_SUPPORT
		#endif
		#ifdef __SSE4_2__
			#define KLAYGE_SSE4_2_SUPPORT
		#endif
		#ifdef __AVX__
			#define KLAYGE_AVX_SUPPORT
		#endif
		#ifdef __AVX2__
			#define KLAYGE_AVX2_SUPPORT
		#endif
	#endif
#elif defined KLAYGE_CPU_X86
	#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
		#ifdef __MMX__
			#define KLAYGE_MMX_SUPPORT
		#endif
		#ifdef __SSE__
			#define KLAYGE_SSE_SUPPORT
		#endif
		#ifdef __SSE2__
			#define KLAYGE_SSE2_SUPPORT
		#endif
		#ifdef __SSE3__
			#define KLAYGE_SSE3_SUPPORT
		#endif
		#ifdef __SSSE3__
			#define KLAYGE_SSSE3_SUPPORT
		#endif
		#ifdef __SSE4_1__
			#define KLAYGE_SSE4_1_SUPPORT
		#endif
		#ifdef __SSE4_2__
			#define KLAYGE_SSE4_2_SUPPORT
		#endif
		#ifdef __AVX__
			#define KLAYGE_AVX_SUPPORT
		#endif
		#ifdef __AVX2__
			#define KLAYGE_AVX2_SUPPORT
		#endif
	#endif
#elif defined KLAYGE_CPU_ARM
	#if defined(KLAYGE_COMPILER_MSVC)
		#define KLAYGE_NEON_SUPPORT
	#elif defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
		#ifdef __ARM_NEON__
			#define KLAYGE_NEON_SUPPORT
		#endif
	#endif
#elif defined KLAYGE_CPU_ARM64
	#define KLAYGE_NEON_SUPPORT
#endif

#if defined(KLAYGE_COMPILER_MSVC) || defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
	#define KLAYGE_HAS_STRUCT_PACK
#endif

// Deprecated features in Boost.System are excluded.
#ifndef BOOST_SYSTEM_NO_DEPRECATED
	#define BOOST_SYSTEM_NO_DEPRECATED
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS) && ((defined(KLAYGE_COMPILER_GCC) && defined(KLAYGE_CPU_X64)) || defined(KLAYGE_COMPILER_CLANG))
	#ifndef BOOST_USE_WINDOWS_H
		#define BOOST_USE_WINDOWS_H
	#endif
#endif

// Prevent Boost to link the Boost.DateTime
#ifndef BOOST_DATE_TIME_NO_LIB
	#define BOOST_DATE_TIME_NO_LIB
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP) || defined(KLAYGE_PLATFORM_LINUX) || defined(KLAYGE_PLATFORM_DARWIN)
	#define KLAYGE_IS_DEV_PLATFORM 1
#else
	#define KLAYGE_IS_DEV_PLATFORM 0
#endif

#endif		// _KFL_CONFIG_HPP
