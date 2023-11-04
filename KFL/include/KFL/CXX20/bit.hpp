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

#include <KFL/Compiler.hpp>

#if defined(KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT) || defined(KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT) || \
	defined(KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT) || defined(KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT)
#include <bit>
#endif

#if !(defined(KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT) && defined(KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT) && \
	  defined(KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT) && defined(KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT))
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4127) // Constant if
#endif
#include <boost/core/bit.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#endif

#if !defined(KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT)
	namespace std
	{
		using boost::core::bit_cast;
	}
#endif

#if !defined(KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT)
	namespace std
	{
		using boost::core::rotl;
		using boost::core::rotr;

		using boost::core::countl_zero;
		using boost::core::countl_one;
		using boost::core::countr_zero;
		using boost::core::countr_one;
		using boost::core::popcount;
	}
#endif

#if !defined(KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT)
	namespace std
	{
		using boost::core::endian;
	}
#endif

#if !defined(KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT)
	namespace std
	{
		using boost::core::has_single_bit;
		using boost::core::bit_ceil;
		using boost::core::bit_floor;
		using boost::core::bit_width;;
	}
#endif

#endif		// KFL_CXX20_BIT_HPP
