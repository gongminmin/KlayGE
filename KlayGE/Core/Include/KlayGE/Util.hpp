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
#include <functional>

#include <boost/smart_ptr.hpp>
#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 设第n bit为1
	inline uint32
	SetMask(uint32 n)
		{ return 1UL << n; }
	template <uint32 n>
	struct Mask
	{
		enum { value = 1UL << n };
	};

	// 取数中的第 n bit
	inline uint32
	GetBit(uint32 x, uint32 n)
		{ return (x >> n) & 1; }
	// 置数中的第 n bit为1
	inline uint32
	SetBit(uint32 x, uint32 n)
		{ return x & SetMask(n); }

	// 取低字节
	inline uint16
	LO_U8(uint16 x)
		{ return x & 0xFF; }
	// 取高字节
	inline uint16
	HI_U8(uint16 x)
		{ return (x & 0xFF) >> 8; }

	// 取低字
	inline uint32
	LO_U16(uint32 x)
		{ return x & 0xFFFF; }
	// 取高字
	inline uint32
	HI_U16(uint32 x)
		{ return (x & 0xFFFF) >> 16; }

	// 高低字节交换
	inline uint16
	HI_LO_SwapU8(uint16 x)
		{ return (LO_U8(x) << 8) | HI_U8(x); }
	// 高低字交换
	inline uint32
	HI_LO_SwapU16(uint32 x)
		{ return (LO_U16(x) << 16) | HI_U16(x); }

	// 获得某一位是1的掩码
	inline uint32
	MakeMask(uint32 dw)
		{ return (1UL << (dw + 1)) - 1; }

	// 产生FourCC常量
	template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
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
	void Sleep(uint32 ms);

	// 网络格式和本地格式之间转换
	uint32 ToNet(uint32 host);
	uint16 ToNet(uint16 host);
	uint32 ToHost(uint32 network);
	uint16 ToHost(uint16 network);

	// 得到COM对象的智能指针
	template <typename T>
	inline boost::shared_ptr<T>
	MakeCOMPtr(T* p)
	{
		return boost::shared_ptr<T>(p, boost::mem_fn(&T::Release));
	}

#ifndef _SELECT1ST2ND_SUPPORT
	template <typename Pair>
	struct select1st : public std::unary_function<Pair, typename Pair::first_type>
	{
		const typename Pair::first_type& operator()(const Pair& x) const
		{
			return x.first;
		}
	};

	template <typename Pair>
	struct select2nd : public std::unary_function<Pair, typename Pair::second_type>
	{
		const typename Pair::second_type& operator()(const Pair& x) const
		{
			return x.second;
		}
	};
#endif		// _SELECT1ST2ND_SUPPORT
}

#endif		// _UTIL_HPP
