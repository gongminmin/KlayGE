/**
 * @file Config.hpp
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

#ifndef KFL_CONFIG_HPP
#define KFL_CONFIG_HPP

#if !defined(__cplusplus)
	#error C++ compiler required.
#endif

#if defined(DEBUG) | defined(_DEBUG)
	#define KLAYGE_DEBUG
#endif

#define KFL_STRINGIZE(X) KFL_DO_STRINGIZE(X)
#define KFL_DO_STRINGIZE(X) #X

#define KFL_JOIN(X, Y) KFL_DO_JOIN(X, Y)
#define KFL_DO_JOIN(X, Y) KFL_DO_JOIN2(X, Y)
#define KFL_DO_JOIN2(X, Y) X##Y

#include <KFL/Compiler.hpp>
#include <KFL/Platform.hpp>
#include <KFL/Architecture.hpp>

#if defined(KLAYGE_COMPILER_MSVC) || defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
	#define KLAYGE_HAS_STRUCT_PACK
#endif

// Forces all boost's libraries to be linked as dll
#ifndef BOOST_ALL_DYN_LINK
	#define BOOST_ALL_DYN_LINK
#endif
// Skips boost's #pragma lib
#ifndef BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif

// Deprecated features in Boost.System are excluded.
#ifndef BOOST_SYSTEM_NO_DEPRECATED
	#define BOOST_SYSTEM_NO_DEPRECATED
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS) && ((defined(KLAYGE_COMPILER_GCC) && defined(KLAYGE_CPU_X64)) || defined(KLAYGE_COMPILER_CLANG))
	#ifndef BOOST_USE_WINDOWS_H
		#define BOOST_USE_WINDOWS_H
	#endif
#endif

// Prevent Boost to link the Boost.DateTime
#ifndef BOOST_DATE_TIME_NO_LIB
	#define BOOST_DATE_TIME_NO_LIB
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP) || defined(KLAYGE_PLATFORM_LINUX) || defined(KLAYGE_PLATFORM_DARWIN)
	#define KLAYGE_IS_DEV_PLATFORM 1
#else
	#define KLAYGE_IS_DEV_PLATFORM 0
#endif

#endif		// KFL_CONFIG_HPP
