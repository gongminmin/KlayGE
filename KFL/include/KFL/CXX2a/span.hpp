/**
 * @file span.cpp
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

#ifndef KFL_CXX2A_SPAN_HPP
#define KFL_CXX2A_SPAN_HPP

#pragma once

#include <KFL/Config.hpp>

#if defined(KLAYGE_CXX2A_LIBRARY_SPAN_SUPPORT)
	#include <span>
#else
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstrict-overflow" // Ignore the strict overflow
	#endif
	#include <gsl/span>
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic pop
	#endif
	namespace std
	{
		using gsl::span;
		using gsl::as_bytes;
		using gsl::as_writeable_bytes;
	}
#endif

namespace KlayGE
{
	template <typename ElementType>
	constexpr std::span<ElementType> MakeSpan()
	{
		return std::span<ElementType>();
	}

	template <typename ElementType>
	constexpr std::span<ElementType> MakeSpan(ElementType* ptr, typename std::span<ElementType>::index_type count)
	{
		return std::span<ElementType>(ptr, count);
	}

	template <typename ElementType>
	constexpr std::span<ElementType> MakeSpan(ElementType* first_elem, ElementType* last_elem)
	{
		return std::span<ElementType>(first_elem, last_elem);
	}

	template <typename ElementType, std::size_t N>
	constexpr std::span<ElementType, N> MakeSpan(ElementType (&arr)[N]) noexcept
	{
		return std::span<ElementType, N>(arr);
	}

	template <typename Container>
	constexpr std::span<typename Container::value_type> MakeSpan(Container& cont)
	{
		return std::span<typename Container::value_type>(cont);
	}

	template <typename Container>
	constexpr std::span<typename Container::value_type const> MakeSpan(Container const& cont)
	{
		return std::span<typename Container::value_type const>(cont);
	}

	template <typename Ptr>
	constexpr std::span<typename Ptr::element_type> MakeSpan(Ptr& cont, std::ptrdiff_t count)
	{
		return std::span<typename Ptr::element_type>(cont, count);
	}

	template <typename Ptr>
	constexpr std::span<typename Ptr::element_type> MakeSpan(Ptr& cont)
	{
		return std::span<typename Ptr::element_type>(cont);
	}

	template <typename T>
	constexpr std::span<T> MakeSpan(std::initializer_list<T> v)
	{
		return std::span<T>(v.begin(), v.end());
	}

	template <typename T>
	constexpr std::span<T const> MakeSpan(std::initializer_list<T const> v)
	{
		return std::span<T const>(v.begin(), v.end());
	}

	template <int dummy, typename T>
	constexpr std::span<T, 1> MakeSpan(T& val)
	{
		return std::span<T, 1>(&val, 1);
	}

	template <int dummy, typename T>
	constexpr std::span<T const, 1> MakeSpan(T const& val)
	{
		return std::span<T const, 1>(&val, 1);
	}

	template <typename ElementType1, typename ElementType2, std::ptrdiff_t FirstExtent, std::ptrdiff_t SecondExtent>
	constexpr bool operator==(std::span<ElementType1, FirstExtent> lhs, std::span<ElementType2, SecondExtent> rhs)
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	template <typename ElementType1, typename ElementType2, std::ptrdiff_t Extent>
	constexpr bool operator!=(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs)
	{
		return !(lhs == rhs);
	}

	template <typename ElementType1, typename ElementType2, std::ptrdiff_t Extent>
	constexpr bool operator<(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs)
	{
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	template <typename ElementType1, typename ElementType2, std::ptrdiff_t Extent>
	constexpr bool operator<=(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs)
	{
		return !(lhs > rhs);
	}

	template <typename ElementType1, typename ElementType2, std::ptrdiff_t Extent>
	constexpr bool operator>(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs)
	{
		return rhs < lhs;
	}

	template <typename ElementType1, typename ElementType2, std::ptrdiff_t Extent>
	constexpr bool operator>=(std::span<ElementType1, Extent> l, std::span<ElementType2, Extent> r)
	{
		return !(l < r);
	}
}

#endif		// KFL_CXX2A_SPAN_HPP
