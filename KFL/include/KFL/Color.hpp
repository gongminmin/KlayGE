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
	class Color_T final : boost::addable<Color_T<T>,
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
		constexpr Color_T() noexcept
		{
		}
		explicit constexpr Color_T(T const * rhs) noexcept
			: col_(rhs)
		{
		}
		Color_T(Color_T const & rhs) noexcept;
		Color_T(Color_T&& rhs) noexcept;
		constexpr Color_T(T r, T g, T b, T a) noexcept
			: col_(r, g, b, a)
		{
		}
		explicit Color_T(uint32_t dw) noexcept;

		// 取颜色
		iterator begin() noexcept
		{
			return col_.begin();
		}
		constexpr const_iterator begin() const noexcept
		{
			return col_.begin();
		}
		iterator end() noexcept
		{
			return col_.end();
		}
		constexpr const_iterator end() const noexcept
		{
			return col_.end();
		}
		reference operator[](size_t index) noexcept
		{
			return col_[index];
		}
		constexpr const_reference operator[](size_t index) const noexcept
		{
			return col_[index];
		}

		reference r() noexcept
		{
			return col_[0];
		}
		constexpr const_reference r() const noexcept
		{
			return col_[0];
		}
		reference g() noexcept
		{
			return col_[1];
		}
		constexpr const_reference g() const noexcept
		{
			return col_[1];
		}
		reference b() noexcept
		{
			return col_[2];
		}
		constexpr const_reference b() const noexcept
		{
			return col_[2];
		}
		reference a() noexcept
		{
			return col_[3];
		}
		constexpr const_reference a() const noexcept
		{
			return col_[3];
		}

		void RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const noexcept;

		uint32_t ARGB() const noexcept;
		uint32_t ABGR() const noexcept;

		// 赋值操作符
		Color_T& operator+=(Color_T<T> const & rhs) noexcept;
		Color_T& operator-=(Color_T<T> const & rhs) noexcept;
		Color_T& operator*=(T rhs) noexcept;
		Color_T& operator*=(Color_T<T> const & rhs) noexcept;
		Color_T& operator/=(T rhs) noexcept;

		Color_T& operator=(Color_T const & rhs) noexcept;
		Color_T& operator=(Color_T&& rhs) noexcept;

		// 一元操作符
		Color_T const operator+() const noexcept;
		Color_T const operator-() const noexcept;

		bool operator==(Color_T<T> const & rhs) const noexcept;

	private:
		Vector_T<T, elem_num> col_;
	};
}

#endif			// _KFL_COLOR_HPP
