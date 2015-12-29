/**
 * @file CXX11.hpp
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

#ifndef _KFL_CXX11_HPP
#define _KFL_CXX11_HPP

#pragma once

#ifdef KLAYGE_CXX11_CORE_NOEXCEPT_SUPPORT
	#define KLAYGE_NOEXCEPT noexcept
	#define KLAYGE_NOEXCEPT_IF(predicate) noexcept((predicate))
	#define KLAYGE_NOEXCEPT_EXPR(expression) noexcept((expression))
#else
	#define KLAYGE_NOEXCEPT throw()
	#define KLAYGE_NOEXCEPT_IF(predicate)
	#define KLAYGE_NOEXCEPT_EXPR(expression) false
#endif
#ifdef KLAYGE_CXX11_CORE_CONSTEXPR_SUPPORT
	#define KLAYGE_CONSTEXPR constexpr
#else
	#define KLAYGE_CONSTEXPR
#endif

#ifdef KLAYGE_CXX11_LIBRARY_CHRONO_SUPPORT
	#include <chrono>
#else
	#include <boost/chrono.hpp>
	namespace std
	{
		namespace chrono = boost::chrono;
	}
#endif

#ifdef KLAYGE_CXX11_LIBRARY_MEM_FN_SUPPORT
	#include <functional>
	namespace KlayGE
	{
		using std::mem_fn;
	}
#else
	#if defined(KLAYGE_PLATFORM_WIN32) && defined(KLAYGE_CPU_X86)
		#ifndef BOOST_MEM_FN_ENABLE_STDCALL
			#define BOOST_MEM_FN_ENABLE_STDCALL
		#endif
	#endif
	#include <boost/mem_fn.hpp>
	namespace KlayGE
	{
		using boost::mem_fn;
	}
#endif

#if !(((defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)) && (__GLIBCXX__ >= 20130531)) \
		|| defined(KLAYGE_PLATFORM_DARWIN) || defined(KLAYGE_PLATFORM_IOS) \
		|| defined(KLAYGE_COMPILER_MSVC))
#include <type_traits>
namespace std
{
	template <typename T>
	struct is_trivially_destructible
	{
		static const bool value = std::has_trivial_destructor<T>::value;
	};
}
#endif

#endif		// _KFL_CXX11_HPP
