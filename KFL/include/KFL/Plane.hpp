/**
 * @file Plane.hpp
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

#ifndef _KFL_PLANE_HPP
#define _KFL_PLANE_HPP

#pragma once

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

namespace KlayGE
{
	// 描述一个平面 ax + by + cz + d = 0
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Plane_T : boost::equality_comparable<Plane_T<T>>
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
		Plane_T() KLAYGE_NOEXCEPT
		{
		}
		explicit Plane_T(T const * rhs) KLAYGE_NOEXCEPT;
		Plane_T(Plane_T const & rhs) KLAYGE_NOEXCEPT;
		Plane_T(Plane_T&& rhs) KLAYGE_NOEXCEPT;
		Plane_T(Vector_T<T, elem_num> const & rhs) KLAYGE_NOEXCEPT;
		Plane_T(Vector_T<T, elem_num>&& rhs) KLAYGE_NOEXCEPT;
		Plane_T(T a, T b, T c, T d) KLAYGE_NOEXCEPT;

		// 取向量
		iterator begin() KLAYGE_NOEXCEPT
		{
			return plane_.begin();
		}
		const_iterator begin() const KLAYGE_NOEXCEPT
		{
			return plane_.begin();
		}
		iterator end() KLAYGE_NOEXCEPT
		{
			return plane_.end();
		}
		const_iterator end() const KLAYGE_NOEXCEPT
		{
			return plane_.end();
		}
		reference operator[](size_t index) KLAYGE_NOEXCEPT
		{
			return plane_[index];
		}
		const_reference operator[](size_t index) const KLAYGE_NOEXCEPT
		{
			return plane_[index];
		}

		reference a() KLAYGE_NOEXCEPT
		{
			return plane_[0];
		}
		const_reference a() const KLAYGE_NOEXCEPT
		{
			return plane_[0];
		}
		reference b() KLAYGE_NOEXCEPT
		{
			return plane_[1];
		}
		const_reference b() const KLAYGE_NOEXCEPT
		{
			return plane_[1];
		}
		reference c() KLAYGE_NOEXCEPT
		{
			return plane_[2];
		}
		const_reference c() const KLAYGE_NOEXCEPT
		{
			return plane_[2];
		}
		reference d() KLAYGE_NOEXCEPT
		{
			return plane_[3];
		}
		const_reference d() const KLAYGE_NOEXCEPT
		{
			return plane_[3];
		}

		// 赋值操作符
		Plane_T& operator=(Plane_T const & rhs) KLAYGE_NOEXCEPT;
		Plane_T& operator=(Plane_T&& rhs) KLAYGE_NOEXCEPT;
		Plane_T& operator=(Vector_T<T, elem_num> const & rhs) KLAYGE_NOEXCEPT;
		Plane_T& operator=(Vector_T<T, elem_num>&& rhs) KLAYGE_NOEXCEPT;

		// 一元操作符
		Plane_T const operator+() const KLAYGE_NOEXCEPT;
		Plane_T const operator-() const KLAYGE_NOEXCEPT;

		// 取法向向量
		Vector_T<T, 3> const Normal() const KLAYGE_NOEXCEPT;
		void Normal(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT;

		bool operator==(Plane_T<T> const & rhs) const KLAYGE_NOEXCEPT;

	private:
		Vector_T<T, elem_num> plane_;
	};
}

#endif			// _KFL_PLANE_HPP
