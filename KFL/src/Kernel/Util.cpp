/**
 * @file Util.cpp
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
#include <KFL/ResIdentifier.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
	#include <windows.h>
#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
	#include <KFL/Thread.hpp>
#endif
#else
	#include <KFL/Thread.hpp>
	#include <cerrno>
	#include <cstdlib>
	#include <cwchar>
	#include <clocale>
#endif

#include <vector>
#include <algorithm>
#include <boost/assert.hpp>

#include <KFL/Util.hpp>

namespace KlayGE
{
	// 把一个wstring转化为string
	/////////////////////////////////////////////////////////////////////////////////
	std::string& Convert(std::string& dest, std::wstring_view src)
	{
#if defined KLAYGE_PLATFORM_WINDOWS
		int const mbs_len = WideCharToMultiByte(CP_ACP, 0, src.data(), static_cast<int>(src.size()), nullptr, 0, nullptr, nullptr);
		std::vector<char> tmp(mbs_len + 1);
		WideCharToMultiByte(CP_ACP, 0, src.data(), static_cast<int>(src.size()), &tmp[0], mbs_len, nullptr, nullptr);
#elif defined KLAYGE_PLATFORM_ANDROID
		// Hack for wcstombs
		std::vector<char> tmp;
		for (auto ch : src)
		{
			if (ch < 0x80)
			{
				tmp.push_back(static_cast<char>(ch));
			}
			else
			{
				tmp.push_back(static_cast<char>((ch >> 0) & 0xFF));
				tmp.push_back(static_cast<char>((ch >> 8) & 0xFF));
			}
		}
		tmp.push_back('\0');
#else
		std::setlocale(LC_CTYPE, "");

		size_t const mbs_len = wcstombs(nullptr, src.data(), src.size());
		std::vector<char> tmp(mbs_len + 1);
		wcstombs(&tmp[0], src.data(), tmp.size());
#endif

		dest.assign(tmp.begin(), tmp.end() - 1);

		return dest;
	}

	// 把一个string转化为string
	/////////////////////////////////////////////////////////////////////////////////
	std::string& Convert(std::string& dest, std::string_view src)
	{
		dest = std::string(src);

		return dest;
	}

	// 把一个string转化为wstring
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring& Convert(std::wstring& dest, std::string_view src)
	{
#if defined KLAYGE_PLATFORM_WINDOWS
		int const wcs_len = MultiByteToWideChar(CP_ACP, 0, src.data(), static_cast<int>(src.size()), nullptr, 0);
		std::vector<wchar_t> tmp(wcs_len + 1);
		MultiByteToWideChar(CP_ACP, 0, src.data(), static_cast<int>(src.size()), &tmp[0], wcs_len);
#elif defined KLAYGE_PLATFORM_ANDROID
		// Hack for mbstowcs
		std::vector<wchar_t> tmp;
		for (auto iter = src.begin(); iter != src.end(); ++ iter)
		{
			unsigned char ch = *iter;
			wchar_t wch = ch;
			if (ch >= 0x80)
			{
				++ iter;
				if (iter != src.end())
				{
					wch |= (*iter) << 8;
				}
			}
			tmp.push_back(wch);
		}
		tmp.push_back(L'\0');
#else
		std::setlocale(LC_CTYPE, "");

		size_t const wcs_len = mbstowcs(nullptr, src.data(), src.size());
		std::vector<wchar_t> tmp(wcs_len + 1);
		mbstowcs(&tmp[0], src.data(), src.size());
#endif

		dest.assign(tmp.begin(), tmp.end() - 1);

		return dest;
	}

	// 把一个wstring转化为wstring
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring& Convert(std::wstring& dest, std::wstring_view src)
	{
		dest = std::wstring(src);

		return dest;
	}

	// 暂停几毫秒
	/////////////////////////////////////////////////////////////////////////////////
	void Sleep(uint32_t ms)
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::Sleep(ms);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
	}

	// Endian的切换
	/////////////////////////////////////////////////////////////////////////////////
	template <>
	void EndianSwitch<2>(void* p) noexcept
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[1]);
	}

	template <>
	void EndianSwitch<4>(void* p) noexcept
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[3]);
		std::swap(bytes[1], bytes[2]);
	}

	template <>
	void EndianSwitch<8>(void* p) noexcept
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[7]);
		std::swap(bytes[1], bytes[6]);
		std::swap(bytes[2], bytes[5]);
		std::swap(bytes[3], bytes[4]);
	}

	// 获取上一个错误代码
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t LastError()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		return ::GetLastError();
#else
		return errno;
#endif
	}

	std::string ReadShortString(ResIdentifierPtr const & res)
	{
		uint8_t len;
		res->read(&len, sizeof(len));

		std::string tmp;
		if (len > 0)
		{
			tmp.resize(len);
			res->read(&tmp[0], len * sizeof(tmp[0]));
		}

		return tmp;
	}

	void WriteShortString(std::ostream& os, std::string_view str)
	{
		uint8_t len = static_cast<uint8_t>(std::min(str.size(), static_cast<size_t>(255)));
		os.write(reinterpret_cast<char*>(&len), sizeof(len));

		if (len > 0)
		{
			os.write(str.data(), len * sizeof(str[0]));
		}
	}
}
