/**
 * @file endian.cpp
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

#ifndef KFL_CXX2A_ENDIAN_HPP
#define KFL_CXX2A_ENDIAN_HPP

#pragma once

#include <KFL/Config.hpp>

#if defined(KLAYGE_CXX2A_LIBRARY_ENDIAN_SUPPORT)
	#include <bit>
#else
	namespace std
	{
		enum class endian
		{
			little = 0,
			big = 1,

#if defined(KLAYGE_CPU_ARM) || defined(KLAYGE_CPU_ARM64)
	#if defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__)
			native = big
	#else
			native = little
	#endif
#elif defined(KLAYGE_CPU_X86) || defined(KLAYGE_CPU_X64) || defined(KLAYGE_PLATFORM_WINDOWS)
			native = little
#else
			#error "Unknown CPU endian."
#endif
		};
	}
#endif

#endif		// KFL_CXX2A_ENDIAN_HPP
