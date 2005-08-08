// glloader
// Copyright (C) 2004-2005 Minmin Gong
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <string>
#include <vector>

typedef std::vector<std::string> funcs_names_t;
typedef std::vector<void**> entries_t;

namespace glloader
{
	class gl_features_extractor
	{
	public:
		static gl_features_extractor& instance();

		bool is_supported(std::string const & name);
		void promote(std::string const & low_name, std::string const & high_name);
		void promote(std::string const & high_name);

	private:
		gl_features_extractor();

		void gl_version(int& major, int& minor);
		void gl_features();
		void wgl_features();
		void glx_version(int& major, int& minor);
		void glx_features();

	private:
		std::vector<std::string> features_;
	};

	void load_funcs(entries_t& entries, funcs_names_t const & names);

	void gl_init();
	void wgl_init();
	void glx_init();
}

#endif			// _UTILS_HPP
