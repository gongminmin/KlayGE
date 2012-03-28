// Log.cpp
// KlayGE Cross-platform log utilities implement file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2012
// Homepage: http://www.klayge.org
//
// 4.1.0
// First release (2012.1.3)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>

#include <cstdarg>
#include <cstdio>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <android/log.h>
#else
#include <iostream>
#endif

#include <KlayGE/Log.hpp>

namespace KlayGE
{
	void LogInfo(char const * fmt, ...)
	{
		std::string const & app_name = Context::Instance().AppInstance().Name();

		va_list args;
		va_start(args, fmt); 

#ifdef KLAYGE_PLATFORM_ANDROID
		__android_log_vprint(ANDROID_LOG_INFO, app_name.c_str(), fmt, args);
#else
		char buffer[1024];
		vsprintf(buffer, fmt, args);

		std::clog << "(INFO)" << app_name << ": " << buffer << std::endl;
#endif

		va_end(args);
	}

	void LogWarn(char const * fmt, ...)
	{
		std::string const & app_name = Context::Instance().AppInstance().Name();

		va_list args;
		va_start(args, fmt); 

#ifdef KLAYGE_PLATFORM_ANDROID
		__android_log_vprint(ANDROID_LOG_WARN, app_name.c_str(), fmt, args);
#else
		char buffer[1024];
		vsprintf(buffer, fmt, args);

		std::clog << "(WARN)" << app_name << ": " << buffer << std::endl;
#endif

		va_end(args);
	}

	void LogError(char const * fmt, ...)
	{
		std::string const & app_name = Context::Instance().AppInstance().Name();

		va_list args;
		va_start(args, fmt); 

#ifdef KLAYGE_PLATFORM_ANDROID
		__android_log_vprint(ANDROID_LOG_ERROR, app_name.c_str(), fmt, args);
#else
		char buffer[1024];
		vsprintf(buffer, fmt, args);

		std::clog << "(ERROR)" << app_name << ": " << buffer << std::endl;
#endif

		va_end(args);
	}
}
