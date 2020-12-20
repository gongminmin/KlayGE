/**
 * @file format.hpp
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

#ifndef KFL_CXX20_FORMAT_HPP
#define KFL_CXX20_FORMAT_HPP

#pragma once

#include <KFL/Config.hpp>

#if defined(KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT)
	#include <format>
#else
	#include <fmt/format.h>
	namespace std
	{
		using fmt::format;
		using fmt::format_to;
		using fmt::format_to_n;
		using fmt::formatted_size;

		using fmt::vformat;
		using fmt::vformat_to;

		using fmt::basic_format_arg;

		using fmt::formatter;

		using fmt::basic_format_parse_context;
		using fmt::format_parse_context;
		using fmt::wformat_parse_context;

		using fmt::basic_format_context;
		using fmt::format_context;
		using fmt::wformat_context;

		using fmt::visit_format_arg;

		using fmt::make_format_args;

		using fmt::basic_format_args;
		using fmt::format_args;
		using fmt::wformat_args;
		using fmt::format_args_t;

		using fmt::format_error;
	}
#endif

#endif		// KFL_CXX20_FORMAT_HPP
