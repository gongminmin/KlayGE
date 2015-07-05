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
#ifdef GLLOADER_EAGL
	#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID) || defined(__APPLE__) || defined(__APPLE_CC__)
#include <dlfcn.h>
#endif

#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "glloader", __VA_ARGS__))
#endif

#if (defined(__GNUC__) && !defined(__clang__)) || (defined(__clang__) && defined(__MINGW32__))
	#include <bits/c++config.h>
	#ifdef _GLIBCXX_USE_FLOAT128
		#undef _GLIBCXX_USE_FLOAT128
	#endif
	#ifdef _GLIBCXX_USE_INT128
		#undef _GLIBCXX_USE_INT128
	#endif
#endif

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include <sstream>

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

#ifdef GLLOADER_WGL
	typedef PROC (WINAPI *wglGetProcAddressFUNC)(LPCSTR lpszProc);
	wglGetProcAddressFUNC DynamicWglGetProcAddress;
#endif
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
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
		std::vector<std::map<std::string, std::string>> const & gl_dll_entries() const
		{
			return gl_dll_entries_;
		}
#endif

	private:
		gl_dll_container()
		{
			void* ogl_dll;
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#ifdef GLLOADER_GLES
			ogl_dll = ::LoadLibraryExA("libEGL.dll", NULL, 0);
			if (ogl_dll != NULL)
			{
				gl_dlls_.push_back(ogl_dll);
				this->DumpEntries("libEGL.dll");

				ogl_dll = ::LoadLibraryExA("libGLESv3.dll", NULL, 0);
				if (ogl_dll != NULL)
				{
					gl_dlls_.push_back(ogl_dll);
					this->DumpEntries("libGLESv3.dll");
				}
				else
				{
					ogl_dll = ::LoadLibraryExA("libGLESv2.dll", NULL, 0);
					if (ogl_dll != NULL)
					{
						gl_dlls_.push_back(ogl_dll);
						this->DumpEntries("libGLESv2.dll");
					}
				}
			}
			else
			{
				ogl_dll = ::LoadLibraryExA("libGLES20.dll", NULL, 0);
				if (ogl_dll != NULL)
				{
					gl_dlls_.push_back(ogl_dll);
					this->DumpEntries("libGLES20.dll");
				}
				else
				{
					ogl_dll = ::LoadLibraryExA("atioglxx.dll", NULL, 0);
					if (ogl_dll != NULL)
					{
						gl_dlls_.push_back(ogl_dll);
						this->DumpEntries("atioglxx.dll");
					}
				}
			}
#else
			ogl_dll = ::LoadLibraryExA("opengl32.dll", NULL, 0);
			if (ogl_dll != NULL)
			{
				gl_dlls_.push_back(ogl_dll);
				this->DumpEntries("opengl32.dll");
			}
#endif
#endif
#if defined(__APPLE__) || defined(__APPLE_CC__)
	#ifdef GLLOADER_GLES
		// http://forum.imgtec.com/discussion/comment/18323#Comment_18323
		// For PowerVR_SDK, we need to load libGLESv2 before libEGL
		ogl_dll = ::dlopen("libGLESv2.dylib", RTLD_LAZY);
		if (ogl_dll != NULL)
		{
			gl_dlls_.push_back(ogl_dll);
		}
		ogl_dll = ::dlopen("libGLESv3.dylib", RTLD_LAZY);
		if (ogl_dll != NULL)
		{
			gl_dlls_.push_back(ogl_dll);
		}
		ogl_dll = ::dlopen("libEGL.dylib", RTLD_LAZY);
		if (ogl_dll != NULL)
		{
			gl_dlls_.push_back(ogl_dll);
		}
	#endif
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
#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID) || defined(__APPLE__) || defined(__APPLE_CC__)
				::dlclose(gl_dlls_[i]);
#endif
			}
		}

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
		void DumpEntries(char const * name)
		{
			HMODULE dll = ::LoadLibraryExA(name, NULL, DONT_RESOLVE_DLL_REFERENCES);
			assert(IMAGE_DOS_SIGNATURE == reinterpret_cast<PIMAGE_DOS_HEADER>(dll)->e_magic);
			PIMAGE_NT_HEADERS header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<BYTE*>(dll) + reinterpret_cast<PIMAGE_DOS_HEADER>(dll)->e_lfanew);
			assert(IMAGE_NT_SIGNATURE == header->Signature);
			assert(header->OptionalHeader.NumberOfRvaAndSizes > 0);
			PIMAGE_EXPORT_DIRECTORY exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<BYTE*>(dll)
				+ header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			DWORD const * names = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(dll) + exports->AddressOfNames);
			std::vector<std::string> entries(exports->NumberOfNames);
			for (DWORD i = 0; i < exports->NumberOfNames; ++ i)
			{
				entries[i] = reinterpret_cast<char const *>(reinterpret_cast<BYTE const *>(dll) + names[i]);
			}
			::FreeLibrary(dll);

			bool decorated = false;
			for (size_t i = 0; i < entries.size(); ++ i)
			{
				if (('_' == entries[i][0]) || ('@' == entries[i][0]) || ('?' == entries[i][0]))
				{
					decorated = true;
					break;
				}
			}

			gl_dll_entries_.resize(gl_dll_entries_.size() + 1);
			if (decorated)
			{
				for (size_t i = 0; i < entries.size(); ++ i)
				{
					if (('_' == entries[i][0]) || ('@' == entries[i][0]) || ('?' == entries[i][0]))
					{
						std::string::size_type at_pos = entries[i].find("@", 1);
						if (at_pos != std::string::npos)
						{
							gl_dll_entries_.back().insert(std::make_pair(entries[i].substr(1, at_pos - 1), entries[i]));
						}
						else
						{
							gl_dll_entries_.back().insert(std::make_pair(entries[i].substr(1), entries[i]));
						}
					}
				}
			}
		}
