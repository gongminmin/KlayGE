// CommFuncs.hpp
// KlayGE 公用函数库 头文件
// Ver 1.2.8.10
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.enginedev.com
//
// 1.2.8.10
// 用string代替字符串指针 (2002.10.27)
//
// 1.2.8.9
// 修改了UNICODE函数返回类型 (2002.10.23)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _COMMFUNCS_HPP
#define _COMMFUNCS_HPP

#include <string>
#include <KlayGE/KlayGE.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 设第n bit为1
	inline U32
	SetMask(U32 n)
		{ return 1UL << n; }
	template <U32 n>
	struct Mask
	{
		enum { value = 1UL << n };
	};

	// 取数中的第 n bit
	inline U32
	GetBit(U32 x, U32 n)
		{ return (x >> n) & 1; }
	// 置数中的第 n bit为1
	inline U32
	SetBit(U32 x, U32 n)
		{ return x & SetMask(n); }

	// 取低字节
	inline U16
	LO_U8(U16 x)
		{ return x & 0xFF; }
	// 取高字节
	inline U16
	HI_U8(U16 x)
		{ return (x & 0xFF) >> 8; }

	// 取低字
	inline U32
	LO_U16(U32 x)
		{ return x & 0xFFFF; }
	// 取高字
	inline U32
	HI_U16(U32 x)
		{ return (x & 0xFFFF) >> 16; }

	// 高低字节交换
	inline U16
	HI_LO_SwapU8(U16 x)
		{ return (LO_U8(x) << 8) | HI_U8(x); }
	// 高低字交换
	inline U32
	HI_LO_SwapU16(U32 x)
		{ return (LO_U16(x) << 16) | HI_U16(x); }

	// 获得某一位是1的掩码
	inline U32
	MakeMask(U32 dw)
		{ return (1UL << (dw + 1)) - 1; }

	// UNICODE函数, 用于String, WString之间的转化
	/////////////////////////////////////////////////////////////////////////////////
	String& Convert(String& strDest, const String& strSrc);
	String& Convert(String& strDest, const WString& wstrSrc);
	WString& Convert(WString& wstrDest, const String& strSrc);
	WString& Convert(WString& wstrDest, const WString& wstrSrc);

	// 暂停几毫秒
	/////////////////////////////////////////////////////////////////////////////////
	void Sleep(U32 ms);
}

#endif		// _COMMFUNC_HPP