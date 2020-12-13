/**
 * @file ErrorHandling.hpp
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

#ifndef _KFL_ERRORHANDLING_HPP
#define _KFL_ERRORHANDLING_HPP

#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

namespace KlayGE
{
	std::string CombineFileLine(std::string_view file, int line);
	void Verify(bool x);

#if defined(KLAYGE_DEBUG) || !defined(KLAYGE_BUILTIN_UNREACHABLE)
	KLAYGE_ATTRIBUTE_NORETURN void KFLUnreachableInternal(char const * msg = nullptr, char const * file = nullptr, uint32_t line = 0);
#endif
}

// Throw error code
#define TEC(x)			{ throw std::system_error(x, KlayGE::CombineFileLine(__FILE__, __LINE__)); }

// Throw error message
#define TMSG(msg)		{ throw std::runtime_error(msg); }

// Throw if failed (error code)
#define TIFEC(x)		{ if (x) TEC(x) }

// Throw if failed (errc)
#define TERRC(x)		TEC(std::make_error_code(x))

// Throw if failed (errc)
#define TIFERRC(x)		TIFEC(std::make_error_code(x))

// Throw if failed (HRESULT)
#define TIFHR(x)		{ if ((x) < 0) { TMSG(KlayGE::CombineFileLine(__FILE__, __LINE__)); } }

#ifdef KLAYGE_DEBUG
	#define KFL_UNREACHABLE(msg) KlayGE::KFLUnreachableInternal(msg, __FILE__, __LINE__)
#elif defined(KLAYGE_BUILTIN_UNREACHABLE)
	#define KFL_UNREACHABLE(msg) KLAYGE_BUILTIN_UNREACHABLE
#else
	#define KFL_UNREACHABLE(msg) KlayGE::KFLUnreachableInternal()
#endif

#endif		// _KFL_ERRORHANDLING_HPP
