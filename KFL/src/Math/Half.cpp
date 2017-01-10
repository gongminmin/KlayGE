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

#include <KFL/KFL.hpp>

#include <KFL/Half.hpp>

namespace KlayGE
{
	half::half(float f) noexcept
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

	half::operator float() const noexcept
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

	half half::pos_inf() noexcept
	{
		half h;
		h.value_ = 0x7C00;
		return h;
	}

	half half::neg_inf() noexcept
	{
		half h;
		h.value_ = 0xFC00;
		return h;
	}

	half half::q_nan() noexcept
	{
		half h;
		h.value_ = 0x7FFF;
		return h;
	}

	half half::s_nan() noexcept
	{
		half h;
		h.value_ = 0x7DFF;
		return h;
	}


	half const & half::operator+=(half const & rhs) noexcept
	{
		*this = half(float(*this) + float(rhs));
		return *this;
	}

	half const & half::operator-=(half const & rhs) noexcept
	{
		*this = half(float(*this) - float(rhs));
		return *this;
	}

	half const & half::operator*=(half const & rhs) noexcept
	{
		*this = half(float(*this) * float(rhs));
		return *this;
	}

	half const & half::operator/=(half const & rhs) noexcept
	{
		*this = half(float(*this) / float(rhs));
		return *this;
	}

	half& half::operator=(half const & rhs) noexcept
	{
		if (this != &rhs)
		{
			value_ = rhs.value_;
		}
		return *this;
	}

	// Ò»Ôª²Ù×÷·û
	half const half::operator+() const noexcept
	{
		return *this;
	}

	half const half::operator-() const noexcept
	{
		half temp(*this);
		temp.value_ = -temp.value_;
		return temp;
	}

	bool half::operator==(half const & rhs) noexcept
	{
		return value_ == rhs.value_;
	}
}
