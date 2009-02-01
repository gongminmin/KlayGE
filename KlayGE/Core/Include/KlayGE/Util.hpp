// Util.hpp
// KlayGE 实用函数库 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 增加了MakeSharedPtr (2008.10.17)
//
// 3.6.0
// 增加了identity, project1st和project2nd (2007.4.15)
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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <string>
#include <functional>

#include <boost/assert.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

#define UNREF_PARAM(x) (x)

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
		{ return x | SetMask(n); }

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

	// 获得n位都是1的掩码
	inline uint32_t
	MakeMask(uint32_t n)
		{ return (1UL << (n + 1)) - 1; }

	// 产生FourCC常量
	template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
	struct MakeFourCC
	{
		enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
	};

	// Unicode函数, 用于string, wstring之间的转换
	KLAYGE_CORE_API std::string& Convert(std::string& strDest, std::string const & strSrc);
	KLAYGE_CORE_API std::string& Convert(std::string& strDest, std::wstring const & wstrSrc);
	KLAYGE_CORE_API std::wstring& Convert(std::wstring& wstrDest, std::string const & strSrc);
	KLAYGE_CORE_API std::wstring& Convert(std::wstring& wstrDest, std::wstring const & wstrSrc);

	// 暂停几毫秒
	KLAYGE_CORE_API void Sleep(uint32_t ms);

	// Endian的转换
	template <int size>
	void EndianSwitch(void* p);

	template <>
	KLAYGE_CORE_API void EndianSwitch<2>(void* p);
	template <>
	KLAYGE_CORE_API void EndianSwitch<4>(void* p);
	template <>
	KLAYGE_CORE_API void EndianSwitch<8>(void* p);

	template <int size>
	void NativeToBigEndian(void* p)
	{
	#ifdef KLAYGE_LITTLE_ENDIAN
		EndianSwitch<size>(p);
	#else
		UNREF_PARAM(p);
	#endif
	}
	template <int size>
	void NativeToLittleEndian(void* p)
	{
	#ifdef KLAYGE_LITTLE_ENDIAN
		UNREF_PARAM(p);
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

	KLAYGE_CORE_API uint32_t LastError();

#ifdef _IDENTITY_SUPPORT
	template <typename arg_type>
	struct identity : public std::unary_function<arg_type, arg_type>
	{
		arg_type const & operator()(arg_type const & x) const
		{
			return x;
		}
	};
#else
	using std::identity;
#endif		// _IDENTITY_SUPPORT

#ifdef _SELECT1ST2ND_SUPPORT
	template <typename pair_type>
	struct select1st : public std::unary_function<pair_type, typename pair_type::first_type>
	{
		typename pair_type::first_type const & operator()(pair_type const & x) const
		{
			return x.first;
		}
	};

	template <typename pair_type>
	struct select2nd : public std::unary_function<pair_type, typename pair_type::second_type>
	{
		typename pair_type::second_type const & operator()(pair_type const & x) const
		{
			return x.second;
		}
	};
#else
	using std::select1st;
	using std::select2nd;
#endif		// _SELECT1ST2ND_SUPPORT

#ifdef _PROJECT1ST2ND_SUPPORT
	template <typename arg1_type, typename arg2_type>
	struct project1st : public std::binary_function<arg1_type, arg2_type, arg1_type>
	{
		arg1_type operator()(arg1_type const & x, arg2_type const & /*y*/) const
		{
			return x;
		}
	};

	template <typename arg1_type, typename arg2_type>
	struct project2nd : public std::binary_function<arg1_type, arg2_type, arg2_type>
	{
		arg2_type operator()(arg1_type const & /*x*/, arg2_type const & y) const
		{
			return y;
		}
	};
#else
	using std::project1st;
	using std::project2nd;
#endif		// _PROJECT1ST2ND_SUPPORT

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

	template <typename T>
	inline boost::shared_ptr<T> MakeSharedPtr()
	{
		return boost::shared_ptr<T>(new T, boost::checked_deleter<T>());
	}

	template <typename T, typename A1>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1)
	{
		return boost::shared_ptr<T>(new T(a1), boost::checked_deleter<T>());
	}

	template <typename T, typename A1>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1)
	{
		return boost::shared_ptr<T>(new T(a1), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2)
	{
		return boost::shared_ptr<T>(new T(a1, a2), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2)
	{
		return boost::shared_ptr<T>(new T(a1, a2), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	inline boost::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	inline boost::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10)
	{
		return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), boost::checked_deleter<T>());
	}
}

#endif		// _UTIL_HPP
