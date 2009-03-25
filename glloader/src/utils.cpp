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

#ifdef GLLOADER_AGL
	#include <Carbon/Carbon.h>
#endif

#include <string>
#include <vector>

#include <algorithm>
#include <cassert>

#ifdef GLLOADER_DEBUG
#include <iostream>
#endif

#include "utils.h"

namespace
{
	// Split a string with white space to a vector<string>
	std::vector<std::string> split(std::string const & str)
	{
		std::vector<std::string> ret;
		for (std::string::const_iterator iter = str.begin(); iter != str.end(); ++ iter)
		{
			std::string::const_iterator start_iter = iter;
			while ((start_iter != str.end()) && (' ' == *start_iter))
			{
				++ start_iter;
				++ iter;
			}

			while ((iter != str.end()) && (*iter != ' '))
			{
				++ iter;
			}

			ret.push_back(std::string(start_iter, iter));
		}

		return ret;
	}
}

namespace glloader
{
	class gl_features_extractor
	{
	public:
		static gl_features_extractor& instance()
		{
			static gl_features_extractor inst;
			return inst;
		}

		bool is_supported(std::string const & name)
		{
			return std::binary_search(features_.begin(), features_.end(), name);
		}
		void promote(std::string const & low_name, std::string const & high_name)
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
		void promote(std::string const & high_name)
		{
			std::vector<std::string>::iterator iter = std::lower_bound(features_.begin(), features_.end(), high_name);
			if (*iter != high_name)
			{
#ifdef GLLOADER_DEBUG
				std::cerr << high_name << " is promoted" << std::endl;
#endif
				features_.insert(iter, high_name);
			}
		}

	private:
		gl_features_extractor()
		{
			gl_features();
			wgl_features();
			glx_features();

			std::sort(features_.begin(), features_.end());
		}

		void gl_version(int& major, int& minor)
		{
			GLubyte const * str = ::glGetString(GL_VERSION);
			if (str != NULL)
			{
				std::string const ver(reinterpret_cast<char const *>(str));
				std::string::size_type const pos(ver.find("."));

				major = ver[pos - 1] - '0';
				minor = ver[pos + 1] - '0';
			}
			else
			{
				// GL context has not actived yet
				major = minor = -1;
			}
		}
		void gl_features()
		{
			GLubyte const * str = ::glGetString(GL_EXTENSIONS);
			if (str != NULL)
			{
				std::vector<std::string> gl_exts = split(reinterpret_cast<char const *>(str));
				gl_exts.erase(std::remove(gl_exts.begin(), gl_exts.end(), ""), gl_exts.end());
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
				if (ver_code >= 21)
				{
					features_.push_back("GL_VERSION_2_1");
				}
				if (ver_code >= 30)
				{
					features_.push_back("GL_VERSION_3_0");
				}
			}
		}
		void wgl_features()
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
			wgl_exts.erase(std::remove(wgl_exts.begin(), wgl_exts.end(), ""), wgl_exts.end());
			features_.insert(features_.end(), wgl_exts.begin(), wgl_exts.end());
#endif		// GLLOADER_WGL
		}
		void glx_version(int& major, int& minor)
		{
#ifdef GLLOADER_GLX
			::glXQueryVersion(::glXGetCurrentDisplay(), &major, &minor);
#else
			major = minor = 0;
#endif		// GLLOADER_GLX
		}
		void glx_features()
		{
#ifdef GLLOADER_GLX
			::glXGetCurrentDisplay = (glXGetCurrentDisplayFUNC)(::glloader_get_gl_proc_address("glXGetCurrentDisplay"));

			char const * str = ::glXGetClientString(::glXGetCurrentDisplay(), GLX_EXTENSIONS);
			if (str != NULL)
			{
				std::vector<std::string> glx_exts = split(str);
				glx_exts.erase(std::remove(glx_exts.begin(), glx_exts.end(), ""), glx_exts.end());
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
			}
#endif		// GLLOADER_GLX
		}

	private:
		std::vector<std::string> features_;
	};
}

void promote_low_high(char const * low_name, char const * high_name)
{
	glloader::gl_features_extractor::instance().promote(low_name, high_name);
}

void promote_high(char const * high_name)
{
	glloader::gl_features_extractor::instance().promote(high_name);
}

void glloader_init()
{
	gl_init();

#ifdef GLLOADER_WGL
	wgl_init();
#endif

#ifdef GLLOADER_GLX
	glx_init();
#endif
}

void* glloader_get_gl_proc_address(const char* name)
{
	void* ret;

#ifdef GLLOADER_WGL
	ret = (void*)(::wglGetProcAddress(name));
#endif
#ifdef GLLOADER_AGL
	CFURLRef bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
		CFSTR("/System/Library/Frameworks/OpenGL.framework"), kCFURLPOSIXPathStyle, true);

	CFStringRef functionName = CFStringCreateWithCString(kCFAllocatorDefault, extname, kCFStringEncodingASCII);

	CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
	assert(bundle != NULL);

	ret = CFBundleGetFunctionPointerForName(bundle, functionName);

	CFRelease(bundleURL);
	CFRelease(functionName);
	CFRelease(bundle);
#endif
#ifdef GLLOADER_GLX
	ret = (void*)(glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name)));
#endif

#ifdef GLLOADER_DEBUG
	if (NULL == ret)
	{
		std::cerr << name << " is missing!" << std::endl;
	}
#endif

	return ret;
}

int glloader_is_supported(char const * name)
{
	assert(name != NULL);

	return glloader::gl_features_extractor::instance().is_supported(name);
}
