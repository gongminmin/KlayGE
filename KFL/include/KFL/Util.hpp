/**
 * @file Util.hpp
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

#ifndef _KFL_UTIL_HPP
#define _KFL_UTIL_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <string>
#include <functional>

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>

#define UNREF_PARAM(x) (void)(x)

#include <KFL/Log.hpp>

#define KFL_STRINGIZE(X) KFL_DO_STRINGIZE(X)
#define KFL_DO_STRINGIZE(X) #X

#define KFL_JOIN(X, Y) KFL_DO_JOIN(X, Y)
#define KFL_DO_JOIN(X, Y) KFL_DO_JOIN2(X, Y)
#define KFL_DO_JOIN2(X, Y) X##Y

#ifdef KLAYGE_DEBUG
#define KLAYGE_DBG_SUFFIX "_d"
#else
#define KLAYGE_DBG_SUFFIX ""
#endif

#define KLAYGE_OUTPUT_SUFFIX "_" KFL_STRINGIZE(KLAYGE_COMPILER_NAME) KFL_STRINGIZE(KLAYGE_COMPILER_VERSION) KLAYGE_DBG_SUFFIX

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
		{ return x >> 8; }

	// 取低字
	inline uint32_t
	LO_U16(uint32_t x)
		{ return x & 0xFFFF; }
	// 取高字
	inline uint32_t
	HI_U16(uint32_t x)
		{ return x >> 16; }

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
	std::string& Convert(std::string& strDest, std::string const & strSrc);
	std::string& Convert(std::string& strDest, std::wstring const & wstrSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::string const & strSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::wstring const & wstrSrc);

	// 暂停几毫秒
	void Sleep(uint32_t ms);

	// Endian的转换
	template <int size>
	void EndianSwitch(void* p);

	template <typename T>
	T Native2BE(T x)
	{
#ifdef KLAYGE_LITTLE_ENDIAN
		EndianSwitch<sizeof(T)>(&x);
#else
		UNREF_PARAM(x);
#endif
		return x;
	}
	template <typename T>
	T Native2LE(T x)
	{
#ifdef KLAYGE_LITTLE_ENDIAN
		UNREF_PARAM(x);
#else
		EndianSwitch<sizeof(T)>(&x);
#endif
		return x;
	}

	template <typename T>
	T BE2Native(T x)
	{
		return Native2BE(x);
	}
	template <typename T>
	T LE2Native(T x)
	{
		return Native2LE(x);
	}


	template <typename To, typename From>
	inline To
	checked_cast(From p)
	{
		BOOST_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}

	template <typename To, typename From>
	inline shared_ptr<To>
	checked_pointer_cast(shared_ptr<From> const & p)
	{
		BOOST_ASSERT(dynamic_pointer_cast<To>(p) == static_pointer_cast<To>(p));
		return static_pointer_cast<To>(p);
	}

	uint32_t LastError();

#ifdef KLAYGE_IDENTITY_SUPPORT
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
#endif		// KLAYGE_IDENTITY_SUPPORT

#ifdef KLAYGE_SELECT1ST2ND_SUPPORT
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
#endif		// KLAYGE_SELECT1ST2ND_SUPPORT

#ifdef KLAYGE_PROJECT1ST2ND_SUPPORT
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
#endif		// KLAYGE_PROJECT1ST2ND_SUPPORT

	std::string ReadShortString(ResIdentifierPtr const & res);
	void WriteShortString(std::ostream& os, std::string const & str);

	template <typename T>
	inline shared_ptr<T> MakeSharedPtr()
	{
		return shared_ptr<T>(new T, boost::checked_deleter<T>());
	}

	template <typename T, typename A1>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1)
	{
		return shared_ptr<T>(new T(a1), boost::checked_deleter<T>());
	}

	template <typename T, typename A1>
	inline shared_ptr<T> MakeSharedPtr(A1& a1)
	{
		return shared_ptr<T>(new T(a1), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2)
	{
		return shared_ptr<T>(new T(a1, a2), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2)
	{
		return shared_ptr<T>(new T(a1, a2), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3)
	{
		return shared_ptr<T>(new T(a1, a2, a3), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3)
	{
		return shared_ptr<T>(new T(a1, a2, a3), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8, A9& a9)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9, typename A10>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9, A10 const & a10)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9, typename A10>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8, A9& a9, A10& a10)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), boost::checked_deleter<T>());
	}

#ifdef KLAYGE_CXX11_CORE_CONSTEXPR_SUPPORT
	#define CONSTEXPR constexpr
#else
	#if defined(KLAYGE_COMPILER_MSVC)
		#define CONSTEXPR __forceinline
	#else
		#define CONSTEXPR inline
	#endif
#endif

	CONSTEXPR size_t _Hash(const char (&str)[1])
	{
		return *str + 0x9e3779b9;
	}

	template <size_t N>
	CONSTEXPR size_t _Hash(const char (&str)[N])
	{
		typedef const char (&truncated_str)[N - 1];
#ifdef KLAYGE_CXX11_CORE_CONSTEXPR_SUPPORT
		#define seed _Hash((truncated_str)str)
#else
		size_t seed = _Hash((truncated_str)str);
#endif
		return seed ^ (*(str + N - 1) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
#ifdef KLAYGE_CXX11_CORE_CONSTEXPR_SUPPORT
		#undef seed
#endif
	}

	template <size_t N>
	CONSTEXPR size_t CT_HASH(const char (&str)[N])
	{
		typedef const char (&truncated_str)[N - 1];
		return _Hash((truncated_str)str);
	}

#undef CONSTEXPR

	inline size_t RT_HASH(char const * str)
	{
		size_t seed = 0;
		while (*str != 0)
		{
			seed ^= (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2));
			++ str;
		}
		return seed;
	}
}

#endif		// _KFL_UTIL_HPP
