/**
 * @file string_view.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of Dilithium
 * For the latest info, see https://github.com/gongminmin/Dilithium
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

#ifndef _KFL_CXX17_STRING_VIEW_HPP
#define _KFL_CXX17_STRING_VIEW_HPP

#pragma once

#include <KFL/Config.hpp>

#if defined(KLAYGE_CXX17_LIBRARY_STRING_VIEW_SUPPORT)
	#include <string_view>
#else
	#include <nonstd/string_view.hpp>

	namespace std
	{
		using nonstd::basic_string_view;
		using nonstd::string_view;
		using nonstd::wstring_view;
	}
#endif

#endif		// _KFL_CXX17_STRING_VIEW_HPP
