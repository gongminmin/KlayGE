// Util.cpp
// KlayGE 实用函数库 实现文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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

#pragma warning(disable : 4251)
#pragma warning(disable : 4275)

#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>

#ifdef WIN32
	#include <windows.h>
#endif

#include <cassert>
#include <vector>
#include <algorithm>

#include <KlayGE/Util.hpp>

namespace KlayGE
{
	// 把一个WString转化为String
	/////////////////////////////////////////////////////////////////////////////////
	std::string& Convert(std::string& dest, std::wstring const & src)
	{
#ifdef WIN32
		std::vector<char> vecTemp(::WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1,
			NULL, 0, NULL, NULL) - 1);
		::WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &vecTemp[0],
			static_cast<int>(vecTemp.size()), NULL, NULL);

		dest.assign(vecTemp.begin(), vecTemp.end());

		return dest;
#endif	// WIN32
	}

	// 把一个String转化为String
	/////////////////////////////////////////////////////////////////////////////////
	std::string& Convert(std::string& dest, std::string const & src)
	{
		dest = src;

		return dest;
	}

	// 把一个String转化为WString
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring& Convert(std::wstring& dest, std::string const & src)
	{
#ifdef WIN32
		std::vector<wchar_t> vecTemp(::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0) - 1);
		::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, &vecTemp[0],
			static_cast<int>(vecTemp.size()));

		dest.assign(vecTemp.begin(), vecTemp.end());

		return dest;
#endif		// WIN32
	}

	// 把一个WString转化为WString
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

	// uint32_t本地格式到网络格式
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t ToNet(uint32_t host)
	{
		union
		{
			uint8_t byte[sizeof(uint32_t) / sizeof(uint8_t)];
			uint32_t net;
		} ret;

		ret.byte[0] = static_cast<uint8_t>((host & 0xFF000000) >> 24);
		ret.byte[1] = static_cast<uint8_t>((host & 0x00FF0000) >> 16);
		ret.byte[2] = static_cast<uint8_t>((host & 0x0000FF00) >> 8);
		ret.byte[3] = static_cast<uint8_t>((host & 0x000000FF) >> 0);

		return ret.net;
	}

	// uint16_t本地格式到网络格式
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t ToNet(uint16_t host)
	{
		union
		{
			uint8_t byte[sizeof(uint16_t) / sizeof(uint8_t)];
			uint16_t net;
		} ret;

		ret.byte[0] = static_cast<uint8_t>((host & 0xFF00) >> 8);
		ret.byte[1] = static_cast<uint8_t>((host & 0x00FF) >> 0);

		return ret.net;
	}

	// uint32_t网络格式到本地格式
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t ToHost(uint32_t net)
	{
		return ToNet(net);
	}

	// uint16_t网络格式到本地格式
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t ToHost(uint16_t net)
	{
		return ToNet(net);
	}
}
