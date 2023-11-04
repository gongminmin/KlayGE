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

// Detects supported compilers
#if defined(__clang__)
	// Clang++

	#define CLANG_VERSION KFL_JOIN(__clang_major__, __clang_minor__)

	#if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif

	#if defined(_MSC_VER)
		#define KLAYGE_COMPILER_CLANGCL
		#define KLAYGE_COMPILER_NAME clangcl

		#if CLANG_VERSION >= 120
			#define KLAYGE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang-cl 12.0 or up."
		#endif
	#else
		#define KLAYGE_COMPILER_CLANG
		#define KLAYGE_COMPILER_NAME clang

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
		#elif defined(linux) || defined(__linux) || defined(__linux__)
			#if CLANG_VERSION >= 140
				#define KLAYGE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 14.0 or up."
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

	#if __cplusplus < 201703L
		#error "/std:c++17 must be turned on."
	#endif

	#pragma warning(disable: 4251) // STL classes are not dllexport.
	#pragma warning(disable: 4819) // Allow non-ANSI characters.
#else
	#error "Unknown compiler. Please install vc, g++, or clang."
#endif

#if defined(KLAYGE_COMPILER_MSVC) || defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG) || defined(KLAYGE_COMPILER_CLANGCL)
#define KLAYGE_HAS_STRUCT_PACK
#endif
