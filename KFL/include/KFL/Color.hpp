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

#include <KFL/PreDeclare.hpp>

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
						boost::equality_comparable<Color_T<T>>>>>>>
	{
	public:
		enum { elem_num = 4 };

		typedef T value_type;

		typedef typename Vector_T<T, elem_num>::pointer pointer;
		typedef typename Vector_T<T, elem_num>::const_pointer const_pointer;

		typedef typename Vector_T<T, elem_num>::reference reference;
		typedef typename Vector_T<T, elem_num>::const_reference const_reference;

		typedef typename Vector_T<T, elem_num>::iterator iterator;
		typedef typename Vector_T<T, elem_num>::const_iterator const_iterator;

	public:
		Color_T() KLAYGE_NOEXCEPT
		{
		}
		explicit Color_T(T const * rhs) KLAYGE_NOEXCEPT;
		Color_T(Color_T const & rhs) KLAYGE_NOEXCEPT;
		Color_T(Color_T&& rhs) KLAYGE_NOEXCEPT;
		Color_T(T r, T g, T b, T a) KLAYGE_NOEXCEPT;
		explicit Color_T(uint32_t dw) KLAYGE_NOEXCEPT;

		// 取颜色
		iterator begin() KLAYGE_NOEXCEPT
		{
			return col_.begin();
		}
		const_iterator begin() const KLAYGE_NOEXCEPT
		{
			return col_.begin();
		}
		iterator end() KLAYGE_NOEXCEPT
		{
			return col_.end();
		}
		const_iterator end() const KLAYGE_NOEXCEPT
		{
			return col_.end();
		}
		reference operator[](size_t index) KLAYGE_NOEXCEPT
		{
			return col_[index];
		}
		const_reference operator[](size_t index) const KLAYGE_NOEXCEPT
		{
			return col_[index];
		}

		reference r() KLAYGE_NOEXCEPT
		{
			return col_[0];
		}
		const_reference r() const KLAYGE_NOEXCEPT
		{
			return col_[0];
		}
		reference g() KLAYGE_NOEXCEPT
		{
			return col_[1];
		}
		const_reference g() const KLAYGE_NOEXCEPT
		{
			return col_[1];
		}
		reference b() KLAYGE_NOEXCEPT
		{
			return col_[2];
		}
		const_reference b() const KLAYGE_NOEXCEPT
		{
			return col_[2];
		}
		reference a() KLAYGE_NOEXCEPT
		{
			return col_[3];
		}
		const_reference a() const KLAYGE_NOEXCEPT
		{
			return col_[3];
		}

		void RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const KLAYGE_NOEXCEPT;

		uint32_t ARGB() const KLAYGE_NOEXCEPT;
		uint32_t ABGR() const KLAYGE_NOEXCEPT;

		// 赋值操作符
		Color_T& operator+=(Color_T<T> const & rhs) KLAYGE_NOEXCEPT;
		Color_T& operator-=(Color_T<T> const & rhs) KLAYGE_NOEXCEPT;
		Color_T& operator*=(T rhs) KLAYGE_NOEXCEPT;
		Color_T& operator*=(Color_T<T> const & rhs) KLAYGE_NOEXCEPT;
		Color_T& operator/=(T rhs) KLAYGE_NOEXCEPT;

		Color_T& operator=(Color_T const & rhs) KLAYGE_NOEXCEPT;
		Color_T& operator=(Color_T&& rhs) KLAYGE_NOEXCEPT;

		// 一元操作符
		Color_T const operator+() const KLAYGE_NOEXCEPT;
		Color_T const operator-() const KLAYGE_NOEXCEPT;

		bool operator==(Color_T<T> const & rhs) const KLAYGE_NOEXCEPT;

	private:
		Vector_T<T, elem_num> col_;
	};
}

#endif			// _KFL_COLOR_HPP
