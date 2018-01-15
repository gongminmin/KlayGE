/**
 * @file Compiler.hpp
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

#ifndef KFL_COMPILER_HPP
#define KFL_COMPILER_HPP

// KlayGE requires vc 14.0+, g++ 7.1+, clang 3.6+, with C++14 or C++17 option on.

// Detects supported compilers
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
		#if CLANG_VERSION >= 50
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang++ 5.0 (NDK 16) or up."
		#endif

		#define KLAYGE_CXX17_LIBRARY_ANY_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_OPTIONAL_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_STRING_VIEW_SUPPORT

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
		#error "Clang++ on an unknown platform. Only Apple+, Android, and Windows are supported."
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

	#if GCC_VERSION >= 71
		#define KLAYGE_COMPILER_VERSION GCC_VERSION
	#else
		#error "Unsupported compiler version. Please install g++ 7.1 or up."
	#endif

	#if __cplusplus < 201703L
		#error "-std=c++1z must be turned on."
	#endif
	#if !defined(_GLIBCXX_HAS_GTHREADS)
		#error "_GLIBCXX_HAS_GTHREADS must be turned on."
	#endif

	#define KLAYGE_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT
	#define KLAYGE_CXX17_LIBRARY_ANY_SUPPORT
	#define KLAYGE_CXX17_LIBRARY_OPTIONAL_SUPPORT
	#define KLAYGE_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
	#define KLAYGE_CXX17_LIBRARY_STRING_VIEW_SUPPORT
	#define KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT

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
	// MSVC

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
	#error "Unknown compiler. Please install vc, g++, or clang."
#endif

#endif		// KFL_COMPILER_HPP
