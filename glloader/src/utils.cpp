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

#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID)
#ifdef GLLOADER_GLES
	#include <dlfcn.h>
#endif
#endif

#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "glloader", __VA_ARGS__))
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
		std::string::const_iterator iter = str.begin();
		std::string::const_iterator start_iter = iter;

		std::vector<std::string> ret;
		while (start_iter != str.end())
		{
			start_iter = iter;
			while ((start_iter != str.end()) && (' ' == *start_iter))
			{
				++ start_iter;
			}

			iter = start_iter;
			while ((iter != str.end()) && (*iter != ' '))
			{
				++ iter;
			}

			if (start_iter != iter)
			{
				ret.push_back(std::string(start_iter, iter));
			}
		}

		return ret;
	}
}

namespace glloader
{
	class gl_dll_container
	{
	public:
		static gl_dll_container& instance()
		{
			if (NULL == inst_)
			{
				inst_ = new gl_dll_container;
			}
			return *inst_;
		}

		static void delete_instance()
		{
			delete inst_;
			inst_ = NULL;
		}

		std::vector<void*> const & gl_dlls() const
		{
			return gl_dlls_;
		}

	private:
		gl_dll_container()
		{
			void* ogl_dll;
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#ifdef GLLOADER_GLES
			ogl_dll = ::LoadLibraryA("libEGL.dll");
			if (ogl_dll != NULL)
			{
				gl_dlls_.push_back(ogl_dll);
				ogl_dll = ::LoadLibraryA("libGLESv3.dll");
				if (ogl_dll != NULL)
				{
					gl_dlls_.push_back(ogl_dll);
				}
				else
				{
					ogl_dll = ::LoadLibraryA("libGLESv2.dll");
					if (ogl_dll != NULL)
					{
						gl_dlls_.push_back(ogl_dll);
					}
				}
			}
			else
			{
				ogl_dll = ::LoadLibraryA("libGLES20.dll");
				if (ogl_dll != NULL)
				{
					gl_dlls_.push_back(ogl_dll);
				}
				else
				{
					ogl_dll = ::LoadLibraryA("atioglxx.dll");
					if (ogl_dll != NULL)
					{
						gl_dlls_.push_back(ogl_dll);
					}
				}
			}
#else
			ogl_dll = ::LoadLibraryA("opengl32.dll");
			if (ogl_dll != NULL)
			{
				gl_dlls_.push_back(ogl_dll);
			}
#endif
#endif
#if defined(__APPLE__) || defined(__APPLE_CC__)
			// TODO
#endif
#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID)
#ifdef GLLOADER_GLES
			ogl_dll = ::dlopen("libEGL.so", RTLD_LAZY);
			if (ogl_dll != NULL)
			{
				gl_dlls_.push_back(ogl_dll);
				ogl_dll = ::dlopen("libGLESv3.so", RTLD_LAZY);
				if (ogl_dll != NULL)
				{
					gl_dlls_.push_back(ogl_dll);
				}
				else
				{
					ogl_dll = ::dlopen("libGLESv2.so", RTLD_LAZY);
					if (ogl_dll != NULL)
					{
						gl_dlls_.push_back(ogl_dll);
					}
				}
			}
#else
			ogl_dll = ::dlopen("libGL.so", RTLD_LAZY);
			if (ogl_dll != NULL)
			{
				gl_dlls_.push_back(ogl_dll);
			}
#endif
#endif
		}

		~gl_dll_container()
		{
			for (size_t i = 0; i < gl_dlls_.size(); ++ i)
			{
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
				::FreeLibrary(static_cast<HMODULE>(gl_dlls_[i]));
#endif
#if defined(__APPLE__) || defined(__APPLE_CC__)
				// TODO
#endif
#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID)
				::dlclose(gl_dlls_[i]);
#endif
			}
		}

	private:
		std::vector<void*> gl_dlls_;

		static gl_dll_container* inst_;
	};

	class gl_features_extractor
	{
	public:
		static gl_features_extractor& instance()
		{
			if (NULL == inst_)
			{
				inst_ = new gl_features_extractor;
			}
			return *inst_;
		}

