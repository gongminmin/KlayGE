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

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

namespace KlayGE
{
	template <typename T>
	class Quaternion_T : boost::addable<Quaternion_T<T>,
						boost::subtractable<Quaternion_T<T>,
						boost::dividable2<Quaternion_T<T>, T,
						boost::multipliable<Quaternion_T<T>,
						boost::multipliable2<Quaternion_T<T>, T,
						boost::equality_comparable<Quaternion_T<T>>>>>>>
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
		Quaternion_T() KLAYGE_NOEXCEPT
		{
		}
		explicit Quaternion_T(T const * rhs) KLAYGE_NOEXCEPT;
		Quaternion_T(Vector_T<T, 3> const & vec, T s) KLAYGE_NOEXCEPT;
		Quaternion_T(Quaternion_T const & rhs) KLAYGE_NOEXCEPT;
		Quaternion_T(Quaternion_T&& rhs) KLAYGE_NOEXCEPT;
		Quaternion_T(T x, T y, T z, T w) KLAYGE_NOEXCEPT;

		static Quaternion_T const & Identity() KLAYGE_NOEXCEPT;

		// 取向量
		iterator begin() KLAYGE_NOEXCEPT
		{
			return quat_.begin();
		}
		const_iterator begin() const KLAYGE_NOEXCEPT
		{
			return quat_.begin();
		}
		iterator end() KLAYGE_NOEXCEPT
		{
			return quat_.end();
		}
		const_iterator end() const KLAYGE_NOEXCEPT
		{
			return quat_.end();
		}
		reference operator[](size_t index) KLAYGE_NOEXCEPT
		{
			return quat_[index];
		}
		const_reference operator[](size_t index) const KLAYGE_NOEXCEPT
		{
			return quat_[index];
		}

		reference x() KLAYGE_NOEXCEPT
		{
			return quat_[0];
		}
		const_reference x() const KLAYGE_NOEXCEPT
		{
			return quat_[0];
		}
		reference y() KLAYGE_NOEXCEPT
		{
			return quat_[1];
		}
		const_reference y() const KLAYGE_NOEXCEPT
		{
			return quat_[1];
		}
		reference z() KLAYGE_NOEXCEPT
		{
			return quat_[2];
		}
		const_reference z() const KLAYGE_NOEXCEPT
		{
			return quat_[2];
		}
		reference w() KLAYGE_NOEXCEPT
		{
			return quat_[3];
		}
		const_reference w() const KLAYGE_NOEXCEPT
		{
			return quat_[3];
		}

		// 赋值操作符
		Quaternion_T const & operator+=(Quaternion_T const & rhs) KLAYGE_NOEXCEPT;
		Quaternion_T const & operator-=(Quaternion_T const & rhs) KLAYGE_NOEXCEPT;

		Quaternion_T const & operator*=(Quaternion_T const & rhs) KLAYGE_NOEXCEPT;
		Quaternion_T const & operator*=(T rhs) KLAYGE_NOEXCEPT;
		Quaternion_T const & operator/=(T rhs) KLAYGE_NOEXCEPT;

		Quaternion_T& operator=(Quaternion_T const & rhs) KLAYGE_NOEXCEPT;
		Quaternion_T& operator=(Quaternion_T&& rhs) KLAYGE_NOEXCEPT;

		// 一元操作符
		Quaternion_T const operator+() const KLAYGE_NOEXCEPT;
		Quaternion_T const operator-() const KLAYGE_NOEXCEPT;

		// 取方向向量
		Vector_T<T, 3> const v() const KLAYGE_NOEXCEPT;
		void v(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT;

		bool operator==(Quaternion_T<T> const & rhs) const KLAYGE_NOEXCEPT;

	private:
		Vector_T<T, elem_num> quat_;
	};
}

#endif			// _KFL_QUATERNION_HPP
