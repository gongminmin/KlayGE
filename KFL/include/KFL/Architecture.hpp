/**
 * @file Architecture.hpp
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

#ifndef KFL_ARCHITECTURE_HPP
#define KFL_ARCHITECTURE_HPP

// Detects supported CPU architectures
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
		#error "Unknown CPU type. In g++/clang, x86, x64, arm, and arm64 are supported."
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

// Detects optional instruction sets
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

#endif		// KFL_ARCHITECTURE_HPP
