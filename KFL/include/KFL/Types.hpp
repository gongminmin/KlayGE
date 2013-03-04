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

#ifndef KLAYGE_CXX11_CORE_NULLPTR_SUPPORT
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

#ifdef KLAYGE_CXX11_CORE_STATIC_ASSERT_SUPPORT
	#define KLAYGE_STATIC_ASSERT(x) static_assert(x, #x)
#else
	#include <boost/static_assert.hpp>
	#define KLAYGE_STATIC_ASSERT(x) BOOST_STATIC_ASSERT(x)
#endif

#ifdef KLAYGE_CXX11_CORE_DECLTYPE_SUPPORT
	#define KLAYGE_AUTO(var, expr) auto var = expr
	#define KLAYGE_DECLTYPE(expr) KlayGE::remove_reference<decltype(expr)>::type
#else
	#include <boost/typeof/typeof.hpp>
	#define KLAYGE_AUTO(Var, Expr) BOOST_AUTO(Var, Expr)
	#define KLAYGE_DECLTYPE(expr) BOOST_TYPEOF(expr)
#endif

#ifdef KLAYGE_CXX11_CORE_FOREACH_SUPPORT
	#define KLAYGE_FOREACH(var, col) for (var : col)
#else
	#include <boost/foreach.hpp>
	#define KLAYGE_FOREACH(var, col) BOOST_FOREACH(var, col)
#endif
#ifdef KLAYGE_CXX11_CORE_NOEXCEPT_SUPPORT
	#define KLAYGE_NOEXCEPT noexcept
	#define KLAYGE_NOEXCEPT_IF(Predicate) noexcept((Predicate))
	#define KLAYGE_NOEXCEPT_EXPR(Expression) noexcept((Expression))
#else
	#define KLAYGE_NOEXCEPT throw()
	#define KLAYGE_NOEXCEPT_IF(Predicate)
	#define KLAYGE_NOEXCEPT_EXPR(Expression) false
#endif
#ifdef KLAYGE_CXX11_CORE_OVERRIDE_SUPPORT
	#define KLAYGE_OVERRIDE override
	#define KLAYGE_FINAL final
#else
	#define KLAYGE_OVERRIDE
	#define KLAYGE_FINAL
#endif
#ifdef KLAYGE_CXX11_CORE_RVALUE_REFERENCES_SUPPORT
	#include <utility>
	namespace KlayGE
	{
		using std::move;
	}
