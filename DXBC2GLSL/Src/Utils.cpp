/**************************************************************************
 *
 * Copyright 2013 Minmin Gong
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <DXBC2GLSL/Utils.hpp>
#include <exception>
#include <sstream>
#include <limits>

namespace boost
{
	void assertion_failed(char const * expr, char const * function, char const * file, long line)
	{
		std::stringstream ss;
		ss << expr << " in " << function << ", line " << line << " of " << file;
		throw std::exception(ss.str().c_str());
	}

	void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
	{
		std::stringstream ss;
		ss << expr << ' ' << msg << " in " << function << ", line " << line << " of " << file;
		throw std::exception(ss.str().c_str());
	}
}

bool ValidFloat(float f)
{
	return (f == f)
		&& ((f >= std::numeric_limits<float>::min())
			|| (-f >= std::numeric_limits<float>::min()))
		&& ((f <= std::numeric_limits<float>::max())
			|| (-f <= std::numeric_limits<float>::max()));
}
