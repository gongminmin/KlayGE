/**
 * @file ErrorHandling.cpp
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

#include <KFL/KFL.hpp>

#include <KFL/CXX20/format.hpp>

#include <ostream>
#include <string>
#include <string_view>
#include <system_error>

#include <KFL/ErrorHandling.hpp>

namespace KlayGE
{
	std::string CombineFileLine(std::string_view file, uint32_t line)
	{
		return std::format("{}: {}", std::move(file), line);
	}

	void Verify(bool x)
	{
		if (!x)
		{
			TERRC(std::errc::function_not_supported);
		}
	}

#if defined(KLAYGE_DEBUG) || !defined(KLAYGE_BUILTIN_UNREACHABLE)
	void KFLUnreachableInternal(std::string_view msg, std::string_view file, uint32_t line)
	{
		if (!msg.empty())
		{
			LogError() << msg << std::endl;
		}
		LogError() << "UNREACHABLE executed";
		if (!file.empty())
		{
			LogError() << " at " << file << ": " << line;
		}
		LogError() << "." << std::endl;

		TMSG("Unreachable.");
	}
#endif

}
