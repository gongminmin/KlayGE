/**
 * @file Color.cpp
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

#include <KFL/Color.hpp>

namespace KlayGE
{
	template Color_T<float>::Color_T(float const * rhs);
	template Color_T<float>::Color_T(Color const & rhs);
	template Color_T<float>::Color_T(Color&& rhs);
	template Color_T<float>::Color_T(float r, float g, float b, float a);
	template Color_T<float>::Color_T(uint32_t dw);
	template void Color_T<float>::RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const;
	template uint32_t Color_T<float>::ARGB() const;
	template uint32_t Color_T<float>::ABGR() const;
	template Color& Color_T<float>::operator+=(Color const & rhs);
	template Color& Color_T<float>::operator-=(Color const & rhs);
	template Color& Color_T<float>::operator*=(float rhs);
	template Color& Color_T<float>::operator*=(Color const & rhs);
	template Color& Color_T<float>::operator/=(float rhs);
	template Color& Color_T<float>::operator=(Color const & rhs);
	template Color& Color_T<float>::operator=(Color&& rhs);
	template Color const Color_T<float>::operator+() const;
	template Color const Color_T<float>::operator-() const;
	template bool Color_T<float>::operator==(Color const & rhs) const;


	template <typename T>
	Color_T<T>::Color_T(T const * rhs)
		: col_(rhs)
	{
	}

	template <typename T>
	Color_T<T>::Color_T(Color_T const & rhs)
		: col_(rhs.col_)
	{
	}

	template <typename T>
	Color_T<T>::Color_T(Color_T&& rhs)
		: col_(std::move(rhs.col_))
	{
	}

	template <typename T>
	Color_T<T>::Color_T(T r, T g, T b, T a)
	{
		this->r() = r;
		this->g() = g;
		this->b() = b;
		this->a() = a;
	}

	template <typename T>
	Color_T<T>::Color_T(uint32_t dw)
	{
		static T const f(1 / T(255));
		this->a() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 24)));
		this->r() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 16)));
		this->g() = f * (static_cast<T>(static_cast<uint8_t>(dw >>  8)));
		this->b() = f * (static_cast<T>(static_cast<uint8_t>(dw >>  0)));
	}

	template <typename T>
	void Color_T<T>::RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
	{
		R = static_cast<uint8_t>(MathLib::clamp(this->r(), T(0), T(1)) * 255 + 0.5f);
		G = static_cast<uint8_t>(MathLib::clamp(this->g(), T(0), T(1)) * 255 + 0.5f);
		B = static_cast<uint8_t>(MathLib::clamp(this->b(), T(0), T(1)) * 255 + 0.5f);
		A = static_cast<uint8_t>(MathLib::clamp(this->a(), T(0), T(1)) * 255 + 0.5f);
	}

	template <typename T>
	uint32_t Color_T<T>::ARGB() const
	{
		uint8_t r, g, b, a;
		this->RGBA(r, g, b, a);
		return (a << 24) | (r << 16) | (g << 8) | (b << 0);
	}

	template <typename T>
	uint32_t Color_T<T>::ABGR() const
	{
		uint8_t r, g, b, a;
		this->RGBA(r, g, b, a);
		return (a << 24) | (b << 16) | (g << 8) | (r << 0);
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator+=(Color_T<T> const & rhs)
	{
		col_ += rhs.col_;
		return *this;
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator-=(Color_T<T> const & rhs)
	{
		col_ -= rhs.col_;
		return *this;
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator*=(T rhs)
	{
		col_ *= rhs;
		return *this;
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator*=(Color_T<T> const & rhs)
	{
		*this = MathLib::modulate(*this, rhs);
		return *this;
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator/=(T rhs)
	{
		col_ /= rhs;
		return *this;
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator=(Color_T<T> const & rhs)
	{
		if (this != &rhs)
		{
			col_ = rhs.col_;
		}
		return *this;
	}

	template <typename T>
	Color_T<T>& Color_T<T>::operator=(Color_T<T>&& rhs)
	{
		col_ = std::move(rhs.col_);
		return *this;
	}

	template <typename T>
	Color_T<T> const Color_T<T>::operator+() const
	{
		return *this;
	}
	
	template <typename T>
	Color_T<T> const Color_T<T>::operator-() const
	{
		return Color_T(-this->r(), -this->g(), -this->b(), -this->a());
	}

	template <typename T>
	bool Color_T<T>::operator==(Color_T<T> const & rhs) const
	{
		return col_ == rhs.col_;
	}
}
