// CommFuncs.cpp
// KlayGE 公用函数 实现文件
// Ver 1.4.8.4
// 版权所有(C) 龚敏敏, 2001--2003
// Homepage: http://www.enginedev.com
//
// 1.2.8.10
// 用string代替字符串指针 (2002.10.27)
//
// 1.2.8.9
// 修改了UNICODE函数返回类型 (2002.10.23)
//
// 1.4.8.4
// 增强的代码的安全性 (2003.3.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/select.h>
#endif

#include <cassert>
#include <vector>
#include <algorithm>

#include <KlayGE/CommFuncs.hpp>

namespace KlayGE
{
	// 把一个WString转化为String
	/////////////////////////////////////////////////////////////////////////////////
	String& Convert(String& dest, const WString& src)
	{
#ifdef WIN32
		std::vector<char> vecTemp(::WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1,
			NULL, 0, NULL, NULL) - 1);
		::WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &vecTemp[0],
			vecTemp.size(), NULL, NULL);

		dest.assign(vecTemp.begin(), vecTemp.end());

		return dest;
#endif	// WIN32
	}

	// 把一个String转化为String
	/////////////////////////////////////////////////////////////////////////////////
	String& Convert(String& dest, const String& src)
	{
		dest = src;

		return dest;
	}

	// 把一个String转化为WString
	/////////////////////////////////////////////////////////////////////////////////
	WString& Convert(WString& dest, const String& src)
	{
#ifdef WIN32
		std::vector<wchar_t> vecTemp(::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0) - 1);
		::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, &vecTemp[0],
			vecTemp.size());

		dest.assign(vecTemp.begin(), vecTemp.end());

		return dest;
#endif		// WIN32
	}

	// 把一个WString转化为WString
	/////////////////////////////////////////////////////////////////////////////////
	WString& Convert(WString& dest, const WString& src)
	{
		dest = src;

		return dest;
	}

	void Sleep(U32 ms)
	{
#ifdef WIN32
		::Sleep(ms);
#else
		timeval sleeper;

		sleeper.tv_sec = ms / 1000;
		sleeper.tv_usec = (ms % 1000) * 1000;

		select(0, NULL, NULL, NULL, &sleeper);
#endif		// WIN32
	}
}
