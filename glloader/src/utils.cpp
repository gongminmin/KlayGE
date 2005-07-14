// glloader
// Copyright (C) 2004 Minmin Gong
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

#include <glloader/glloader.h>

#include <cassert>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>

#ifdef GLLOADER_DEBUG
#include <iostream>
#endif

#include "utils.hpp"

namespace
{
	// Split a string with white space to a vector<string>
	std::vector<std::string> split(std::string const & str)
	{
		std::vector<std::string> ret;
		boost::algorithm::split(ret, str, boost::bind(std::equal_to<char>(), ' ', _1));

		return ret;
	}

	// Return the version of OpenGL in current system
	void gl_version(int& major, int& minor)
	{
		GLubyte const * str = ::glGetString(GL_VERSION);

		std::string const ver(reinterpret_cast<char const *>(str));
		std::string::size_type const pos(ver.find("."));

		major = ver[pos - 1] - '0';
		minor = ver[pos + 1] - '0';
	}

	std::vector<std::string> gl_features()
	{
		GLubyte const * str = ::glGetString(GL_EXTENSIONS);

		std::vector<std::string> ret = split(reinterpret_cast<char const *>(str));

		int major, minor;
		gl_version(major, minor);

		int const ver_code = major * 10 + minor;
		if (ver_code >= 10)
		{
			ret.push_back("GL_VERSION_1_0");
		}
		if (ver_code >= 11)
		{
			ret.push_back("GL_VERSION_1_1");
		}
		if (ver_code >= 12)
		{
			ret.push_back("GL_VERSION_1_2");
		}
		if (ver_code >= 13)
		{
			ret.push_back("GL_VERSION_1_3");
		}
		if (ver_code >= 14)
		{
			ret.push_back("GL_VERSION_1_4");
		}
		if (ver_code >= 15)
		{
			ret.push_back("GL_VERSION_1_5");
		}
		if (ver_code >= 20)
		{
			ret.push_back("GL_VERSION_2_0");
		}

		return ret;
	}

#ifdef GLLOADER_WGL
	std::vector<std::string> wgl_features()
	{
		std::string exts_str;

		::wglGetExtensionsStringARB = (wglGetExtensionsStringARBFUNC)(::glloader_get_gl_proc_address("wglGetExtensionsStringARB"));
		if (::wglGetExtensionsStringARB != NULL)
		{
			exts_str = ::wglGetExtensionsStringARB(wglGetCurrentDC());
		}
		else
		{
			::wglGetExtensionsStringEXT = (wglGetExtensionsStringEXTFUNC)(::glloader_get_gl_proc_address("wglGetExtensionsStringEXT"));
			if (::wglGetExtensionsStringEXT != NULL)
			{
				exts_str = ::wglGetExtensionsStringEXT();
			}
		}

		return split(exts_str);
	}
#endif		// GLLOADER_WGL

#ifdef GLLOADER_GLX
	// Return the version of GLX in current system	
	int glx_version()
	{
		int major, minor;
		::glXQueryVersion(::glXGetCurrentDisplay(), &major, &minor);
		return majoy * 10 + minor;
	}

	std::vector<std::string> glx_features()
	{
		std::vector<std::string> ret = Split(::glXGetClientString(::glXGetCurrentDisplay(), GLX_EXTENSIONS));

		int const ver_code = glx_version();
		if (ver_code >= 10)
		{
			ret.push_back("GLX_VERSION_1_0");
		}
		if (ver_code >= 11)
		{
			ret.push_back("GLX_VERSION_1_1");
		}
		if (ver_code >= 12)
		{
			ret.push_back("GLX_VERSION_1_2");
		}
		if (ver_code >= 13)
		{
			ret.push_back("GLX_VERSION_1_3");
		}
		if (ver_code >= 14)
		{
			ret.push_back("GLX_VERSION_1_4");
		}

		return ret;
	}
#endif		// GLLOADER_GLX

	std::vector<std::string> const & all_features()
	{
		static std::vector<std::string> features;

		if (features.empty())
		{
			std::vector<std::string> gl = gl_features();
			features.insert(features.end(), gl.begin(), gl.end());

	#ifdef GLLOADER_WGL
			std::vector<std::string> wgl = wgl_features();
			features.insert(features.end(), wgl.begin(), wgl.end());
	#endif

	#ifdef GLLOADER_GLX
			std::vector<std::string> glx = glx_features();
			features.insert(features.end(), glx.begin(), glx.end());
	#endif

			std::sort(features.begin(), features.end());
		}

		return features;
	}
}

void glloader_init()
{
	glloader::gl_init();

#ifdef GLLOADER_WGL
	glloader::wgl_init();
#endif

#ifdef GLLOADER_GLX
	glloader::glx_init();
#endif
}

void* glloader_get_gl_proc_address(const char* name)
{
#ifdef ISWIN32
	return (void*)(::wglGetProcAddress(name));
#else
	return (void*)(::glXGetProcAddress(reinterpret_cast<const GLubyte*>(name)));
#endif
}

// Load an OpenGL extension given by 'names'.
// The functions entries are putted in 'entries'.
void glloader::load_funcs(entries_t& entries, funcs_names_t const & names)
{
	assert(names.size() == entries.size());

	for (size_t i = 0; i < entries.size(); ++ i)
	{
		*entries[i] = ::glloader_get_gl_proc_address(names[i].c_str());

#ifdef GLLOADER_DEBUG
		if (NULL == *entries[i])
		{
			std::cerr << names[i] << " is missing!" << std::endl;
		}
#endif
	}
}

int glloader_is_supported(const char* name)
{
	std::vector<std::string> const & features = all_features();
	return std::binary_search(features.begin(), features.end(), std::string(name));
}
