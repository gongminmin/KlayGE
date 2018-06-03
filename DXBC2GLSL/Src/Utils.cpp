/**
 * @file Utils.cpp
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

#include <DXBC2GLSL/Utils.hpp>
#include <KFL/CustomizedStreamBuf.hpp>
#include <exception>
#include <sstream>
#include <limits>
#include <cmath>

namespace
{
	class bad_assert : public std::exception
	{
	public:
		bad_assert(char const * expr, char const * msg, char const * function, char const * file, long line)
		{
			KlayGE::StringOutputStreamBuf what_buff(what_);
			std::ostream ss(&what_buff);
			ss << expr;
			if (msg)
			{
				ss << ' ' << msg;
			}
			ss << " in " << function << ", line " << line << " of " << file;
		}

		char const * what() const noexcept
		{
			return what_.c_str();
		}

	private:
		std::string what_;
	};
}

namespace boost
{
	void assertion_failed(char const * expr, char const * function, char const * file, long line)
	{
		throw bad_assert(expr, nullptr, function, file, line);
	}

	void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
	{
		throw bad_assert(expr, msg, function, file, line);
	}
}

bool ValidFloat(float f)
{
	union FNUI
	{
		float f;
		uint32_t ui;
	} fnui;
	fnui.f = f;
	return (0x80000000 == fnui.ui) || (!std::isnan(f)
		&& ((f >= std::numeric_limits<float>::min())
			|| (-f >= std::numeric_limits<float>::min()))
		&& ((f <= std::numeric_limits<float>::max())
			|| (-f <= std::numeric_limits<float>::max())));
}
