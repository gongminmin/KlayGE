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
#include <KFL/CXX17.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KFL/CXX2a/endian.hpp>

#include <string>
#include <functional>

#include <boost/assert.hpp>

#define KFL_UNUSED(x) (void)(x)

// The opposite side of "explicit", for tracking purpose
#define KFL_IMPLICIT

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
	constexpr uint32_t SetMask(uint32_t n) noexcept
	{
		return 1UL << n;
	}
	template <uint32_t n>
	struct Mask
	{
		static uint32_t constexpr value = SetMask(n);
	};

	// 取数中的第 n bit
	constexpr uint32_t GetBit(uint32_t x, uint32_t n) noexcept
	{
		return (x >> n) & 1;
	}
	// 置数中的第 n bit为1
	constexpr uint32_t SetBit(uint32_t x, uint32_t n) noexcept
	{
		return x | SetMask(n);
	}

	// 取低字节
	constexpr uint16_t LO_U8(uint16_t x) noexcept
	{
		return x & 0xFF;
	}
	// 取高字节
	constexpr uint16_t HI_U8(uint16_t x) noexcept
	{
		return x >> 8;
	}

	// 取低字
	constexpr uint32_t LO_U16(uint32_t x) noexcept
	{
		return x & 0xFFFF;
	}
	// 取高字
	constexpr uint32_t HI_U16(uint32_t x) noexcept
	{
		return x >> 16;
	}

	// 高低字节交换
	constexpr uint16_t HI_LO_SwapU8(uint16_t x) noexcept
	{
		return (LO_U8(x) << 8) | HI_U8(x);
	}
	// 高低字交换
	constexpr uint32_t HI_LO_SwapU16(uint32_t x) noexcept
	{
		return (LO_U16(x) << 16) | HI_U16(x);
	}

	// 获得n位都是1的掩码
	constexpr uint32_t MakeMask(uint32_t n) noexcept
	{
		return (1UL << (n + 1)) - 1;
	}

	// 产生FourCC常量
	template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
	struct MakeFourCC
	{
		static uint32_t constexpr value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24);
	};

	// Unicode函数, 用于string, wstring之间的转换
	std::string& Convert(std::string& dest, std::string_view src);
	std::string& Convert(std::string& dest, std::wstring_view src);
	std::wstring& Convert(std::wstring& dest, std::string_view src);
	std::wstring& Convert(std::wstring& dest, std::wstring_view src);

	// 暂停几毫秒
	void Sleep(uint32_t ms);

	// Endian的转换
	template <int size>
	void EndianSwitch(void* p) noexcept;

	template <typename T>
	T Native2BE(T x) noexcept
	{
		KLAYGE_IF_CONSTEXPR (std::endian::native == std::endian::little)
		{
			EndianSwitch<sizeof(T)>(&x);
		}
		return x;
	}
	template <typename T>
	T Native2LE(T x) noexcept
	{
		KLAYGE_IF_CONSTEXPR (std::endian::native == std::endian::big)
		{
			EndianSwitch<sizeof(T)>(&x);
		}
		return x;
	}

	template <typename T>
	T BE2Native(T x) noexcept
	{
		return Native2BE(x);
	}
	template <typename T>
	T LE2Native(T x) noexcept
	{
		return Native2LE(x);
	}


	template <typename To, typename From>
	inline To checked_cast(From* p) noexcept
	{
		BOOST_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}
	
	template <typename To, typename From>
	inline To checked_cast(From const* p) noexcept
	{
		BOOST_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}

	template <typename To, typename From>
	inline typename std::add_rvalue_reference<To>::type checked_cast(From& p) noexcept
	{
		typedef typename std::remove_reference<To>::type RawToType;
		BOOST_ASSERT(dynamic_cast<RawToType*>(&p) == static_cast<RawToType*>(&p));
		return static_cast<RawToType&>(p);
	}

	template <typename To, typename From>
	inline typename std::add_rvalue_reference<To const>::type checked_cast(From const& p) noexcept
	{
		typedef typename std::remove_reference<To const>::type RawToType;
		BOOST_ASSERT(dynamic_cast<RawToType const*>(&p) == static_cast<RawToType const*>(&p));
		return static_cast<RawToType const&>(p);
	}

	template <typename To, typename From>
	inline std::shared_ptr<To>
	checked_pointer_cast(std::shared_ptr<From> const & p) noexcept
	{
		BOOST_ASSERT(dynamic_cast<To*>(p.get()) == static_cast<To*>(p.get()));
		return std::static_pointer_cast<To>(p);
	}

	uint32_t LastError();

	std::string ReadShortString(ResIdentifier& res);
	void WriteShortString(std::ostream& os, std::string_view str);

	template <typename T, typename... Args>
	inline std::shared_ptr<T> MakeSharedPtr(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtrHelper(std::false_type, Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtrHelper(std::true_type, size_t size)
	{
		static_assert(0 == std::extent<T>::value,
			"make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

		return std::make_unique<T>(size);
	}

	template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtr(Args&&... args)
	{
		return MakeUniquePtrHelper<T>(std::is_array<T>(), std::forward<Args>(args)...);
	}
}

#endif		// _KFL_UTIL_HPP
