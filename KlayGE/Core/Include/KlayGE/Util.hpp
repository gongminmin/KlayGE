// Util.hpp
// KlayGE 实用函数库 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 增加了checked_pointer_cast (2006.8.17)
//
// 3.2.0
// 增加了copy_if (2006.2.23)
// 增加了EndianSwitch (2006.5.13)
//
// 3.0.0
// 增加了checked_cast (2005.9.26)
//
// 2.1.2
// 增加了本地和网络格式的转换函数 (2004.6.2)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_HPP
#define _UTIL_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <string>
#include <functional>

#include <boost/assert.hpp>
#include <boost/smart_ptr.hpp>

namespace KlayGE
{
	// 设第n bit为1
	inline uint32_t
	SetMask(uint32_t n)
		{ return 1UL << n; }
	template <uint32_t n>
	struct Mask
	{
		enum { value = 1UL << n };
	};

	// 取数中的第 n bit
	inline uint32_t
	GetBit(uint32_t x, uint32_t n)
		{ return (x >> n) & 1; }
	// 置数中的第 n bit为1
	inline uint32_t
	SetBit(uint32_t x, uint32_t n)
		{ return x & SetMask(n); }

	// 取低字节
	inline uint16_t
	LO_U8(uint16_t x)
		{ return x & 0xFF; }
	// 取高字节
	inline uint16_t
	HI_U8(uint16_t x)
		{ return (x & 0xFF) >> 8; }

	// 取低字
	inline uint32_t
	LO_U16(uint32_t x)
		{ return x & 0xFFFF; }
	// 取高字
	inline uint32_t
	HI_U16(uint32_t x)
		{ return (x & 0xFFFF) >> 16; }

	// 高低字节交换
	inline uint16_t
	HI_LO_SwapU8(uint16_t x)
		{ return (LO_U8(x) << 8) | HI_U8(x); }
	// 高低字交换
	inline uint32_t
	HI_LO_SwapU16(uint32_t x)
		{ return (LO_U16(x) << 16) | HI_U16(x); }

	// 获得某一位是1的掩码
	inline uint32_t
	MakeMask(uint32_t dw)
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
	void Sleep(uint32_t ms);

	// Endian的转换
	template <int size>
	void EndianSwitch(void* p);

	template <>
	void EndianSwitch<2>(void* p);
	template <>
	void EndianSwitch<4>(void* p);
	template <>
	void EndianSwitch<8>(void* p);

	template <int size>
	void NativeToBigEndian(void* p)
	{
	#ifdef KLAYGE_LITTLE_ENDIAN
		EndianSwitch<size>(p);
	#else
		p = p;
	#endif
	}
	template <int size>
	void NativeToLittleEndian(void* p)
	{
	#ifdef KLAYGE_LITTLE_ENDIAN
		p = p;
	#else
		EndianSwitch<size>(p);
	#endif
	}

	template <int size>
	void BigEndianToNative(void* p)
	{
		NativeToBigEndian<size>(p);
	}
	template <int size>
	void LittleEndianToNative(void* p)
	{
		NativeToLittleEndian<size>(p);
	}


	template <typename To, typename From>
	inline To
	checked_cast(From p)
	{
		BOOST_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}

	template <typename To, typename From>
	inline boost::shared_ptr<To>
	checked_pointer_cast(boost::shared_ptr<From> const & p)
	{
		BOOST_ASSERT(boost::dynamic_pointer_cast<To>(p) == boost::static_pointer_cast<To>(p));
		return boost::static_pointer_cast<To>(p);
	}

	uint32_t LastError();

#ifdef _SELECT1ST2ND_SUPPORT
	template <typename pair_type>
	struct select1st : public std::unary_function<pair_type const &, typename pair_type::first_type const &>
	{
		const typename pair_type::first_type& operator()(pair_type const & x) const
		{
			return x.first;
		}
	};

	template <typename pair_type>
	struct select2nd : public std::unary_function<pair_type const &, typename pair_type::second_type const &>
	{
		const typename pair_type::second_type& operator()(pair_type const & x) const
		{
			return x.second;
		}
	};
#else
	using std::select1st;
	using std::select2nd;
#endif		// _SELECT1ST2ND_SUPPORT

#ifdef _COPYIF_SUPPORT
	template<typename InputIterator, typename OutputIterator, typename Predicate>
	OutputIterator copy_if(InputIterator first, InputIterator last,
							OutputIterator dest_first,
							Predicate p)
	{
		for (InputIterator iter = first; iter != last; ++ iter)
		{
			if (p(*iter))
			{
				*dest_first = *iter;
				++ dest_first;
			}
		}

		return dest_first;
	}
#else
	using std::copyif;
#endif
}

#endif		// _UTIL_HPP
