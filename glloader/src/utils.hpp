//
// glloader Copyright (C) 2004 Minmin Gong
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <string>
#include <vector>

typedef std::vector<std::string> funcs_names_t;
typedef std::vector<void**> entries_t;

namespace glloader
{
	void load_funcs(entries_t& entries, funcs_names_t const & names);

	void init_all();

	void gl_init();
	void wgl_init();
	void glx_init();
}

#endif			// _UTILS_HPP
