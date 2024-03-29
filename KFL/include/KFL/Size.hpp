/**
 * @file Size.hpp
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

#ifndef _KFL_SIZE_HPP
#define _KFL_SIZE_HPP

#pragma once

#include <KFL/Operators.hpp>
#include <KFL/Vector.hpp>

namespace KlayGE
{
	template <typename T>
	class Size_T final
	{
		template <typename U>
		friend class Size_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		static constexpr size_t elem_num = 2;

	public:
		constexpr Size_T() noexcept
		{
		}
		explicit constexpr Size_T(T const* rhs) noexcept : size_(rhs)
		{
		}
		// Leave them in header due to a compiling issue under GCC
		constexpr Size_T(Size_T const& rhs) noexcept : size_(rhs.size_)
		{
		}
		template <typename U>
		constexpr Size_T(Size_T<U> const& rhs) noexcept : size_(rhs.size_)
		{
		}
		constexpr Size_T(Size_T&& rhs) noexcept : size_(std::move(rhs.size_))
		{
		}
		constexpr Size_T(T cx, T cy) noexcept : size_(std::move(cx), std::move(cy))
		{
		}

		// 取向量
		constexpr reference cx() noexcept
		{
			return size_[0];
		}
		constexpr const_reference cx() const noexcept
		{
			return size_[0];
		}
		constexpr reference cy() noexcept
		{
			return size_[1];
		}
		constexpr const_reference cy() const noexcept
		{
			return size_[1];
		}

		// 赋值操作符
		template <typename U>
		Size_T const & operator+=(Size_T<U> const & rhs) noexcept;
		template <typename U>
		Size_T const & operator-=(Size_T<U> const & rhs) noexcept;

		// Leave them in header due to a compiling issue under GCC
		Size_T& operator=(Size_T const & rhs) noexcept
		{
			if (this != &rhs)
			{
				size_ = rhs.size_;
			}
			return *this;
		}
		template <typename U>
		Size_T& operator=(Size_T<U> const & rhs) noexcept
		{
			size_ = rhs.size_;
			return *this;
		}
		Size_T& operator=(Size_T&& rhs) noexcept;

		// 一元操作符
		constexpr Size_T<T> const& operator+() const noexcept
		{
			return *this;
		}
		Size_T<T> const operator-() const noexcept;

		constexpr Vector_T<T, elem_num> const& AsVector()
		{
			return size_;
		}

		bool operator==(Size_T<T> const & rhs) const noexcept;

		KLAYGE_DEFAULT_ADD_OPERATOR1(Size_T<T>);
		KLAYGE_DEFAULT_SUB_OPERATOR1(Size_T<T>);

		KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(Size_T<T>);

	private:
		Vector_T<T, elem_num> size_;
	};

	using Size = Size_T<float>;
	using ISize = Size_T<int32_t>;
	using UISize = Size_T<uint32_t>;
}

#endif			// _KFL_SIZE_HPP
