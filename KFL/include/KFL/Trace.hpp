/**
 * @file Trace.hpp
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

#ifndef _KFL_TRACE_HPP
#define _KFL_TRACE_HPP

#pragma once

#include <KFL/Log.hpp>

namespace KlayGE
{
	class Trace final
	{
	public:
		Trace(char const * func, int line = 0, char const * file = nullptr)
			: func_(func), line_(line), file_(file)
		{
#ifdef KLAYGE_DEBUG
			LogInfo() << "Enter " << func_ << " in file " << (file_ != nullptr ? file_ : "" ) << " (line " << line_ << ")" << std::endl;
#endif
		}

		~Trace()
		{
#ifdef KLAYGE_DEBUG
			LogInfo() << "Leave " << func_ << " in file " << (file_ != nullptr ? file_ : "") << " (line " << line_ << ")" << std::endl;
#endif
		}

	private:
		char const * func_;
		int line_;
		char const * file_;
	};
}

#endif		// _KFL_TRACE_HPP
