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
#include <KFL/CXX2a/span.hpp>
#include <KFL/CustomizedStreamBuf.hpp>

#include <cstdarg>
#include <cstdio>
#include <iostream>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <android/log.h>
#include <cstring>
#else
#include <fstream>
#endif

#include <KFL/Log.hpp>

namespace
{
	using namespace KlayGE;

#ifdef KLAYGE_PLATFORM_ANDROID
	class AndroidLogStreamCallback : boost::noncopyable
	{
	public:
		explicit AndroidLogStreamCallback(int prio)
			: prio_(prio)
		{
		}
		AndroidLogStreamCallback(AndroidLogStreamCallback&& rhs)
			: prio_(rhs.prio_)
		{
		}

		std::streambuf::int_type operator()(void const * buff, std::streamsize count)
		{
			std::vector<char> tmp(count + 1);
			std::memcpy(tmp.data(), buff, count);
			tmp.back() = 0;
			__android_log_write(prio_, "KlayGE", tmp.data());
			return static_cast<std::streambuf::int_type>(count);
		}

	private:
		int prio_;
	};

	template <int PRIO>
	std::ostream& AndroidLog()
	{
		static CallbackOutputStreamBuf<AndroidLogStreamCallback> log_stream_buff((AndroidLogStreamCallback(PRIO)));
		static std::ostream log_stream(&log_stream_buff);
		return log_stream;
	}
#else
	class MultiOStreamsCallback : boost::noncopyable
	{
	public:
		explicit MultiOStreamsCallback(std::span<std::ostream*> oss)
			: oss_(oss)
		{
		}
		MultiOStreamsCallback(MultiOStreamsCallback&& rhs)
			: oss_(std::move(rhs.oss_))
		{
		}

		std::streambuf::int_type operator()(void const * buff, std::streamsize count)
		{
			for (auto& os : oss_)
			{
				os->write(static_cast<char const *>(buff), count);
			}
			return static_cast<std::streambuf::int_type>(count);
		}

	private:
		std::span<std::ostream*> oss_;
	};

	std::ostream& Log()
	{
#ifdef KLAYGE_DEBUG
		static std::ofstream log_file("KlayGE.log");
#endif

		static std::ostream* oss[] =
		{
#ifdef KLAYGE_DEBUG
			&log_file,
#endif
			&std::clog
		};
		static CallbackOutputStreamBuf<MultiOStreamsCallback> log_stream_buff((MultiOStreamsCallback(oss)));
		static std::ostream log_stream(&log_stream_buff);
		return log_stream;
	}
#endif

#ifndef KLAYGE_DEBUG
	class EmptyOStreamsCallback : boost::noncopyable
	{
	public:
		EmptyOStreamsCallback()
		{
		}
		EmptyOStreamsCallback(EmptyOStreamsCallback&& rhs)
		{
			KFL_UNUSED(rhs);
		}

		std::streambuf::int_type operator()(void const * buff, std::streamsize count)
		{
			KFL_UNUSED(buff);
			return static_cast<std::streambuf::int_type>(count);
		}
	};

	std::ostream& EmptyLog()
	{
		static CallbackOutputStreamBuf<EmptyOStreamsCallback> empty_stream_buff((EmptyOStreamsCallback()));
		static std::ostream empty_stream(&empty_stream_buff);
		return empty_stream;
	}
#endif
}

namespace KlayGE
{
	std::ostream& LogDebug()
	{
#ifdef KLAYGE_DEBUG
#ifdef KLAYGE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_DEBUG>();
#else
		return Log() << "(DEBUG) KlayGE: ";
#endif
#else
		return EmptyLog();
#endif
	}

	std::ostream& LogInfo()
	{
#ifdef KLAYGE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_INFO>();
#else
		return Log() << "(INFO) KlayGE: ";
#endif
	}

	std::ostream& LogWarn()
	{
#ifdef KLAYGE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_WARN>();
#else
		return Log() << "(WARN) KlayGE: ";
#endif
	}

	std::ostream& LogError()
	{
#ifdef KLAYGE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_ERROR>();
#else
		return Log() << "(ERROR) KlayGE: ";
#endif
	}
}
