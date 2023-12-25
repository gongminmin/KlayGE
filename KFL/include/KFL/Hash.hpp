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

#include <string>
#include <string_view>

#include <KFL/CXX20/span.hpp>

namespace KlayGE
{
	constexpr size_t HashCombineImpl(size_t value, size_t seed) noexcept
	{
		size_t constexpr PRIME_NUM = 0x9e3779b9U;
		return seed ^ (value + PRIME_NUM + (seed << 6) + (seed >> 2));
	}

	constexpr size_t CtHash(char const* str, size_t seed = 0) noexcept
	{
		return 0 == *str ? seed : CtHash(str + 1, HashCombineImpl(*str, seed));
	}

	inline size_t RtHash(char const* str) noexcept
	{
		size_t seed = 0;
		while (*str != 0)
		{
			seed = HashCombineImpl(*str, seed);
			++ str;
		}
		return seed;
	}

	template <typename T>
	inline size_t HashValue(T v) noexcept
	{
		return static_cast<size_t>(v);
	}

	template <typename T>
	inline size_t HashValue(T* v) noexcept
	{
		return static_cast<size_t>(reinterpret_cast<ptrdiff_t>(v));
	}

	template <typename T>
	inline void HashCombine(size_t& seed, T const& v) noexcept
	{
		seed = HashCombineImpl(HashValue(v), seed);
	}

	template <typename T>
	inline void HashRange(size_t& seed, T first, T last) noexcept
	{
		for (; first != last; ++ first)
		{
			HashCombine(seed, *first);
		}
	}

	template <typename T>
	inline size_t HashRange(T first, T last) noexcept
	{
		size_t seed = 0;
		HashRange(seed, first, last);
		return seed;
	}

	template <typename T>
	inline size_t HashValue(std::span<T const> s) noexcept
	{
		return HashRange(s.begin(), s.end());
	}

	template <typename T>
	inline size_t HashValue(std::basic_string_view<T> sv) noexcept
	{
		return HashRange(sv.begin(), sv.end());
	}

	template <typename T>
	inline size_t HashValue(std::basic_string<T> const& str) noexcept
	{
		return HashRange(str.begin(), str.end());
	}
}

#endif		// _KFL_HASH_HPP
