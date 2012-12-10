// Types.hpp
// KlayGE 类型定义头文件
// Ver 2.6.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.2.0
// 用boost的类型定义代替直接定义 (2004.10.30)
//
// 1.3.8.3
// 初次建立
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _TYPES_HPP
#define _TYPES_HPP

#pragma once

#include <boost/cstdint.hpp>
#ifndef KLAYGE_STATIC_ASSERT_SUPPORT
	#include <boost/static_assert.hpp>
#endif
#ifndef KLAYGE_DECLTYPE_SUPPORT
	#include <boost/typeof/typeof.hpp>
#endif
#ifndef KLAYGE_FOREACH_SUPPORT
	#include <boost/foreach.hpp>
#endif
#ifdef KLAYGE_TYPE_TRAITS_SUPPORT
	#include <type_traits>
#else
	#include <boost/type_traits/remove_reference.hpp>
#endif

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
	#define KLAYGE_STATIC_ASSERT(x) BOOST_STATIC_ASSERT(x)
#endif

#ifdef KLAYGE_DECLTYPE_SUPPORT
	#define KLAYGE_AUTO(var, expr) auto var = expr
	#ifdef KLAYGE_TYPE_TRAITS_SUPPORT
		#define KLAYGE_DECLTYPE(expr) std::remove_reference<decltype(expr)>::type
	#else
		#define KLAYGE_DECLTYPE(expr) boost::remove_reference<decltype(expr)>::type
	#endif
#else
	#define KLAYGE_AUTO(Var, Expr) BOOST_AUTO(Var, Expr)
	#define KLAYGE_DECLTYPE(expr) BOOST_TYPEOF(expr)
#endif

#ifdef KLAYGE_FOREACH_SUPPORT
	#define KLAYGE_FOREACH(var, col) for (var : col)
#else
	#define KLAYGE_FOREACH(var, col) BOOST_FOREACH(var, col)
#endif

namespace KlayGE
{
#ifdef KLAYGE_COMPILER_MSVC
	#ifndef _WCHAR_T_DEFINED
		typedef unsigned short		wchar_t;
		#define _WCHAR_T_DEFINED
	#endif		// _WCHAR_T_DEFINED
#endif

	using boost::uint64_t;
	using boost::uint32_t;
	using boost::uint16_t;
	using boost::uint8_t;

	using boost::int64_t;
	using boost::int32_t;
	using boost::int16_t;
	using boost::int8_t;

	typedef uint32_t FourCC;
}

#endif		// _TYPES_HPP
