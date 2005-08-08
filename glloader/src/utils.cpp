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

#include <glloader/glloader.h>

#include <algorithm>

#include <boost/assert.hpp>
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
}

namespace glloader
{
	gl_features_extractor& gl_features_extractor::instance()
	{
		static gl_features_extractor inst;
		return inst;
	}

	bool gl_features_extractor::is_supported(std::string const & name)
	{
		return std::binary_search(features_.begin(), features_.end(), name);
	}

	void gl_features_extractor::promote(std::string const & low_name, std::string const & high_name)
	{
		if (low_name != high_name)
		{
			std::vector<std::string>::iterator iter = std::lower_bound(features_.begin(), features_.end(), low_name);
			if (*iter == low_name)
			{
				features_.erase(iter);

				iter = std::lower_bound(features_.begin(), features_.end(), high_name);
				if (*iter != high_name)
				{
					features_.insert(iter, high_name);
				}
			}
		}
	}

	void gl_features_extractor::promote(std::string const & high_name)
	{
		std::vector<std::string>::iterator iter = std::lower_bound(features_.begin(), features_.end(), high_name);
		if (*iter != high_name)
		{
			features_.insert(iter, high_name);
		}
	}

	gl_features_extractor::gl_features_extractor()
	{
		gl_features();
		wgl_features();
		glx_features();

		std::sort(features_.begin(), features_.end());
	}

	// Return the version of OpenGL in current system
	void gl_features_extractor::gl_version(int& major, int& minor)
	{
		GLubyte const * str = ::glGetString(GL_VERSION);
		BOOST_ASSERT(str != NULL);

		std::string const ver(reinterpret_cast<char const *>(str));
		std::string::size_type const pos(ver.find("."));

		major = ver[pos - 1] - '0';
		minor = ver[pos + 1] - '0';
	}

	void gl_features_extractor::gl_features()
	{
		GLubyte const * str = ::glGetString(GL_EXTENSIONS);
		BOOST_ASSERT(str != NULL);

		std::vector<std::string> gl_exts = split(reinterpret_cast<char const *>(str));
		features_.insert(features_.end(), gl_exts.begin(), gl_exts.end());

		int major, minor;
		gl_version(major, minor);

		int const ver_code = major * 10 + minor;
		if (ver_code >= 10)
		{
			features_.push_back("GL_VERSION_1_0");
		}
		if (ver_code >= 11)
		{
			features_.push_back("GL_VERSION_1_1");
		}
		if (ver_code >= 12)
		{
			features_.push_back("GL_VERSION_1_2");
		}
		if (ver_code >= 13)
		{
			features_.push_back("GL_VERSION_1_3");
		}
		if (ver_code >= 14)
		{
			features_.push_back("GL_VERSION_1_4");
		}
		if (ver_code >= 15)
		{
			features_.push_back("GL_VERSION_1_5");
		}
		if (ver_code >= 20)
		{
			features_.push_back("GL_VERSION_2_0");
		}
	}

	void gl_features_extractor::wgl_features()
	{
#ifdef GLLOADER_WGL
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

		std::vector<std::string> wgl_exts = split(exts_str);
		features_.insert(features_.end(), wgl_exts.begin(), wgl_exts.end());
#endif		// GLLOADER_WGL
	}

	// Return the version of GLX in current system	
	void gl_features_extractor::glx_version(int& major, int& minor)
	{
#ifdef GLLOADER_GLX
		::glXQueryVersion(::glXGetCurrentDisplay(), &major, &minor);
#endif		// GLLOADER_GLX
	}

	void gl_features_extractor::glx_features()
	{
#ifdef GLLOADER_GLX
		std::vector<std::string> glx_exts = split(::glXGetClientString(::glXGetCurrentDisplay(), GLX_EXTENSIONS));
		features_.insert(features_.end(), glx_exts.begin(), glx_exts.end());

		int major, minor;
		glx_version(major, minor);

		int const ver_code = major * 10 + minor;
		if (ver_code >= 10)
		{
			features_.push_back("GLX_VERSION_1_0");
		}
		if (ver_code >= 11)
		{
			features_.push_back("GLX_VERSION_1_1");
		}
		if (ver_code >= 12)
		{
			features_.push_back("GLX_VERSION_1_2");
		}
		if (ver_code >= 13)
		{
			features_.push_back("GLX_VERSION_1_3");
		}
		if (ver_code >= 14)
		{
			features_.push_back("GLX_VERSION_1_4");
		}
#endif		// GLLOADER_GLX
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
	BOOST_ASSERT(names.size() == entries.size());

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

int glloader_is_supported(char const * name)
{
	BOOST_ASSERT(name != NULL);

	return glloader::gl_features_extractor::instance().is_supported(name);
}
