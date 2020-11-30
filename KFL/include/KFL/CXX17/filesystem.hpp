/**
 * @file filesystem.hpp
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

#ifndef KFL_CXX17_FILESYSTEM_HPP
#define KFL_CXX17_FILESYSTEM_HPP

#pragma once

#include <KFL/Config.hpp>

#if defined(KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT)
	#if defined(KLAYGE_COMPILER_CLANG) && (defined(KLAYGE_PLATFORM_DARWIN) || defined(KLAYGE_PLATFORM_IOS))
		#if (MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14)
			// libcxx on macOS 10.14 has filesystem, but unuseable. Need to fallback to ghc's, and not redefine it to std.
			#include <ghc/filesystem.hpp>
			#define FILESYSTEM_NS ghc::filesystem
		#else
			#include <filesystem>
			#define FILESYSTEM_NS std::filesystem
		#endif
	#else
		#include <filesystem>
		#define FILESYSTEM_NS std::filesystem
	#endif
#elif defined(KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT)
	#include <experimental/filesystem>
	namespace std
	{
		namespace filesystem = experimental::filesystem;
	}
	#define FILESYSTEM_NS std::filesystem
#else
	#include <ghc/filesystem.hpp>
	namespace std
	{
		namespace filesystem = ghc::filesystem;
	}
	#define FILESYSTEM_NS std::filesystem
#endif

#endif		// KFL_CXX17_FILESYSTEM_HPP
