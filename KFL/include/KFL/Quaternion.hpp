/**
 * @file Quaternion.hpp
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

#ifndef _KFL_QUATERNION_HPP
#define _KFL_QUATERNION_HPP

#pragma once

#include <KFL/Operators.hpp>
#include <KFL/Vector.hpp>

namespace KlayGE
{
	template <typename T>
	class Quaternion_T final
	{
	public:
		static constexpr size_t elem_num = 4;

		typedef T value_type;

		typedef typename Vector_T<T, elem_num>::pointer pointer;
		typedef typename Vector_T<T, elem_num>::const_pointer const_pointer;

		typedef typename Vector_T<T, elem_num>::reference reference;
		typedef typename Vector_T<T, elem_num>::const_reference const_reference;

		typedef typename Vector_T<T, elem_num>::iterator iterator;
		typedef typename Vector_T<T, elem_num>::const_iterator const_iterator;

	public:
		constexpr Quaternion_T() noexcept
		{
		}
		explicit constexpr Quaternion_T(T const* rhs) noexcept : quat_(rhs)
		{
		}
		constexpr Quaternion_T(Vector_T<T, 3> const& vec, T s) noexcept : quat_(vec.x(), vec.y(), vec.z(), s)
		{
		}
		constexpr Quaternion_T(Quaternion_T const& rhs) noexcept : quat_(rhs.quat_)
		{
		}
		constexpr Quaternion_T(Quaternion_T&& rhs) noexcept : quat_(std::move(rhs.quat_))
		{
		}
		constexpr Quaternion_T(T x, T y, T z, T w) noexcept : quat_(std::move(x), std::move(y), std::move(z), std::move(w))
		{
		}

		static constexpr size_t size() noexcept
		{
			return elem_num;
		}

		static Quaternion_T const & Identity() noexcept;

		// 取向量
		constexpr iterator begin() noexcept
		{
			return quat_.begin();
		}
		constexpr const_iterator begin() const noexcept
		{
			return quat_.begin();
		}
		constexpr iterator end() noexcept
		{
			return quat_.end();
		}
		constexpr const_iterator end() const noexcept
		{
			return quat_.end();
		}
		reference operator[](size_t index) noexcept
		{
			return quat_[index];
		}
		constexpr const_reference operator[](size_t index) const noexcept
		{
			return quat_[index];
		}

		constexpr reference x() noexcept
		{
			return quat_[0];
		}
		constexpr const_reference x() const noexcept
		{
			return quat_[0];
		}
		constexpr reference y() noexcept
		{
			return quat_[1];
		}
		constexpr const_reference y() const noexcept
		{
			return quat_[1];
		}
		constexpr reference z() noexcept
		{
			return quat_[2];
		}
		constexpr const_reference z() const noexcept
		{
			return quat_[2];
		}
		constexpr reference w() noexcept
		{
			return quat_[3];
		}
		constexpr const_reference w() const noexcept
		{
			return quat_[3];
		}

		// 赋值操作符
		Quaternion_T const & operator+=(Quaternion_T const & rhs) noexcept;
		Quaternion_T const & operator-=(Quaternion_T const & rhs) noexcept;

		Quaternion_T const & operator*=(Quaternion_T const & rhs) noexcept;
		Quaternion_T const & operator*=(T const& rhs) noexcept;
		Quaternion_T const & operator/=(T const& rhs) noexcept;

		Quaternion_T& operator=(Quaternion_T const & rhs) noexcept;
		Quaternion_T& operator=(Quaternion_T&& rhs) noexcept;

		// 一元操作符
		constexpr Quaternion_T const& operator+() const noexcept
		{
			return *this;
		}
		Quaternion_T const operator-() const noexcept;

		// 取方向向量
		constexpr Vector_T<T, 3> const& v() const noexcept
		{
			return quat_.template AsVector<3>();
		}
		void v(Vector_T<T, 3> const & rhs) noexcept;

		constexpr Vector_T<T, 4> const& AsVector4() const noexcept
		{
			return quat_;
		}

		bool operator==(Quaternion_T<T> const & rhs) const noexcept;

		DEFAULT_ADD_OPERATOR1(Quaternion_T<T>);
		DEFAULT_SUB_OPERATOR1(Quaternion_T<T>);

		DEFAULT_MUL_OPERATOR1(Quaternion_T<T>);
		DEFAULT_MUL_OPERATOR2(Quaternion_T<T>, T);
		DEFAULT_MUL_OPERATOR3(T, Quaternion_T<T>);

		DEFAULT_DIV_OPERATOR2(Quaternion_T<T>, T);

		DEFAULT_EQUALITY_COMPARE_OPERATOR(Quaternion_T<T>);

	private:
		Vector_T<T, elem_num> quat_;
	};
}

#endif			// _KFL_QUATERNION_HPP