		static void delete_instance()
		{
			delete inst_;
			inst_ = NULL;
		}

		bool is_supported(std::string const & name)
		{
			return std::binary_search(features_.begin(), features_.end(), name);
		}

		int num_features()
		{
			return static_cast<int>(features_.size());
		}

		char const * feature_name(int index)
		{
			return features_[index].c_str();
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
			egl_features();

			std::sort(features_.begin(), features_.end());
			features_.erase(std::unique(features_.begin(), features_.end()), features_.end());
		}

		void gl_version(int& major, int& minor)
		{
			GLubyte const * str = glGetString(GL_VERSION);
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
			int major, minor;
			gl_version(major, minor);

			if (major > 0)
			{
				std::vector<std::string> gl_exts;
				if (major >= 3)
				{
					LOAD_FUNC1(glGetStringi);
					GLint num_exts;
					glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
					gl_exts.resize(num_exts);
					for (GLint i = 0; i < num_exts; ++ i)
					{
						gl_exts[i] = reinterpret_cast<char const *>(glGetStringi(GL_EXTENSIONS, i));
					}
				}
				else
				{
					GLubyte const * str = glGetString(GL_EXTENSIONS);
					if (str != NULL)
					{
						gl_exts = split(reinterpret_cast<char const *>(str));
					}
				}

				gl_exts.erase(std::remove(gl_exts.begin(), gl_exts.end(), ""), gl_exts.end());
#ifdef GLLOADER_GLES
				for (std::vector<std::string>::iterator iter = gl_exts.begin(); iter != gl_exts.end(); ++ iter)
				{
					if (0 == iter->find("GL_"))
					{
						*iter = "GLES_" + iter->substr(3);
					}
				}
#endif
				features_.insert(features_.end(), gl_exts.begin(), gl_exts.end());

				int const ver_code = major * 10 + minor;
#ifndef GLLOADER_GLES
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
				if (ver_code >= 31)
				{
					features_.push_back("GL_VERSION_3_1");
				}
				if (ver_code >= 32)
				{
					features_.push_back("GL_VERSION_3_2");
				}
				if (ver_code >= 33)
				{
					features_.push_back("GL_VERSION_3_3");
				}
				if (ver_code >= 40)
				{
					features_.push_back("GL_VERSION_4_0");
				}
				if (ver_code >= 41)
				{
					features_.push_back("GL_VERSION_4_1");
				}
				if (ver_code >= 42)
				{
					features_.push_back("GL_VERSION_4_2");
				}
				if (ver_code >= 43)
				{
					features_.push_back("GL_VERSION_4_3");
				}
				if (ver_code >= 44)
				{
					features_.push_back("GL_VERSION_4_4");
				}
#else
				if (ver_code >= 10)
				{
					features_.push_back("GLES_VERSION_1_0");
				}
				if (ver_code >= 11)
				{
					features_.push_back("GLES_VERSION_1_1");
				}
				if (ver_code >= 20)
				{
					features_.push_back("GLES_VERSION_2_0");
				}
				if (ver_code >= 30)
				{
					features_.push_back("GLES_VERSION_3_0");
				}
#endif
			}
		}
		void wgl_features()
		{
#ifdef GLLOADER_WGL
			std::string exts_str;

			LOAD_FUNC1(wglGetExtensionsStringARB);
			if (wglGetExtensionsStringARB != NULL)
			{
				exts_str = wglGetExtensionsStringARB(::wglGetCurrentDC());
			}
			else
			{
				LOAD_FUNC1(wglGetExtensionsStringEXT);
				if (wglGetExtensionsStringEXT != NULL)
				{
					exts_str = wglGetExtensionsStringEXT();
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
			glXQueryVersion(glXGetCurrentDisplay(), &major, &minor);
#else
			major = minor = 0;
#endif		// GLLOADER_GLX
		}
		void glx_features()
		{
#ifdef GLLOADER_GLX
			char const * str = glXGetClientString(glXGetCurrentDisplay(), GLX_EXTENSIONS);
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
		void egl_version(int& major, int& minor)
		{
#ifdef GLLOADER_EGL
			char const * str = eglQueryString(eglGetCurrentDisplay(), EGL_VERSION);
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
#else
			major = minor = 0;
#endif		// GLLOADER_EGL
		}
		void egl_features()
		{
#ifdef GLLOADER_EGL
			char const * str = eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
			if (str != NULL)
			{
				std::vector<std::string> egl_exts = split(str);
				egl_exts.erase(std::remove(egl_exts.begin(), egl_exts.end(), ""), egl_exts.end());
				features_.insert(features_.end(), egl_exts.begin(), egl_exts.end());

				int major, minor;
				egl_version(major, minor);

				int const ver_code = major * 10 + minor;
				if (ver_code >= 10)
				{
					features_.push_back("EGL_VERSION_1_0");
				}
				if (ver_code >= 11)
				{
					features_.push_back("EGL_VERSION_1_1");
				}
				if (ver_code >= 12)
				{
					features_.push_back("EGL_VERSION_1_2");
				}
				if (ver_code >= 13)
				{
					features_.push_back("EGL_VERSION_1_3");
				}
				if (ver_code >= 14)
				{
					features_.push_back("EGL_VERSION_1_4");
				}
			}
#endif		// GLLOADER_EGL
		}

	private:
		std::vector<std::string> features_;

		static gl_features_extractor* inst_;
	};

	gl_dll_container* gl_dll_container::inst_ = NULL;
	gl_features_extractor* gl_features_extractor::inst_ = NULL;
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
	glloader::gl_features_extractor::delete_instance();

#ifdef GLLOADER_GL
	gl_init();
#endif

#ifdef GLLOADER_WGL
	wgl_init();
#endif

#ifdef GLLOADER_GLX
	glx_init();
#endif

#ifdef GLLOADER_GLES
	gles_init();
#endif

#ifdef GLLOADER_EGL
	egl_init();
#endif
}

void* get_gl_proc_address_by_dll(const char* name)
{
	void* ret = NULL;

	std::vector<void*> const & gl_dlls = glloader::gl_dll_container::instance().gl_dlls();

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
	for (size_t i = 0; (i < gl_dlls.size()) && (NULL == ret); ++ i)
	{
		ret = (void*)(::GetProcAddress(static_cast<HMODULE>(gl_dlls[i]), name));
	}
#endif
#if defined(__APPLE__) || defined(__APPLE_CC__)
	// TODO
#endif
#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID)
	for (size_t i = 0; (i < gl_dlls.size()) && (NULL == ret); ++ i)
	{
		ret = ::dlsym(gl_dlls[i], name);
	}
#endif

	return ret;
}

void* get_gl_proc_address_by_api(const char* name)
{
	void* ret = NULL;

#ifdef GLLOADER_GLES
	ret = (void*)(eglGetProcAddress(name));
#endif
#ifdef GLLOADER_WGL
	ret = (void*)(::wglGetProcAddress(name));
#endif
#ifdef GLLOADER_AGL
	CFURLRef bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
		CFSTR("/System/Library/Frameworks/OpenGL.framework"), kCFURLPOSIXPathStyle, true);

	CFStringRef functionName = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);

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

	return ret;
}

void* glloader_get_gl_proc_address(const char* name)
{
	void* ret = get_gl_proc_address_by_dll(name);
	if (NULL == ret)
	{
		ret = get_gl_proc_address_by_api(name);
	}

#ifdef GLLOADER_DEBUG
	if (NULL == ret)
	{
#if defined(__ANDROID__) || defined(ANDROID)
		LOGW("%s is missing!\n", name);
#else
		std::cerr << name << " is missing!" << std::endl;
#endif
	}
#endif

	return ret;
}

int glloader_is_supported(char const * name)
{
	assert(name != NULL);

	return glloader::gl_features_extractor::instance().is_supported(name);
}

int glloader_num_features()
{
	return glloader::gl_features_extractor::instance().num_features();
}

char const * glloader_get_feature_name(int index)
{
	if ((index >= 0) && (index < glloader_num_features()))
	{
		return glloader::gl_features_extractor::instance().feature_name(index);
	}
	else
	{
		return NULL;
	}
}
