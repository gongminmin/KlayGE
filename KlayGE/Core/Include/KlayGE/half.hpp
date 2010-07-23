// half.hpp
// KlayGE 半精度浮点类型头文件
// Ver 2.6.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
//
// 2.6.0
// 初次建立 (2005.5.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <boost/operators.hpp>
#include <boost/cstdint.hpp>
#include <limits>

#pragma once

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
	class half : boost::addable<half,
						boost::subtractable<half,
						boost::multipliable<half,
						boost::dividable<half> > > >
	{
	public:
		half()
		{
		}

		explicit half(float f)
		{
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = f;
			int32_t i = fni.i;

			int32_t s = (i >> 16) & 0x00008000;
			int32_t e = ((i >> 23) & 0x000000FF) - (127 - 15);
			int32_t m = i & 0x007FFFFF;

			if (e <= 0)
			{
				if (e < -10)
				{
					value_ = 0;
				}
				else
				{
					m = (m | 0x00800000) >> (1 - e);

					if (m &  0x00001000)
					{
						m += 0x00002000;
					}

					value_ = static_cast<uint16_t>(s | (m >> 13));
				}
			}
			else
			{
				if (0xFF - (127 - 15) == e)
				{
					e = 31;
				}
				else
				{
					if (m & 0x00001000)
					{
						m += 0x00002000;

						if (m & 0x00800000)
						{
							m = 0;		// overflow in significand,
							e += 1;		// adjust exponent
						}
					}
				}

				value_ = static_cast<uint16_t>(s | (e << 10) | (m >> 13));
			}
		}

		operator float() const
		{
			int32_t ret;

			int32_t s = ((value_ & 0x8000) >> 15) << 31;
			int32_t e = (value_ & 0x7C00) >> 10;
			int32_t m = value_ & 0x03FF;

			if (0 == e)
			{
				if (m != 0)
				{
					// Denormalized number -- renormalize it

					while (!(m & 0x00000400))
					{
						m <<= 1;
						e -= 1;
					}

					e += 1;
					m &= ~0x00000400;
				}
			}
			else
			{
				if (31 == e)
				{
					if (m != 0)
					{
						// Nan -- preserve sign and significand bits
						e = 0xFF - (127 - 15);
					}
				}
			}

			// Normalized number
			e += 127 - 15;
			m <<= 13;

			ret = s | (e << 23) | m;

			union INF
			{
				int32_t i;
				float f;
			} inf;
			inf.i = ret;
			return inf.f;
		}

		// 特殊值

		// returns +infinity
		static half pos_inf()
		{
			half h;
			h.value_ = 0x7C00;
			return h;
		}

		// returns -infinity
		static half neg_inf()
		{
			half h;
			h.value_ = 0xFC00;
			return h;
		}

		// returns a NAN with the bit pattern 0111111111111111
		static half q_nan()
		{
			half h;
			h.value_ = 0x7FFF;
			return h;
		}

		// returns a NAN with the bit pattern 0111110111111111
		static half s_nan()
		{
			half h;
			h.value_ = 0x7DFF;
			return h;
		}


		// 赋值操作符
		half const & operator+=(half const & rhs)
		{
			*this = half(float(*this) + float(rhs));
			return *this;
		}
		half const & operator-=(half const & rhs)
		{
			*this = half(float(*this) - float(rhs));
			return *this;
		}
		half const & operator*=(half const & rhs)
		{
			*this = half(float(*this) * float(rhs));
			return *this;
		}
		half const & operator/=(half const & rhs)
		{
			*this = half(float(*this) / float(rhs));
			return *this;
		}

		half& operator=(half const & rhs)
		{
			if (this != &rhs)
			{
				value_ = rhs.value_;
			}
			return *this;
		}

		// 一元操作符
		half const operator+() const
		{
			return *this;
		}
		half const operator-() const
		{
			half temp(*this);
			temp.value_ = -temp.value_;
			return temp;
		}

		friend bool
		operator==(half const & lhs, half const & rhs)
		{
			return lhs.value_ == rhs.value_;
		}

	private:
		uint16_t value_;
	};

	inline bool
	operator!=(half const & lhs, half const & rhs)
	{
		return !(lhs == rhs);
	}
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

		static KlayGE::half min() throw()
		{
			return KlayGE::half(HALF_NRM_MIN);
		}
		static KlayGE::half max() throw()
		{
			return KlayGE::half(HALF_MAX);
		}
		static KlayGE::half epsilon() throw()
		{
			return KlayGE::half(HALF_EPSILON);
		}
		static KlayGE::half round_error() throw()
		{
			return KlayGE::half(HALF_EPSILON / 2);
		}

		static KlayGE::half infinity() throw()
		{
			return KlayGE::half::pos_inf();
		}
		static KlayGE::half quiet_NaN() throw()
		{
			return KlayGE::half::q_nan();
		}
		static KlayGE::half signaling_NaN() throw()
		{
			return KlayGE::half::s_nan();
		}
		static KlayGE::half denorm_min() throw()
		{
			return KlayGE::half(HALF_MIN);
		}
	};
}
