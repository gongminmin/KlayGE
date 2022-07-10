/**
 * @file DllLoader.hpp
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

#ifndef _KFL_DLLLOADER_HPP
#define _KFL_DLLLOADER_HPP

#pragma once

#include <string>

#if defined(KLAYGE_COMPILER_MSVC) || defined(KLAYGE_COMPILER_CLANGCL)
	#define DLL_PREFIX ""
#else
	#define DLL_PREFIX "lib"
#endif
#if defined(KLAYGE_PLATFORM_WINDOWS)
	#define DLL_EXT_NAME "dll"
#elif defined(KLAYGE_PLATFORM_DARWIN)
	#define DLL_EXT_NAME "dylib"
#else
	#define DLL_EXT_NAME "so"
#endif

#define DLL_SUFFIX KLAYGE_OUTPUT_SUFFIX "." DLL_EXT_NAME

namespace KlayGE
{
	class DllLoader final
	{
	public:
		DllLoader();
		~DllLoader();

		bool Load(std::string const & dll_name);
		void Free();

		void* GetProcAddress(std::string const & proc_name);

	private:
		void* dll_handle_;
	};
}

#endif		// _KFL_DLLLOADER_HPP
