/**
 * @file Color.hpp
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

#ifndef _KFL_COLOR_HPP
#define _KFL_COLOR_HPP

#pragma once

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

namespace KlayGE
{
	// RGBA，用4个浮点数表示r, g, b, a
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Color_T : boost::addable<Color_T<T>,
						boost::subtractable<Color_T<T>,
						boost::dividable2<Color_T<T>, T,
						boost::multipliable<Color_T<T>,
						boost::multipliable2<Color_T<T>, T,
						boost::equality_comparable<Color_T<T> > > > > > >
	{
	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		enum { elem_num = 4 };

	public:
		Color_T()
			{ }
		explicit Color_T(T const * rhs)
			: col_(rhs)
			{ }
		Color_T(Color_T const & rhs)
			: col_(rhs.col_)
			{ }
		Color_T(T const & r, T const & g, T const & b, T const & a)
		{
			this->r() = r;
			this->g() = g;
			this->b() = b;
			this->a() = a;
		}
		explicit Color_T(uint32_t dw)
		{
			static T const f(1 / T(255));
			this->a() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 24)));
			this->r() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 16)));
			this->g() = f * (static_cast<T>(static_cast<uint8_t>(dw >>  8)));
			this->b() = f * (static_cast<T>(static_cast<uint8_t>(dw >>  0)));
		}

		// 取颜色
		iterator begin()
			{ return col_.begin(); }
		const_iterator begin() const
			{ return col_.begin(); }
		iterator end()
			{ return col_.end(); }
		const_iterator end() const
			{ return col_.end(); }
		reference operator[](size_t index)
			{ return col_[index]; }
		const_reference operator[](size_t index) const
			{ return col_[index]; }

		reference r()
			{ return col_[0]; }
		const_reference r() const
			{ return col_[0]; }
		reference g()
			{ return col_[1]; }
		const_reference g() const
			{ return col_[1]; }
		reference b()
			{ return col_[2]; }
		const_reference b() const
			{ return col_[2]; }
		reference a()
			{ return col_[3]; }
		const_reference a() const
			{ return col_[3]; }

		void RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
		{
			R = static_cast<uint8_t>(MathLib::clamp(this->r(), T(0), T(1)) * 255 + 0.5f);
			G = static_cast<uint8_t>(MathLib::clamp(this->g(), T(0), T(1)) * 255 + 0.5f);
			B = static_cast<uint8_t>(MathLib::clamp(this->b(), T(0), T(1)) * 255 + 0.5f);
			A = static_cast<uint8_t>(MathLib::clamp(this->a(), T(0), T(1)) * 255 + 0.5f);
		}

		uint32_t ARGB() const
		{
			uint8_t r, g, b, a;
			this->RGBA(r, g, b, a);
			return (a << 24) | (r << 16) | (g << 8) | (b << 0);
		}

		uint32_t ABGR() const
		{
			uint8_t r, g, b, a;
			this->RGBA(r, g, b, a);
			return (a << 24) | (b << 16) | (g << 8) | (r << 0);
		}

		// 赋值操作符
		Color_T& operator+=(Color_T const & rhs)
		{
			col_ += rhs.col_;
			return *this;
		}
		Color_T& operator-=(Color_T const & rhs)
		{
			col_ -= rhs.col_;
			return *this;
		}
		Color_T& operator*=(T const & rhs)
		{
			col_ *= rhs;
			return *this;
		}
		Color_T& operator*=(Color_T const & rhs)
		{
			*this = MathLib::modulate(*this, rhs);
			return *this;
		}
		Color_T& operator/=(T const & rhs)
		{
			col_ /= rhs;
			return *this;
		}

		Color_T& operator=(Color_T const & rhs)
		{
			if (this != &rhs)
			{
				col_ = rhs.col_;
			}
			return *this;
		}

		bool operator==(Color_T const & rhs)
		{
			return col_ == rhs.col_;
		}

		// 一元操作符
		Color_T const operator+() const
			{ return *this; }
		Color_T const operator-() const
			{ return Color_T(-this->r(), -this->g(), -this->b(), -this->a()); }

		friend bool
		operator==(Color_T const & lhs, Color_T const & rhs)
		{
			return lhs.col_ == rhs.col_;
		}

	private:
		Vector_T<T, elem_num> col_;
	};

	typedef Color_T<float> Color;
}

#endif			// _KFL_COLOR_HPP
