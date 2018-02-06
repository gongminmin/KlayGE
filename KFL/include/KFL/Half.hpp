/**
 * @file Half.hpp
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

#ifndef _KFL_HALF_HPP
#define _KFL_HALF_HPP

#pragma once

#include <boost/operators.hpp>
#include <limits>

#define HALF_MIN	5.96046448e-08f	// Smallest positive half

#define HALF_NRM_MIN	6.10351562e-05f	// Smallest positive normalized half

#define HALF_MAX	65504.0f	// Largest positive half

#define HALF_EPSILON	0.00097656f	// Smallest positive e for which
					// half (1.0 + e) != half (1.0)

#define HALF_MANT_DIG	11		// Number of digits in mantissa
					// (significand + hidden leading 1)

#define HALF_DIG	2		// Number of base 10 digits that
					// can be represented without change

#define HALF_RADIX	2		// Base of the exponent

#define HALF_MIN_EXP	-13		// Minimum negative integer such that
					// HALF_RADIX raised to the power of
					// one less than that integer is a
					// normalized half

#define HALF_MAX_EXP	16		// Maximum positive integer such that
					// HALF_RADIX raised to the power of
					// one less than that integer is a
					// normalized half

#define HALF_MIN_10_EXP	-4		// Minimum positive integer such
					// that 10 raised to that power is
					// a normalized half

#define HALF_MAX_10_EXP	4		// Maximum positive integer such
					// that 10 raised to that power is
					// a normalized half

namespace KlayGE
{
	// 1s5e10m
	class half final : boost::addable<half,
						boost::subtractable<half,
						boost::multipliable<half,
						boost::dividable<half,
						boost::equality_comparable<half>>>>>
	{
	public:
		constexpr half() noexcept
			: value_()
		{
		}

		explicit half(float f) noexcept;

		operator float() const noexcept;

		// 特殊值

		// returns +infinity
		static half pos_inf() noexcept;

		// returns -infinity
		static half neg_inf() noexcept;

		// returns a NAN with the bit pattern 0111111111111111
		static half q_nan() noexcept;

		// returns a NAN with the bit pattern 0111110111111111
		static half s_nan() noexcept;


		// 赋值操作符
		half const & operator+=(half const & rhs) noexcept;
		half const & operator-=(half const & rhs) noexcept;
		half const & operator*=(half const & rhs) noexcept;
		half const & operator/=(half const & rhs) noexcept;

		half& operator=(half const & rhs) noexcept;

		// 一元操作符
		half const operator+() const noexcept;
		half const operator-() const noexcept;

		bool operator==(half const & rhs) noexcept;

	private:
		uint16_t value_;
	};
}

namespace std
{
	template<>
	class numeric_limits<KlayGE::half>
	{
	public:
		static bool const is_specialized = true;
		static int const digits = HALF_MANT_DIG;
		static int const digits10 = HALF_DIG;
		static bool const is_signed = true;
		static bool const is_integer = false;
		static bool const is_exact = false;
		static int const radix = HALF_RADIX;
		static int const min_exponent = HALF_MIN_EXP;
		static int const min_exponent10 = HALF_MIN_10_EXP;
		static int const max_exponent = HALF_MAX_EXP;
		static int const max_exponent10 = HALF_MAX_10_EXP;
		static bool const has_infinity = true;
		static bool const has_quiet_NaN = true;
		static bool const has_signaling_NaN = true;
		static float_denorm_style const has_denorm = denorm_present;
		static bool const has_denorm_loss = false;
		static bool const is_iec559 = false;
		static bool const is_bounded = false;
		static bool const is_modulo = false;
		static bool const traps = true;
		static bool const tinyness_before = false;
		static float_round_style const round_style = round_to_nearest;

		static KlayGE::half min() noexcept
		{
			return KlayGE::half(HALF_NRM_MIN);
		}
		static KlayGE::half max() noexcept
		{
			return KlayGE::half(HALF_MAX);
		}
		static KlayGE::half epsilon() noexcept
		{
			return KlayGE::half(HALF_EPSILON);
		}
		static KlayGE::half round_error() noexcept
		{
			return KlayGE::half(HALF_EPSILON / 2);
		}

		static KlayGE::half infinity() noexcept
		{
			return KlayGE::half::pos_inf();
		}
		static KlayGE::half quiet_NaN() noexcept
		{
			return KlayGE::half::q_nan();
		}
		static KlayGE::half signaling_NaN() noexcept
		{
			return KlayGE::half::s_nan();
		}
		static KlayGE::half denorm_min() noexcept
		{
			return KlayGE::half(HALF_MIN);
		}
	};
}

#endif			// _KFL_HALF_HPP
