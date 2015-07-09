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
	// ���n bitΪ1
	inline uint32_t
	SetMask(uint32_t n)
		{ return 1UL << n; }
	template <uint32_t n>
	struct Mask
	{
		enum { value = 1UL << n };
	};

	// ȡ���еĵ� n bit
	inline uint32_t
	GetBit(uint32_t x, uint32_t n)
		{ return (x >> n) & 1; }
	// �����еĵ� n bitΪ1
	inline uint32_t
	SetBit(uint32_t x, uint32_t n)
		{ return x | SetMask(n); }

	// ȡ���ֽ�
	inline uint16_t
	LO_U8(uint16_t x)
		{ return x & 0xFF; }
	// ȡ���ֽ�
	inline uint16_t
	HI_U8(uint16_t x)
		{ return x >> 8; }

	// ȡ����
	inline uint32_t
	LO_U16(uint32_t x)
		{ return x & 0xFFFF; }
	// ȡ����
	inline uint32_t
	HI_U16(uint32_t x)
		{ return x >> 16; }

	// �ߵ��ֽڽ���
	inline uint16_t
	HI_LO_SwapU8(uint16_t x)
		{ return (LO_U8(x) << 8) | HI_U8(x); }
	// �ߵ��ֽ���
	inline uint32_t
	HI_LO_SwapU16(uint32_t x)
		{ return (LO_U16(x) << 16) | HI_U16(x); }

	// ���nλ����1������
	inline uint32_t
	MakeMask(uint32_t n)
		{ return (1UL << (n + 1)) - 1; }

	// ����FourCC����
	template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
	struct MakeFourCC
	{
		enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
	};

	// Unicode����, ����string, wstring֮���ת��
	std::string& Convert(std::string& strDest, std::string const & strSrc);
	std::string& Convert(std::string& strDest, std::wstring const & wstrSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::string const & strSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::wstring const & wstrSrc);

	// ��ͣ������
	void Sleep(uint32_t ms);

	// Endian��ת��
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
	inline std::shared_ptr<To>
	checked_pointer_cast(std::shared_ptr<From> const & p)
	{
		BOOST_ASSERT(std::dynamic_pointer_cast<To>(p) == std::static_pointer_cast<To>(p));
		return std::static_pointer_cast<To>(p);
	}

	uint32_t LastError();

	std::string ReadShortString(ResIdentifierPtr const & res);
	void WriteShortString(std::ostream& os, std::string const & str);

#ifdef KLAYGE_CXX11_CORE_VARIADIC_TEMPLATES
	namespace Detail
	{
#if defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION <= 44
		// GCC 4.4 supports an outdated version of rvalue references and creates a copy of the forwarded object.
		// This results in warnings 'returning reference to temporary'. Therefore we use a special version similar to std::forward.
		template <typename T>
		T&& Forward(T&& t) KLAYGE_NOEXCEPT
		{
			return t;
		}
#else
		template <typename T>
		T&& Forward(T& t) KLAYGE_NOEXCEPT
		{
			return static_cast<T&&>(t);
		}
#endif
	}

	template <typename T, typename... Args>
	inline std::shared_ptr<T> MakeSharedPtr(Args&& ... args)
	{
		return std::shared_ptr<T>(new T(Detail::Forward<Args>(args)...), std::default_delete<T>());
	}
#else
	template <typename T>
	inline std::shared_ptr<T> MakeSharedPtr()
	{
		return std::shared_ptr<T>(new T, std::default_delete<T>());
	}

	template <typename T, typename A1>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1)
	{
		return std::shared_ptr<T>(new T(a1), std::default_delete<T>());
	}

	template <typename T, typename A1>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1)
	{
		return std::shared_ptr<T>(new T(a1), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2)
	{
		return std::shared_ptr<T>(new T(a1, a2), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2)
	{
		return std::shared_ptr<T>(new T(a1, a2), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8, A9& a9)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9, typename A10>
	inline std::shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9, A10 const & a10)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), std::default_delete<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9, typename A10>
	inline std::shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8, A9& a9, A10& a10)
	{
		return std::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), std::default_delete<T>());
	}
#endif

	#define PRIME_NUM 0x9e3779b9

#ifdef KLAYGE_CXX11_CORE_CONSTEXPR_SUPPORT
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(disable: 4307) // The hash here could cause integral constant overflow
#endif

	constexpr size_t _Hash(char const * str, size_t seed)
	{
		return 0 == *str ? seed : _Hash(str + 1, seed ^ (*str + PRIME_NUM + (seed << 6) + (seed >> 2)));
	}

#ifdef KLAYGE_COMPILER_MSVC
	template <size_t N>
	struct EnsureConst
	{
		static const size_t value = N;
	};

	#define CT_HASH(x) (EnsureConst<_Hash(x, 0)>::value)
#else
	#define CT_HASH(x) (_Hash(x, 0))
#endif
#else
	#if defined(KLAYGE_COMPILER_MSVC)
		#define FORCEINLINE __forceinline
	#else
		#define FORCEINLINE inline
	#endif

	FORCEINLINE size_t _Hash(const char (&str)[1])
	{
		return *str + PRIME_NUM;
	}

	template <size_t N>
	FORCEINLINE size_t _Hash(const char (&str)[N])
	{
		typedef const char (&truncated_str)[N - 1];
		size_t seed = _Hash((truncated_str)str);
		return seed ^ (*(str + N - 1) + PRIME_NUM + (seed << 6) + (seed >> 2));
	}

	template <size_t N>
	FORCEINLINE size_t CT_HASH(const char (&str)[N])
	{
		typedef const char (&truncated_str)[N - 1];
		return _Hash((truncated_str)str);
	}

	#undef FORCEINLINE
#endif

	inline size_t RT_HASH(char const * str)
	{
		size_t seed = 0;
		while (*str != 0)
		{
			seed ^= (*str + PRIME_NUM + (seed << 6) + (seed >> 2));
			++ str;
		}
		return seed;
	}

#undef PRIME_NUM
}

#endif		// _KFL_UTIL_HPP