#else
	#include <boost/move/move.hpp>
	namespace KlayGE
	{
		using boost::move;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_ARRAY_SUPPORT
	#include <array>
	namespace KlayGE
	{
		using std::array;
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
	namespace KlayGE
	{
		using boost::array;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_ALGORITHM
	#include <algorithm>
	namespace KlayGE
	{
		using std::copy_if;
	}
#else
	namespace KlayGE
	{
		template<typename InputIterator, typename OutputIterator, typename Predicate>
		OutputIterator copy_if(InputIterator first, InputIterator last,
								OutputIterator dest_first,
								Predicate p)
		{
			for (InputIterator iter = first; iter != last; ++ iter)
			{
				if (p(*iter))
				{
					*dest_first = *iter;
					++ dest_first;
				}
			}

			return dest_first;
		}
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_ATOMIC_SUPPORT
	#include <atomic>
	namespace KlayGE
	{
		using std::atomic;
		using std::atomic_thread_fence;
		using std::atomic_signal_fence;

		using std::memory_order_relaxed;
		using std::memory_order_release;
		using std::memory_order_acquire;
		using std::memory_order_consume;
		using std::memory_order_acq_rel;
		using std::memory_order_seq_cst;
	}
#else
	#include <boost/atomic.hpp>
	namespace KlayGE
	{
		using boost::atomic;
		using boost::atomic_thread_fence;
		using boost::atomic_signal_fence;

		using boost::memory_order_relaxed;
		using boost::memory_order_release;
		using boost::memory_order_acquire;
		using boost::memory_order_consume;
		using boost::memory_order_acq_rel;
		using boost::memory_order_seq_cst;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_CHRONO_SUPPORT
	#include <chrono>
	namespace KlayGE
	{
		namespace chrono = std::chrono;
	}
#else
	#include <boost/chrono.hpp>
	namespace KlayGE
	{
		namespace chrono = boost::chrono;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_CSTDINT_SUPPORT
	#include <cstdint>
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
	}
#else
	#include <boost/cstdint.hpp>
	namespace KlayGE
	{
		using boost::uint64_t;
		using boost::uint32_t;
		using boost::uint16_t;
		using boost::uint8_t;
		using boost::int64_t;
		using boost::int32_t;
		using boost::int16_t;
		using boost::int8_t;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_FUNCTIONAL_SUPPORT
	#include <functional>
	namespace KlayGE
	{
		using std::result_of;
	}
#else
	#include <boost/utility/result_of.hpp>
	namespace KlayGE
	{
		using boost::result_of;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_RANDOM_SUPPORT
	#include <random>
	namespace KlayGE
	{
		using std::ranlux24_base;
		using std::uniform_int_distribution;
	}
#else
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(push)
		#pragma warning(disable: 4100 4127 4512 6297 6326 6385)
	#endif
	#include <boost/random.hpp>
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(pop)
	#endif
	namespace KlayGE
	{
		using boost::random;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_SMART_PTR_SUPPORT
	#include <functional>
	#include <memory>
	namespace KlayGE
	{
		using std::bind;
		using std::mem_fn;
		using std::function;
		namespace placeholders
		{
			using std::placeholders::_1;
			using std::placeholders::_2;
			using std::placeholders::_3;
			using std::placeholders::_4;
			using std::placeholders::_5;
			using std::placeholders::_6;
			using std::placeholders::_7;
			using std::placeholders::_8;
			using std::placeholders::_9;
		}

		using std::shared_ptr;
		using std::weak_ptr;
		using std::enable_shared_from_this;
		using std::static_pointer_cast;
		using std::dynamic_pointer_cast;
		using std::ref;
		using std::cref;
	}
#else
	#ifdef KLAYGE_PLATFORM_WIN32
		#ifndef KLAYGE_CPU_ARM
			#ifndef BOOST_MEM_FN_ENABLE_STDCALL
				#define BOOST_MEM_FN_ENABLE_STDCALL
			#endif
		#endif
	#endif
	#include <boost/mem_fn.hpp>
	#include <boost/bind.hpp>
	#include <boost/function.hpp>
	#include <boost/ref.hpp>
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(push)
		#pragma warning(disable: 6011)
	#endif
	#include <boost/smart_ptr.hpp>
	#ifdef KLAYGE_COMPILER_MSVC
		#pragma warning(pop)
	#endif
	namespace KlayGE
	{
		using boost::bind;
		using boost::mem_fn;
		using boost::function;
		namespace placeholders
		{
			static boost::arg<1> _1;
			static boost::arg<2> _2;
			static boost::arg<3> _3;
			static boost::arg<4> _4;
			static boost::arg<5> _5;
			static boost::arg<6> _6;
			static boost::arg<7> _7;
			static boost::arg<8> _8;
			static boost::arg<9> _9;
		}

		using boost::shared_ptr;
		using boost::weak_ptr;
		using boost::enable_shared_from_this;
		using boost::static_pointer_cast;
		using boost::dynamic_pointer_cast;
		using boost::ref;
		using boost::cref;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_TYPE_TRAITS_SUPPORT
	#include <type_traits>
	namespace KlayGE
	{
		using std::add_lvalue_reference;
		using std::has_trivial_destructor;
		using std::is_same;
		using std::remove_reference;
		using std::conditional;
	}
#else
	#include <boost/type_traits.hpp>
	#include <boost/mpl/if.hpp>
	namespace KlayGE
	{
		using boost::add_lvalue_reference;
		using boost::has_trivial_destructor;
		using boost::is_same;
		using boost::remove_reference;
		template <bool B, typename T, typename F>
		struct conditional
		{
			typedef typename boost::mpl::if_c<B, T, F>::type type;
		};
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_TUPLE_SUPPORT
	#include <tuple>
	namespace KlayGE
	{
		using std::tuple;
		using std::get;
		using std::make_tuple;
		using std::tuple_size;
	}
#else
	#include <boost/tuple/tuple.hpp>
	namespace KlayGE
	{
		using boost::tuple;
		using boost::get;
		using boost::make_tuple;
		template <typename tuple_type>
		struct tuple_size
		{
			static const size_t value = boost::tuples::length<tuple_type>::value;
		};
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_UNORDERED_SUPPORT
	#include <unordered_map>
	#include <unordered_set>
	namespace KlayGE
	{
		using std::unordered_map;
		using std::unordered_multimap;
		using std::unordered_set;
		using std::unordered_multiset;
	}
#else
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
