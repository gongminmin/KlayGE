// Util.hpp
// KlayGE 实用函数库 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 增加了本地和网络格式的转换函数 (2004.6.2)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <string>

#include <boost/smart_ptr.hpp>
#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

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

	// 产生FourCC常量
	template <char ch0, char ch1, char ch2, char ch3>
	struct MakeFourCC
	{
		enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
	};

	// Unicode函数, 用于string, wstring之间的转换
	std::string& Convert(std::string& strDest, std::string const & strSrc);
	std::string& Convert(std::string& strDest, std::wstring const & wstrSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::string const & strSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::wstring const & wstrSrc);

	// 暂停几毫秒
	void Sleep(U32 ms);

	// 网络格式和本地格式之间转换
	U32 ToNet(U32 host);
	U16 ToNet(U16 host);
	U32 ToHost(U32 network);
	U16 ToHost(U16 network);

	// 得到COM对象的智能指针
	template <typename T>
	inline boost::shared_ptr<T>
	MakeCOMPtr(T* p)
	{
		return boost::shared_ptr<T>(p, boost::mem_fn(&T::Release));
	}
}

#endif		// _UTIL_HPP
