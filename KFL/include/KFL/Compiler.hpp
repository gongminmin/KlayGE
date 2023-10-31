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

#pragma once

// Macros for C++17 library features:
// KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
// KLAYGE_CXX17_LIBRARY_CHARCONV_FP_SUPPORT

// Macros for C++20 library features:
// KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
// KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
// KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
// KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
// KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
// KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT

// Macros for C++23 library features:
// KLAYGE_CXX23_LIBRARY_TO_UNDERLYING
// KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT

// Detects supported compilers
#if defined(__clang__)
	// Clang++

	#define CLANG_VERSION KFL_JOIN(__clang_major__, __clang_minor__)

	#if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif

	#define KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT

	#if defined(_MSC_VER)
		#define KLAYGE_COMPILER_CLANGCL
		#define KLAYGE_COMPILER_NAME clangcl

		#if CLANG_VERSION >= 120
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang-cl 12.0 or up."
		#endif

		#define KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
		#define KLAYGE_CXX17_LIBRARY_CHARCONV_FP_SUPPORT
		#define KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
		#define KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
		#if __cplusplus > 202002L
			#if CLANG_VERSION >= 130
				#define KLAYGE_CXX23_LIBRARY_TO_UNDERLYING
			#endif
			#if CLANG_VERSION >= 150
				#define KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT
			#endif
		#endif
	#else
		#define KLAYGE_COMPILER_CLANG
		#define KLAYGE_COMPILER_NAME clang

		#if CLANG_VERSION >= 140
			#define KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
			#define KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
		#endif

		#if defined(__APPLE__)
			#if CLANG_VERSION >= 130
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install Apple clang++ 13 or up."
			#endif
		#elif defined(__ANDROID__)
			#if CLANG_VERSION >= 120
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 12.0 (NDK 23c) or up."
			#endif

			#if __cplusplus > 202002L
				#if CLANG_VERSION >= 130
					#define KLAYGE_CXX23_LIBRARY_TO_UNDERLYING
				#endif
			#endif
		#elif defined(linux) || defined(__linux) || defined(__linux__)
			#if CLANG_VERSION >= 140
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 14.0 or up."
			#endif

			#if CLANG_VERSION >= 170
				#define KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
			#endif
			#if __cplusplus > 202002L
				#define KLAYGE_CXX23_LIBRARY_TO_UNDERLYING
				#if CLANG_VERSION >= 150
					#define KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT
				#endif
			#endif
		#else
			#error "Clang++ on an unknown platform. Only Apple, Android, and Linux are supported."
		#endif
	#endif
#elif defined(__GNUC__)
	// GNU C++

	#define KLAYGE_COMPILER_GCC
	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
		#define KLAYGE_COMPILER_NAME mgw
	#else
		#define KLAYGE_COMPILER_NAME gcc
	#endif

	#define GCC_VERSION KFL_JOIN(__GNUC__, __GNUC_MINOR__)

	#if GCC_VERSION >= 110
		#define KLAYGE_COMPILER_VERSION GCC_VERSION
	#else
		#error "Unsupported compiler version. Please install g++ 11.0 or up."
	#endif

	#if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif

	#define KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
	#define KLAYGE_CXX17_LIBRARY_CHARCONV_FP_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
	#define KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT
	#if GCC_VERSION >= 130
		#define KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
	#endif
	#if __cplusplus > 202002L
		#define KLAYGE_CXX23_LIBRARY_TO_UNDERLYING
		#if GCC_VERSION >= 120
			#define KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT
		#endif
	#endif
#elif defined(_MSC_VER)
	// MSVC

	#define KLAYGE_COMPILER_MSVC
	#define KLAYGE_COMPILER_NAME vc

	#if _MSC_VER >= 1930
		#define KLAYGE_COMPILER_VERSION 143
	#elif _MSC_VER >= 1920
		#define KLAYGE_COMPILER_VERSION 142
	#else
		#error "Unsupported compiler version. Please install VS2019 or up."
	#endif

	#if _MSVC_LANG < 201703L
		#error "/std:c++17 must be turned on."
	#endif

	#define KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
	#if _MSC_VER >= 1924
		#define KLAYGE_CXX17_LIBRARY_CHARCONV_FP_SUPPORT
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
		#if _MSC_VER >= 1929
			#define KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
		#endif
	#endif
	#if _MSVC_LANG > 202002L
		#if _MSC_VER >= 1930
			#define KLAYGE_CXX23_LIBRARY_TO_UNDERLYING
		#endif
		#if _MSC_VER >= 1932
			#define KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT
		#endif
	#endif

	#pragma warning(disable: 4251) // STL classes are not dllexport.
	#pragma warning(disable: 4275) // boost::noncopyable is a non dll-interface class.
	#pragma warning(disable: 4819) // Allow non-ANSI characters.
#else
	#error "Unknown compiler. Please install vc, g++, or clang."
#endif
