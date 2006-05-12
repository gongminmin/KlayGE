// Util.cpp
// KlayGE 实用函数库 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 改用libc的函数实现了Convert (2004.11.24)
//
// 2.2.0
// 用Boost重写了Sleep (2004.10.25)
//
// 2.1.2
// 增加了本地和网络格式的转换函数 (2004.6.2)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#pragma warning(disable: 4251 4275)
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>

#ifdef WIN32
	#define NOMINMAX
	#include <windows.h>
#else
	#include <cerrno>
#endif

#include <cwchar>
#include <clocale>
#include <vector>
#include <algorithm>
#include <boost/assert.hpp>

#include <KlayGE/Util.hpp>

namespace KlayGE
{
	// 把一个wstring转化为string
	/////////////////////////////////////////////////////////////////////////////////
	std::string& Convert(std::string& dest, std::wstring const & src)
	{
		std::setlocale(LC_CTYPE, "");

		size_t const mbs_len = std::wcstombs(NULL, src.c_str(), 0);
		std::vector<char> tmp(mbs_len + 1);
		std::wcstombs(&tmp[0], src.c_str(), tmp.size());

		dest.assign(tmp.begin(), tmp.end() - 1);

		return dest;
	}

	// 把一个string转化为string
	/////////////////////////////////////////////////////////////////////////////////
	std::string& Convert(std::string& dest, std::string const & src)
	{
		dest = src;

		return dest;
	}

	// 把一个string转化为wstring
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring& Convert(std::wstring& dest, std::string const & src)
	{
		std::setlocale(LC_CTYPE, "");

		size_t const wcs_len = std::mbstowcs(NULL, src.c_str(), 0);
		std::vector<wchar_t> tmp(wcs_len + 1);
		std::mbstowcs(&tmp[0], src.c_str(), src.size());

		dest.assign(tmp.begin(), tmp.end() - 1);

		return dest;
	}

	// 把一个wstring转化为wstring
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring& Convert(std::wstring& dest, std::wstring const & src)
	{
		dest = src;

		return dest;
	}

	// 暂停几毫秒
	/////////////////////////////////////////////////////////////////////////////////
	void Sleep(uint32_t ms)
	{
		int const MILLISECONDS_PER_SECOND = 1000;
		int const NANOSECONDS_PER_MILLISECOND = 1000000;

		boost::xtime xt;

		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += ms / MILLISECONDS_PER_SECOND;
		boost::thread::sleep(xt);

		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.nsec += (ms % MILLISECONDS_PER_SECOND) * NANOSECONDS_PER_MILLISECOND;
		boost::thread::sleep(xt);
	}

	// Endian的切换
	/////////////////////////////////////////////////////////////////////////////////
	template <>
	void EndianSwitch<2>(void* p)
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[1]);
	}

	template <>
	void EndianSwitch<4>(void* p)
	{
		uint8_t* bytes = static_cast<uint8_t*>(p);
		std::swap(bytes[0], bytes[3]);
		std::swap(bytes[1], bytes[2]);
	}

	template <>
	void EndianSwitch<8>(void* p)
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
#ifdef WIN32
		return ::GetLastError();
#else
		return errno;
#endif
	}
}
