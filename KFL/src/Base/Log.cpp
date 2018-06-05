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
#include <KFL/ArrayRef.hpp>
#include <KFL/CustomizedStreamBuf.hpp>

#include <cstdarg>
#include <cstdio>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <android/log.h>
#else
#include <iostream>
#include <fstream>
#endif

#include <KFL/Log.hpp>

namespace
{
	using namespace KlayGE;

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

	class MultiOStreamsCallback : boost::noncopyable
	{
	public:
		explicit MultiOStreamsCallback(ArrayRef<std::ostream*> oss)
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
		ArrayRef<std::ostream*> oss_;
	};

	std::ostream& Log()
	{
#ifdef KLAYGE_DEBUG
#ifndef KLAYGE_PLATFORM_ANDROID
		static std::ofstream log_file("KlayGE.log");
#endif
#endif

		static std::ostream* oss[] =
		{
#ifdef KLAYGE_DEBUG
#ifndef KLAYGE_PLATFORM_ANDROID
			&log_file,
#endif
#endif
			&std::clog
		};
		static CallbackOutputStreamBuf<MultiOStreamsCallback> log_stream_buff((MultiOStreamsCallback(oss)));
		static std::ostream log_stream(&log_stream_buff);
		return log_stream;
	}

	std::ostream& EmptyLog()
	{
		static CallbackOutputStreamBuf<EmptyOStreamsCallback> empty_stream_buff((EmptyOStreamsCallback()));
		static std::ostream empty_stream(&empty_stream_buff);
		return empty_stream;
	}
}

namespace KlayGE
{
	std::ostream& LogDebug()
	{
#ifdef KLAYGE_DEBUG
		return Log();
#else
		return EmptyLog();
#endif
	}

	std::ostream& LogInfo()
	{
		return Log() << "(INFO) KlayGE: ";
	}

	std::ostream& LogWarn()
	{
		return Log() << "(WARN) KlayGE: ";
	}

	std::ostream& LogError()
	{
		return Log() << "(ERROR) KlayGE: ";
	}
}
