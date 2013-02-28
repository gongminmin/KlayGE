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

// Defines supported compilers
#if defined(__GNUC__)
	// GNU C++

	#define KLAYGE_COMPILER_GCC

	#if __GNUC__ >= 4
		#if __GNUC_MINOR__ >= 8
			#define KLAYGE_COMPILER_VERSION 48
		#elif __GNUC_MINOR__ >= 7
			#define KLAYGE_COMPILER_VERSION 47
		#elif __GNUC_MINOR__ >= 6
			#define KLAYGE_COMPILER_VERSION 46
		#elif __GNUC_MINOR__ >= 5
			#define KLAYGE_COMPILER_VERSION 45
		#elif __GNUC_MINOR__ >= 4
			#define KLAYGE_COMPILER_VERSION 44
		#elif __GNUC_MINOR__ >= 3
			#define KLAYGE_COMPILER_VERSION 43
		#elif __GNUC_MINOR__ >= 2
			#define KLAYGE_COMPILER_VERSION 42
		#elif __GNUC_MINOR__ >= 1
			#define KLAYGE_COMPILER_VERSION 41
		#elif __GNUC_MINOR__ >= 0
			#define KLAYGE_COMPILER_VERSION 40
		#endif

		#if __GNUC_MINOR__ >= 3
			#ifdef __GXX_EXPERIMENTAL_CXX0X__
				#define KLAYGE_CXX11_CORE_STATIC_ASSERT_SUPPORT
				#define KLAYGE_CXX11_CORE_DECLTYPE_SUPPORT
				#define KLAYGE_CXX11_CORE_RVALUE_REFERENCES_SUPPORT
				#define KLAYGE_CXX11_CORE_EXTERN_TEMPLATES_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_ALGORITHM
				#define KLAYGE_CXX11_LIBRARY_ARRAY_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_CSTDINT_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_FUNCTIONAL_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_RANDOM_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_REGEX_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_TUPLE_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_TYPE_TRAITS_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_UNORDERED_SUPPORT
				#if __GNUC_MINOR__ >= 4
					#define KLAYGE_CXX11_CORE_STRONGLY_TYPED_ENUMS_SUPPORT
					#define KLAYGE_CXX11_LIBRARY_ATOMIC_SUPPORT
					#if !defined(__MINGW32__)
						#define KLAYGE_CXX11_LIBRARY_SYSTEM_ERROR_SUPPORT
					#endif
					#ifdef _GLIBCXX_HAS_GTHREADS
						#define KLAYGE_CXX11_LIBRARY_CHRONO_SUPPORT
						#define KLAYGE_CXX11_LIBRARY_THREAD_SUPPORT
					#endif
				#endif
				#if __GNUC_MINOR__ >= 6
					#define KLAYGE_CXX11_CORE_NULLPTR_SUPPORT
					#define KLAYGE_CXX11_CORE_FOREACH_SUPPORT
					#define KLAYGE_CXX11_CORE_NOEXCEPT_SUPPORT
				#endif
				#if __GNUC_MINOR__ >= 7
					#define KLAYGE_CXX11_CORE_OVERRIDE_SUPPORT
				#endif
			#endif
		#endif
	#else
		#error Unknown compiler.
	#endif
#elif defined(_MSC_VER)
	#define KLAYGE_COMPILER_MSVC
	#define KLAYGE_COMPILER_NAME vc

	#define KLAYGE_HAS_DECLSPEC

	#if _MSC_VER >= 1700
		#define KLAYGE_COMPILER_VERSION 110
	#elif _MSC_VER >= 1600
		#define KLAYGE_COMPILER_VERSION 100
	#elif _MSC_VER >= 1500
		#define KLAYGE_COMPILER_VERSION 90

		#ifndef KLAYGE_DEBUG
			#define _SECURE_SCL 0
		#endif
	#elif _MSC_VER >= 1400
		#define KLAYGE_COMPILER_VERSION 80

		#ifndef KLAYGE_DEBUG
			#define _SECURE_SCL 0
		#endif
	#else
		#error Unknown compiler.
	#endif

	#if _MSC_VER >= 1500
		#define KLAYGE_CXX11_CORE_EXTERN_TEMPLATES_SUPPORT
		#if _MSC_VER >= 1600
			#define KLAYGE_CXX11_CORE_NULLPTR_SUPPORT
			#define KLAYGE_CXX11_CORE_STATIC_ASSERT_SUPPORT
			#define KLAYGE_CXX11_CORE_DECLTYPE_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_ARRAY_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_CSTDINT_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_FUNCTIONAL_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_RANDOM_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_REGEX_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_SYSTEM_ERROR_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_TUPLE_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_TYPE_TRAITS_SUPPORT
			#define KLAYGE_CXX11_LIBRARY_UNORDERED_SUPPORT
			#if _MSC_VER >= 1700
				#define KLAYGE_CXX11_CORE_STRONGLY_TYPED_ENUMS_SUPPORT
				#define KLAYGE_CXX11_CORE_FOREACH_SUPPORT
				#define KLAYGE_CXX11_CORE_OVERRIDE_SUPPORT
				#define KLAYGE_CXX11_CORE_RVALUE_REFERENCES_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_ALGORITHM
				#define KLAYGE_CXX11_LIBRARY_ATOMIC_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_CHRONO_SUPPORT
				#define KLAYGE_CXX11_LIBRARY_THREAD_SUPPORT
				#define KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
			#endif
		#endif
	#endif

	#if _MSC_VER >= 1400
		#pragma warning(disable: 4251 4275 4819)

		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _SCL_SECURE_NO_DEPRECATE
			#define _SCL_SECURE_NO_DEPRECATE
		#endif
	#endif
