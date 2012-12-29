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

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

#pragma once

namespace KlayGE
{
	// 描述一个平面 ax + by + cz + d = 0
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Plane_T : boost::equality_comparable<Plane_T<T> >
	{
		template <typename U>
		friend class Plane_T;

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
		Plane_T()
			{ }
		explicit Plane_T(T const * rhs)
			: plane_(rhs)
			{ }
		Plane_T(Plane_T const & rhs)
			: plane_(rhs.plane_)
			{ }
		template <typename U>
		Plane_T(Plane_T<U> const & rhs)
			: plane_(rhs.plane_)
			{ }
		template <typename U>
		Plane_T(Vector_T<U, elem_num> const & rhs)
		{
			plane_ = rhs;
		}
		Plane_T(T const & a, T const & b, T const & c, T const & d)
		{
			this->a() = a;
			this->b() = b;
			this->c() = c;
			this->d() = d;
		}

		// 取向量
		iterator begin()
			{ return plane_.begin(); }
		const_iterator begin() const
			{ return plane_.begin(); }
		iterator end()
			{ return plane_.end(); }
		const_iterator end() const
			{ return plane_.end(); }
		reference operator[](size_t index)
			{ return plane_[index]; }
		const_reference operator[](size_t index) const
			{ return plane_[index]; }

		reference a()
			{ return plane_[0]; }
		const_reference a() const
			{ return plane_[0]; }
		reference b()
			{ return plane_[1]; }
		const_reference b() const
			{ return plane_[1]; }
		reference c()
			{ return plane_[2]; }
		const_reference c() const
			{ return plane_[2]; }
		reference d()
			{ return plane_[3]; }
		const_reference d() const
			{ return plane_[3]; }

		// 赋值操作符
		Plane_T& operator=(Plane_T const & rhs)
		{
			if (this != &rhs)
			{
				plane_ = rhs.plane_;
			}
			return *this;
		}
		template <typename U>
		Plane_T& operator=(Plane_T<U> const & rhs)
		{
			if (this != &rhs)
			{
				plane_ = rhs.plane_;
			}
			return *this;
		}
		template <typename U>
		Plane_T& operator=(Vector_T<U, elem_num> const & rhs)
		{
			plane_ = rhs;
			return *this;
		}

		// 一元操作符
		Plane_T const operator+() const
			{ return *this; }
		Plane_T const operator-() const
			{ return Plane_T<T>(-this->a(), -this->b(), -this->c(), -this->d()); }

		// 取法向向量
		Vector_T<T, 3> const Normal() const
			{ return Vector_T<T, 3>(this->a(), this->b(), this->c()); }
		template <typename U>
		void Normal(Vector_T<U, 3> const & rhs)
		{
			this->a() = rhs.x();
			this->b() = rhs.y();
			this->c() = rhs.z();
		}

		bool operator==(Plane_T<T> const & rhs)
		{
			return plane_ == rhs.plane_;
		}

	private:
		Vector_T<T, elem_num> plane_;
	};
}

#endif			// _KFL_PLANE_HPP
