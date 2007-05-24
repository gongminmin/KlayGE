// BSTR.cpp
// KlayGE 打包系统BSTR 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include "BSTR.hpp"

namespace KlayGE
{
	BSTR AllocBSTR(std::wstring const & sz)
	{
	#ifdef KLAYGE_PLATFORM_WINDOWS
		return ::SysAllocString(sz.c_str());
	#else
		if (sz.empty())
		{
			return NULL;
		}

		uint32_t strLen = sz.size();
		size_t len = (strLen + 1) * sizeof(sz[0]);
		uint8_t* p = new uint8_t[len + sizeof(uint32_t)];
		memcpy(p, &strLen, sizeof(strLen));
		BSTR bstr = reinterpret_cast<BSTR>(p + sizeof(strLen));
		memmove(bstr, sz.c_str(), len);
		return bstr;
	#endif
	}

	void FreeBSTR(BSTR bstr)
	{
	#ifdef KLAYGE_PLATFORM_WINDOWS
		::SysFreeString(bstr);
	#else
		if (bstr)
		{
			char* p = reinterpret_cast<char*>(bstr);
			p -= sizeof(uint32_t);
			delete[] p;
		}
	#endif
	}
}
