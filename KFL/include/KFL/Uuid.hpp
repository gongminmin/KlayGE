/**
 * @file Uuid.hpp
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

#ifndef KFL_UUID_HPP
#define KFL_UUID_HPP

#pragma once

#include <array>
#include <cstring>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <guiddef.h>
#endif

#include <KFL/Util.hpp>

namespace KlayGE
{
	struct Uuid
	{
		uint32_t data1;
		uint16_t data2;
		uint16_t data3;
		uint8_t data4[8];

		Uuid() noexcept = default;
		constexpr Uuid(uint32_t d1, uint16_t d2, uint16_t d3, std::array<uint8_t, 8> const& d4) noexcept
			: data1(d1), data2(d2), data3(d3), data4{d4[0], d4[1], d4[2], d4[3], d4[4], d4[5], d4[6], d4[7]}
		{
		}

#ifdef KLAYGE_PLATFORM_WINDOWS
		constexpr Uuid(GUID const& value) noexcept
			: Uuid(value.Data1, value.Data2, value.Data3,
				  {{value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6],
					  value.Data4[7]}})
		{
		}

		operator GUID const &() const noexcept
		{
			return reinterpret_cast<GUID const&>(*this);
		}
#endif
	};

	inline bool operator==(Uuid const& lhs, Uuid const& rhs) noexcept
	{
		return !std::memcmp(&lhs, &rhs, sizeof(Uuid));
	}

	inline bool operator!=(Uuid const& lhs, Uuid const& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <typename T>
	Uuid const& UuidOf();

	template <typename T>
	inline Uuid const& UuidOf([[maybe_unused]] T* p)
	{
		return UuidOf<T>();
	}

	template <typename T>
	inline Uuid const& UuidOf([[maybe_unused]] T const* p)
	{
		return UuidOf<T>();
	}

	template <typename T>
	inline Uuid const& UuidOf([[maybe_unused]] T& p)
	{
		return UuidOf<T>();
	}

	template <typename T>
	inline Uuid const& UuidOf([[maybe_unused]] T const& p)
	{
		return UuidOf<T>();
	}
} // namespace KlayGE

#define DEFINE_UUID_OF(x)                                      \
	template <>                                                \
	KlayGE::Uuid const& KlayGE::UuidOf<x>()                    \
	{                                                          \
		return reinterpret_cast<KlayGE::Uuid const&>(IID_##x); \
	}

#endif // KFL_UUID_HPP
