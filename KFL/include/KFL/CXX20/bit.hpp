/**
 * @file bit.hpp
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

#ifndef KFL_CXX20_BIT_HPP
#define KFL_CXX20_BIT_HPP

#pragma once

#include <KFL/Config.hpp>

#if !defined(KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT) || !defined(KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT) || \
	!defined(KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT) || !defined(KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT)
	#define bit_CONFIG_SELECT_BIT bit_BIT_NONSTD
	#define bit_CONFIG_STRICT 1
#endif

#if defined(KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT)
	#include <bit>
#else
	#include <nonstd/bit.hpp>

	namespace std
	{
		using nonstd::bit_cast;
	}
#endif

#if defined(KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT)
	#include <bit>
#else
	#include <nonstd/bit.hpp>

	namespace std
	{
		using nonstd::rotl;
		using nonstd::rotr;

		using nonstd::countl_zero;
		using nonstd::countl_one;
		using nonstd::countr_zero;
		using nonstd::countr_one;
		using nonstd::popcount;
	}
#endif

#if defined(KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT)
	#include <bit>
#else
	#include <nonstd/bit.hpp>

	namespace std
	{
		using nonstd::endian;
	}
#endif

#if defined(KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT)
	#include <bit>
#else
	#include <nonstd/bit.hpp>

	namespace std
	{
		using nonstd::has_single_bit;
		using nonstd::bit_ceil;
		using nonstd::bit_floor;
		using nonstd::bit_width;;
	}
#endif

#endif		// KFL_CXX20_BIT_HPP
