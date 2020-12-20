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

// KlayGE requires vc 14.1+, g++ 9.0+, clang 9.0+, with C++17 option on.
// All C++17 core feature used in KlayGE should be available in all supported compilers.

// Macros for optional C++17 library features:
// KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
// KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT

// Macros for C++20 library features:
// KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
// KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
// KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
// KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
// KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
// KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT

// Macros for TS library features:
// KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT

// Detects supported compilers
#if defined(__clang__)
	// Clang++

	#define CLANG_VERSION KFL_JOIN(__clang_major__, __clang_minor__)

	#if __cplusplus < 201703L
		#error "-std=c++17 must be turned on."
	#endif

	#if defined(_MSC_VER)
		#define KLAYGE_COMPILER_CLANGCL
		#define KLAYGE_COMPILER_NAME clangcl

		#if CLANG_VERSION >= 90
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang-cl 9.0 or up."
		#endif

		#if CLANG_VERSION < 100
			#define KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
		#endif
		#define KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT

		#define KLAYGE_HAS_DECLSPEC
		#define KLAYGE_SYMBOL_EXPORT __declspec(dllexport)
		#define KLAYGE_SYMBOL_IMPORT __declspec(dllimport)
	#else
		#define KLAYGE_COMPILER_CLANG
		#define KLAYGE_COMPILER_NAME clang

		#if defined(__APPLE__)
			#if CLANG_VERSION >= 110
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install Apple clang++ 11 or up."
			#endif

			#define KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT
			#if __cplusplus > 201703L
				#if CLANG_VERSION >= 120
					#define KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
				#endif
				#define KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
			#endif
		#elif defined(__ANDROID__)
			#if CLANG_VERSION >= 50
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 5.0 (NDK 16) or up."
			#endif
		#elif defined(linux) || defined(__linux) || defined(__linux__)
			#if CLANG_VERSION >= 100
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 10.0 or up."
			#endif

			#define KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT
			#if __cplusplus > 201703L
				#define KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
				#define KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
				#if CLANG_VERSION >= 120
					#define KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
				#endif
				#define KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT
			#endif
		#else
			#error "Clang++ on an unknown platform. Only Apple, Android, and Linux are supported."
		#endif

		#define KLAYGE_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define KLAYGE_SYMBOL_IMPORT __attribute__((__visibility__("default")))
	#endif

	#define KLAYGE_ATTRIBUTE_NORETURN __attribute__((noreturn))
	#define KLAYGE_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(__GNUC__)
	// GNU C++

	#define KLAYGE_COMPILER_GCC
	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
		#define KLAYGE_COMPILER_NAME mgw
	#else
		#define KLAYGE_COMPILER_NAME gcc
	#endif

	#include <bits/c++config.h>
	#ifdef _GLIBCXX_USE_FLOAT128
		#undef _GLIBCXX_USE_FLOAT128
	#endif
	#ifdef _GLIBCXX_USE_INT128
		#undef _GLIBCXX_USE_INT128
	#endif

	#define GCC_VERSION KFL_JOIN(__GNUC__, __GNUC_MINOR__)

	#if GCC_VERSION >= 90
		#define KLAYGE_COMPILER_VERSION GCC_VERSION
	#else
		#error "Unsupported compiler version. Please install g++ 9.0 or up."
	#endif

	#if __cplusplus < 201703L
		#error "-std=c++17 must be turned on."
	#endif
	#if !defined(_GLIBCXX_HAS_GTHREADS)
		#error "_GLIBCXX_HAS_GTHREADS must be turned on."
	#endif

	#define KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT
	#if __cplusplus > 201703L
		#if GCC_VERSION >= 110
			#define KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
		#endif
		#define KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
		#define KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
		#if GCC_VERSION >= 100
			#define KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
			#define KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT
		#endif
	#endif

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
		#define KLAYGE_SYMBOL_EXPORT __attribute__((__dllexport__))
		#define KLAYGE_SYMBOL_IMPORT __attribute__((__dllimport__))
	#else
		#define KLAYGE_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define KLAYGE_SYMBOL_IMPORT __attribute__((__visibility__("default")))
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

	#if _MSC_VER >= 1920
		#define KLAYGE_COMPILER_VERSION 142
	#elif _MSC_VER >= 1911
		#define KLAYGE_COMPILER_VERSION 141
	#else
		#error "Unsupported compiler version. Please install VS2017 15.3 or up."
	#endif

	#if _MSVC_LANG < 201703L
		#error "/std:c++17 must be turned on."
	#endif

	#if _MSC_VER >= 1914
		#define KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT
	#else
		#define KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT
	#endif
	#if _MSC_VER >= 1915
		#define KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
	#endif
	#if _MSVC_LANG > 201703L
		#if _MSC_VER >= 1922
			#define KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
		#endif
		#if _MSC_VER >= 1926
			#define KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT
		#endif
		#if _MSC_VER >= 1927
			#define KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
		#endif
		#if _MSC_VER >= 1928
			#define KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
			#define KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
		#endif
	#endif

	#pragma warning(disable: 4251) // STL classes are not dllexport.
	#pragma warning(disable: 4275) // boost::noncopyable is a non dll-interface class.
	#pragma warning(disable: 4819) // Allow non-ANSI characters.

	#define KLAYGE_ATTRIBUTE_NORETURN __declspec(noreturn)
	#define KLAYGE_BUILTIN_UNREACHABLE __assume(false)
#else
	#error "Unknown compiler. Please install vc, g++, or clang."
#endif

#endif		// KFL_COMPILER_HPP
