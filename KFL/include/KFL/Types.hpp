/**
 * @file Types.hpp
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

#ifndef _KFL_TYPES_HPP
#define _KFL_TYPES_HPP

#pragma once

#include <cstdint>
#include <KFL/CXX17.hpp>

#ifdef KLAYGE_COMPILER_MSVC
	#define KLAYGE_RESTRICT __restrict
	#define KLAYGE_ASSUME(x) (__assume(x))
#else
	#define KLAYGE_RESTRICT
	#define KLAYGE_ASSUME(x) (BOOST_ASSERT(x))
#endif

namespace KlayGE
{
	using std::uint64_t;
	using std::uint32_t;
	using std::uint16_t;
	using std::uint8_t;
	using std::int64_t;
	using std::int32_t;
	using std::int16_t;
	using std::int8_t;

#ifdef KLAYGE_COMPILER_MSVC
	#ifndef _WCHAR_T_DEFINED
		typedef unsigned short		wchar_t;
		#define _WCHAR_T_DEFINED
	#endif		// _WCHAR_T_DEFINED
#endif

	typedef uint32_t FourCC;
}

#endif		// _KFL_TYPES_HPP
