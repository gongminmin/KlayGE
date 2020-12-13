/**
 * @file Hash.hpp
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

#ifndef _KFL_HASH_HPP
#define _KFL_HASH_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <string_view>

namespace KlayGE
{
#define PRIME_NUM 0x9e3779b9

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(disable: 4307) // The hash here could cause integral constant overflow
#endif

	size_t constexpr CTHashImpl(char const * str, size_t seed)
	{
		return 0 == *str ? seed : CTHashImpl(str + 1, seed ^ (static_cast<size_t>(*str) + PRIME_NUM + (seed << 6) + (seed >> 2)));
	}

#if defined(KLAYGE_COMPILER_MSVC) && (_MSC_VER < 1914)
	template <size_t N>
	struct EnsureConst
	{
		static size_t constexpr value = N;
	};

	#define CT_HASH(x) (EnsureConst<CTHashImpl(x, 0)>::value)
#else
	#define CT_HASH(x) (CTHashImpl(x, 0))
#endif

	template <typename SizeT>
	inline void HashCombineImpl(SizeT& seed, SizeT value)
	{
		seed ^= value + PRIME_NUM + (seed << 6) + (seed >> 2);
	}

	inline size_t RT_HASH(char const * str)
	{
		size_t seed = 0;
		while (*str != 0)
		{
			HashCombineImpl(seed, static_cast<size_t>(*str));
			++ str;
		}
		return seed;
	}

#undef PRIME_NUM

	template <typename T>
	inline size_t HashValue(T v)
	{
		return static_cast<size_t>(v);
	}

	template <typename T>
	inline size_t HashValue(T* v)
	{
		return static_cast<size_t>(reinterpret_cast<ptrdiff_t>(v));
	}

	template <typename T>
	inline void HashCombine(size_t& seed, T const & v)
	{
		return HashCombineImpl(seed, HashValue(v));
	}

	template <typename T>
	inline void HashRange(size_t& seed, T first, T last)
	{
		for (; first != last; ++ first)
		{
			HashCombine(seed, *first);
		}
	}

	template <typename T>
	inline size_t HashRange(T first, T last)
	{
		size_t seed = 0;
		HashRange(seed, first, last);
		return seed;
	}
}

#endif		// _KFL_HASH_HPP