#endif

	private:
		std::vector<void*> gl_dlls_;
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
		std::vector<std::map<std::string, std::string>> gl_dll_entries_;
#endif

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
#ifdef GLLOADER_WGL
			DynamicWglGetProcAddress = (wglGetProcAddressFUNC)(glloader_get_gl_proc_address("wglGetProcAddress"));
#endif

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
				if (ver.find("Fail") != std::string::npos)
				{
					// Something like <Failed to query OpenGL ES version>(Host : 3.3 NVIDIA-8.24.16 310.90.9.05f01)
					major = 2;
					minor = 0;
				}
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
				if (ver_code >= 45)
				{
					features_.push_back("GL_VERSION_4_5");
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
				if (ver_code >= 31)
				{
					features_.push_back("GLES_VERSION_3_1");
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
				typedef HDC (WINAPI *wglGetCurrentDCFUNC)();
				wglGetCurrentDCFUNC DynamicWglGetCurrentDC
					= (wglGetCurrentDCFUNC)(glloader_get_gl_proc_address("wglGetCurrentDC"));
				exts_str = wglGetExtensionsStringARB(DynamicWglGetCurrentDC());
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
			LOAD_FUNC1(glXQueryVersion);
			glXGetCurrentDisplayFUNC DynamicGlXGetCurrentDisplay 
				= (glXGetCurrentDisplayFUNC)(glloader_get_gl_proc_address("glXGetCurrentDisplay"));
			glXQueryVersion(DynamicGlXGetCurrentDisplay(), &major, &minor);
#else
			major = minor = 0;
#endif		// GLLOADER_GLX
		}
		void glx_features()
		{
#ifdef GLLOADER_GLX
			LOAD_FUNC1(glXGetClientString);
			glXGetCurrentDisplayFUNC DynamicGlXGetCurrentDisplay 
				= (glXGetCurrentDisplayFUNC)(glloader_get_gl_proc_address("glXGetCurrentDisplay"));
			char const * str = glXGetClientString(DynamicGlXGetCurrentDisplay(), GLX_EXTENSIONS);
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
	std::vector<std::map<std::string, std::string>> const & gl_dll_entries = glloader::gl_dll_container::instance().gl_dll_entries();
	for (size_t i = 0; (i < gl_dlls.size()) && (NULL == ret); ++ i)
	{
		ret = (void*)(::GetProcAddress(static_cast<HMODULE>(gl_dlls[i]), name));
		if (NULL == ret)
		{
			if (!gl_dll_entries[i].empty())
			{
				std::map<std::string, std::string>::const_iterator iter
					= gl_dll_entries[i].find(name);
				if (iter != gl_dll_entries[i].end())
				{
					ret = (void*)(::GetProcAddress(static_cast<HMODULE>(gl_dlls[i]), iter->second.c_str()));
				}
			}
		}
	}
#endif
#if defined(__unix__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) || defined(ANDROID) || defined(__APPLE__) || defined(__APPLE_CC__)
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

#if defined(GLLOADER_GLES) && !defined(GLLOADER_EAGL)
	ret = (void*)(eglGetProcAddress(name));
#endif
#ifdef GLLOADER_WGL
	ret = (void*)(DynamicWglGetProcAddress(name));
#endif
#if defined(GLLOADER_AGL) || defined(GLLOADER_EAGL)
#ifdef GLLOADER_AGL
	CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
#else
	CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengles"));
#endif
	assert(bundle != NULL);

	CFStringRef functionName = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);

	ret = CFBundleGetFunctionPointerForName(bundle, functionName);

	CFRelease(bundle);
	CFRelease(functionName);
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
