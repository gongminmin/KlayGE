/**
 * @file ThrowErr.hpp
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

#ifndef _KFL_THROWERR_HPP
#define _KFL_THROWERR_HPP

#pragma once

#include <string>
#include <stdexcept>

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif

namespace KlayGE
{
	std::string CombineFileLine(std::string const & file, int line);
	void Verify(bool x);
}

#define THR(x)			{ throw std::system_error(std::make_error_code(x), KlayGE::CombineFileLine(__FILE__, __LINE__)); }

// Throw if failed
#define TIF(x)			{ HRESULT _hr = x; if (static_cast<HRESULT>(_hr) < 0) { throw std::runtime_error(KlayGE::CombineFileLine(__FILE__, __LINE__)); } }

#endif		// _KFL_THROWERR_HPP
