/**
 * @file Rect.hpp
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

#ifndef _KFL_RECT_HPP
#define _KFL_RECT_HPP

#pragma once

#include <KFL/Operators.hpp>
#include <KFL/Size.hpp>
#include <KFL/Vector.hpp>

namespace KlayGE
{
	template <typename T>
	class Rect_T final
	{
		template <typename U>
		friend class Rect_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		static constexpr size_t elem_num = 4;

	public:
		constexpr Rect_T() noexcept
		{
		}
		explicit constexpr Rect_T(T const* rhs) noexcept : rect_(rhs)
		{
		}
		// Leave them in header due to a compiling issue under GCC
		constexpr Rect_T(Rect_T const& rhs) noexcept : rect_(rhs.rect_)
		{
		}
		template <typename U>
		constexpr Rect_T(Rect_T<U> const& rhs) noexcept : rect_(rhs.rect_)
		{
		}
		constexpr Rect_T(Rect_T&& rhs) noexcept : rect_(std::move(rhs.rect_))
		{
		}
		constexpr Rect_T(T left, T top, T right, T bottom) noexcept
			: rect_(std::move(left), std::move(top), std::move(right), std::move(bottom))
		{
		}

		// 取向量
		constexpr reference left() noexcept
		{
			return rect_[0];
		}
		constexpr const_reference left() const noexcept
		{
			return rect_[0];
		}
		constexpr reference top() noexcept
		{
			return rect_[1];
		}
		constexpr const_reference top() const noexcept
		{
			return rect_[1];
		}
		constexpr reference right() noexcept
		{
			return rect_[2];
		}
		constexpr const_reference right() const noexcept
		{
			return rect_[2];
		}
		constexpr reference bottom() noexcept
		{
			return rect_[3];
		}
		constexpr const_reference bottom() const noexcept
		{
			return rect_[3];
		}

		// 赋值操作符
		template <typename U>
		Rect_T const & operator+=(Vector_T<U, 2> const & rhs) noexcept;
		template <typename U>
		Rect_T const & operator-=(Vector_T<U, 2> const & rhs) noexcept;
		template <typename U>
		Rect_T const & operator+=(Rect_T<U> const & rhs) noexcept;
		template <typename U>
		Rect_T const & operator-=(Rect_T<U> const & rhs) noexcept;
		template <typename U>
		Rect_T const & operator&=(Rect_T<U> const & rhs) noexcept;
		template <typename U>
		Rect_T const & operator|=(Rect_T<U> const & rhs) noexcept;

		// Leave them in header due to a compiling issue under GCC
		Rect_T& operator=(Rect_T const & rhs) noexcept
		{
			if (this != &rhs)
			{
				rect_ = rhs.rect_;
			}
			return *this;
		}
		template <typename U>
		Rect_T& operator=(Rect_T<U> const & rhs) noexcept
		{
			rect_ = rhs.rect_;
			return *this;
		}
		Rect_T& operator=(Rect_T&& rhs) noexcept;

		// 一元操作符
		constexpr Rect_T const& operator+() const noexcept
		{
			return *this;
		}
		Rect_T const operator-() const noexcept;

		// 属性
		T Width() const noexcept;
		T Height() const noexcept;
		Size_T<T> const Size() const noexcept;
		bool IsEmpty() const noexcept;

		constexpr Vector_T<T, elem_num> const& AsVector()
		{
			return rect_;
		}

		bool operator==(Rect_T<T> const & rhs) const noexcept;

		bool PtInRect(Vector_T<T, 2> const & pt) const noexcept;

		KLAYGE_DEFAULT_ADD_OPERATOR1(Rect_T<T>);
		KLAYGE_DEFAULT_ADD_OPERATOR2(Rect_T<T>, KLAYGE_ESC(Vector_T<T, 2>));

		KLAYGE_DEFAULT_SUB_OPERATOR1(Rect_T<T>);
		KLAYGE_DEFAULT_SUB_OPERATOR2(Rect_T<T>, KLAYGE_ESC(Vector_T<T, 2>));

		KLAYGE_DEFAULT_AND_OPERATOR1(Rect_T<T>);

		KLAYGE_DEFAULT_OR_OPERATOR1(Rect_T<T>);

		KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(Rect_T<T>);

	private:
		Vector_T<T, elem_num> rect_;
	};

	using Rect = Rect_T<float>;
	using IRect = Rect_T<int32_t>;
	using UIRect = Rect_T<uint32_t>;
}

#endif			// _KFL_RECT_HPP
