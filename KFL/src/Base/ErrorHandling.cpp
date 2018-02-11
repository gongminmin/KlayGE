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

#include <system_error>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore mpl_assertion_in_line_xxx
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KFL/ErrorHandling.hpp>

namespace KlayGE
{
	std::string CombineFileLine(std::string_view file, int line)
	{
		return std::string(file) + ": " + boost::lexical_cast<std::string>(line);
	}

	void Verify(bool x)
	{
		if (!x)
		{
			TERRC(std::errc::function_not_supported);
		}
	}

#if !defined(NDEBUG) || !defined(LLVM_BUILTIN_UNREACHABLE)
	void KFLUnreachableInternal(char const * msg, char const * file, uint32_t line)
	{
		if (msg)
		{
			LogError(msg);
		}
		if (file)
		{
			LogError("UNREACHABLE executed at %s: %d.", file, line);
		}
		else
		{
			LogError("UNREACHABLE executed.");
		}

		TMSG("Unreachable.");
	}
#endif

}
