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

#define KFL_UNUSED(x) (void)(x)

#include <KFL/Log.hpp>

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
		KFL_UNUSED(x);
#endif
		return x;
	}
	template <typename T>
	T Native2LE(T x)
	{
#ifdef KLAYGE_LITTLE_ENDIAN
		KFL_UNUSED(x);
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

	template <typename T, typename... Args>
	inline std::shared_ptr<T> MakeSharedPtr(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtrHelper(std::false_type, Args&&... args)
	{
#ifdef KLAYGE_CXX14_LIBRARY_MAKE_UNIQUE
		return std::make_unique<T>(std::forward<Args>(args)...);
#else
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#endif
	}

	template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtrHelper(std::true_type, size_t size)
	{
		static_assert(0 == std::extent<T>::value,
			"make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

#ifdef KLAYGE_CXX14_LIBRARY_MAKE_UNIQUE
		return std::make_unique<T>(size);
#else
		typedef typename std::remove_extent<T>::type U;
		return std::unique_ptr<T>(new U[size]());
#endif
	}

	template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtr(Args&&... args)
	{
		return MakeUniquePtrHelper<T>(std::is_array<T>(), std::forward<Args>(args)...);
	}

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