#else
	#error Unknown compiler.
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

	// Forces all boost's libraries to be linked as dll
	#ifndef BOOST_ALL_DYN_LINK
		#define BOOST_ALL_DYN_LINK
	#endif

	#if defined(__MINGW32__)
		#define KLAYGE_COMPILER_NAME mgw
		#include <_mingw.h>
	#else
		#include <sdkddkver.h>
	#endif

	#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		#include <winapifamily.h>
		#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			#define KLAYGE_PLATFORM_WINDOWS_DESKTOP
		#else
			#define KLAYGE_PLATFORM_WINDOWS_METRO
		#endif
	#else
		#define KLAYGE_PLATFORM_WINDOWS_DESKTOP
	#endif

	// Shut min/max in windows.h
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef WINDOWS_LEAN_AND_MEAN
		#define WINDOWS_LEAN_AND_MEAN
	#endif
#elif defined(__ANDROID__)
	#define KLAYGE_PLATFORM_ANDROID
	#define KLAYGE_COMPILER_NAME gcc
#elif defined(__CYGWIN__)
	#define KLAYGE_PLATFORM_CYGWIN
	#define KLAYGE_COMPILER_NAME cyg
#elif defined(linux) || defined(__linux) || defined(__linux__)
	#define KLAYGE_PLATFORM_LINUX
	#define KLAYGE_COMPILER_NAME gcc
#else
	#error Unknown platform.
#endif

// Defines supported CPUs
#if defined(KLAYGE_COMPILER_MSVC)
	#if defined(_M_X64)
		#define KLAYGE_CPU_X64		
		#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			#define KLAYGE_COMPILER_TARGET x64
		#else
			#define KLAYGE_COMPILER_TARGET x64_app
		#endif
	#elif defined(_M_IX86)
		#define KLAYGE_CPU_X86
		#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			#define KLAYGE_COMPILER_TARGET x86
		#else
			#define KLAYGE_COMPILER_TARGET x86_app
		#endif
	#elif defined(_M_ARM)
		#define KLAYGE_CPU_ARM
		#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			#define KLAYGE_COMPILER_TARGET arm
		#else
			#define KLAYGE_COMPILER_TARGET arm_app
		#endif
	#else
		#error Unknown CPU type.
	#endif
#elif defined(KLAYGE_COMPILER_GCC)
	#if defined(__x86_64__)
		#define KLAYGE_CPU_X64
		#define KLAYGE_COMPILER_TARGET x64
	#elif defined(__i386__)
		#define KLAYGE_CPU_X86
		#define KLAYGE_COMPILER_TARGET x86
	#elif defined(__arm__)
		#define KLAYGE_CPU_ARM
		#define KLAYGE_COMPILER_TARGET arm
	#else
		#error Unknown CPU type.
	#endif
#endif

// Defines the native endian
#if defined(KLAYGE_CPU_ARM)
	#ifdef __ARMEB__
		#define KLAYGE_BIG_ENDIAN
	#else
		#define KLAYGE_LITTLE_ENDIAN
	#endif
#elif defined(KLAYGE_CPU_X86) || defined(KLAYGE_CPU_X64) || defined(KLAYGE_PLATFORM_WINDOWS)
	#define KLAYGE_LITTLE_ENDIAN
#else
	#define KLAYGE_BIG_ENDIAN
#endif

#define KLAYGE_IDENTITY_SUPPORT
#define KLAYGE_SELECT1ST2ND_SUPPORT
#define KLAYGE_PROJECT1ST2ND_SUPPORT

// Defines some MACROs from compile options
#ifdef KLAYGE_CPU_X64
	#define KLAYGE_SSE_SUPPORT
	#define KLAYGE_SSE2_SUPPORT
	#define KLAYGE_X64_SUPPORT
#elif defined KLAYGE_CPU_X86
	#if defined(KLAYGE_COMPILER_MSVC)
		#if _M_IX86 == 600
			#define KLAYGE_MMX_SUPPORT
		#endif

		#if _M_IX86_FP == 1
			#define KLAYGE_SSE_SUPPORT
		#elif _M_IX86_FP == 2
			#define KLAYGE_SSE_SUPPORT
			#define KLAYGE_SSE2_SUPPORT
		#endif
	#elif defined(KLAYGE_COMPILER_GCC)
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
	#endif
#elif defined KLAYGE_CPU_ARM
	#if defined(KLAYGE_COMPILER_MSVC)
		#define KLAYGE_NEON_SUPPORT
	#elif defined(KLAYGE_COMPILER_GCC)
		#ifdef __ARM_NEON__
			#define KLAYGE_NEON_SUPPORT
		#endif
	#endif
#endif

#if defined(KLAYGE_COMPILER_MSVC) || defined(KLAYGE_COMPILER_GCC)
	#define KLAYGE_HAS_STRUCT_PACK
#endif

#if defined(KLAYGE_COMPILER_GCC) || (defined(KLAYGE_COMPILER_MSVC) && (KLAYGE_COMPILER_VERSION >= 110))
	// Prevent Boost to link the Boost.DateTime
	#ifndef BOOST_DATE_TIME_NO_LIB
		#define BOOST_DATE_TIME_NO_LIB
	#endif
#endif

#endif		// _KFL_CONFIG_HPP
