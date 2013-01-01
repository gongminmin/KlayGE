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

#ifndef KLAYGE_NULLPTR_SUPPORT
const class nullptr_t
{
public:
	template <typename T>
	operator T*() const
	{
		return reinterpret_cast<T*>(0);
	}

	template <typename C, typename T>
	operator T C::*() const
	{
		return reinterpret_cast<T C::*>(0);
	}

private:
	void operator&() const;
} nullptr = {};
#endif

#ifdef KLAYGE_STATIC_ASSERT_SUPPORT
	#define KLAYGE_STATIC_ASSERT(x) static_assert(x, #x)
#else
	#include <boost/static_assert.hpp>
	#define KLAYGE_STATIC_ASSERT(x) BOOST_STATIC_ASSERT(x)
#endif

#ifdef KLAYGE_DECLTYPE_SUPPORT
	#define KLAYGE_AUTO(var, expr) auto var = expr
	#define KLAYGE_DECLTYPE(expr) KlayGE::remove_reference<decltype(expr)>::type
#else
	#include <boost/typeof/typeof.hpp>
	#define KLAYGE_AUTO(Var, Expr) BOOST_AUTO(Var, Expr)
	#define KLAYGE_DECLTYPE(expr) BOOST_TYPEOF(expr)
#endif

#ifdef KLAYGE_FOREACH_SUPPORT
	#define KLAYGE_FOREACH(var, col) for (var : col)
#else
	#include <boost/foreach.hpp>
	#define KLAYGE_FOREACH(var, col) BOOST_FOREACH(var, col)
#endif


#ifdef KLAYGE_CXX11_LIBRARY_SUPPORT
	#include <array>
	#include <cstdint>
	#include <tuple>
	#include <type_traits>
	#include <unordered_map>
	#include <unordered_set>

	namespace KlayGE
	{
		using std::array;

		using std::uint64_t;
		using std::uint32_t;
		using std::uint16_t;
		using std::uint8_t;
		using std::int64_t;
		using std::int32_t;
		using std::int16_t;
		using std::int8_t;

		using std::tuple;
		using std::get;
		using std::make_tuple;
		using std::tuple_size;

		using std::add_reference;
		using std::has_trivial_destructor;
		using std::is_same;
		using std::remove_reference;

		using std::unordered_map;
		using std::unordered_multimap;
		using std::unordered_set;
		using std::unordered_multiset;
	}
#else
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(push)
		#pragma warning(disable: 6385)
	#endif
	#include <boost/array.hpp>
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(pop)
	#endif
	#include <boost/cstdint.hpp>
	#include <boost/tuple/tuple.hpp>
	#include <boost/type_traits.hpp>
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(push)
		#pragma warning(disable: 4100 6011 6334)
	#endif
		#include <boost/unordered_map.hpp>
		#include <boost/unordered_set.hpp>
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(pop)
	#endif

	namespace KlayGE
	{
		using boost::array;

		using boost::uint64_t;
		using boost::uint32_t;
		using boost::uint16_t;
		using boost::uint8_t;
		using boost::int64_t;
		using boost::int32_t;
		using boost::int16_t;
		using boost::int8_t;

		using boost::tuple;
		using boost::get;
		using std::make_tuple;
		template<class tuple_type>
		struct tuple_size
		{
			static const size_t value = boost::tuples::length<tuple_type>::value;
		};

		using boost::add_reference;
		using boost::has_trivial_destructor;
		using boost::is_same;
		using boost::remove_reference;

		using boost::unordered_map;
		using boost::unordered_multimap;
		using boost::unordered_set;
		using boost::unordered_multiset;
	}
#endif

namespace KlayGE
{
#ifdef KLAYGE_COMPILER_MSVC
	#ifndef _WCHAR_T_DEFINED
		typedef unsigned short		wchar_t;
		#define _WCHAR_T_DEFINED
	#endif		// _WCHAR_T_DEFINED
#endif

	typedef uint32_t FourCC;
}

#endif		// _KFL_TYPES_HPP
