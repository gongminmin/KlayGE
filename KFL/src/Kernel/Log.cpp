/**
 * @file Log.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#include <KFL/KFL.hpp>

#include <cstdarg>
#include <cstdio>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <android/log.h>
#else
#include <iostream>
#include <fstream>
#endif

#include <KFL/Log.hpp>

#ifdef KLAYGE_DEBUG
#ifndef KLAYGE_PLATFORM_ANDROID
namespace
{
	std::ofstream log_file("KlayGE.log");
}
#endif
#endif

namespace KlayGE
{
	void LogInfo(char const * fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

#ifdef KLAYGE_PLATFORM_ANDROID
		__android_log_vprint(ANDROID_LOG_INFO, "KlayGE", fmt, args);
#else
		std::array<char, 1024> buffer;
		vsprintf(&buffer[0], fmt, args);

		std::clog << "(INFO) KlayGE: " << &buffer[0] << std::endl;
#ifdef KLAYGE_DEBUG
		log_file << "(INFO) KlayGE: " << &buffer[0] << std::endl;
#endif
#endif

		va_end(args);
	}

	void LogWarn(char const * fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

#ifdef KLAYGE_PLATFORM_ANDROID
		__android_log_vprint(ANDROID_LOG_WARN, "KlayGE", fmt, args);
#else
		std::array<char, 1024> buffer;
		vsprintf(&buffer[0], fmt, args);

		std::clog << "(WARN) KlayGE: " << &buffer[0] << std::endl;
#ifdef KLAYGE_DEBUG
		log_file << "(WARN) KlayGE: " << &buffer[0] << std::endl;
#endif
#endif

		va_end(args);
	}

	void LogError(char const * fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

#ifdef KLAYGE_PLATFORM_ANDROID
		__android_log_vprint(ANDROID_LOG_ERROR, "KlayGE", fmt, args);
#else
		std::array<char, 1024> buffer;
		vsprintf(&buffer[0], fmt, args);

		std::clog << "(ERROR) KlayGE: " << &buffer[0] << std::endl;
#ifdef KLAYGE_DEBUG
		log_file << "(ERROR) KlayGE: " << &buffer[0] << std::endl;
#endif
#endif

		va_end(args);
	}
}
