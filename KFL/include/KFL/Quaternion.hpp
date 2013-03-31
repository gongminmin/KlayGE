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

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

#pragma once

namespace KlayGE
{
	template <typename T>
	class Quaternion_T : boost::addable<Quaternion_T<T>,
						boost::subtractable<Quaternion_T<T>,
						boost::dividable2<Quaternion_T<T>, T,
						boost::multipliable<Quaternion_T<T>,
						boost::multipliable2<Quaternion_T<T>, T,
						boost::equality_comparable<Quaternion_T<T> > > > > > >
	{
		template <typename U>
		friend class Quaternion_T;

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
		Quaternion_T()
			{ }
		explicit Quaternion_T(T const * rhs)
			: quat_(rhs)
			{ }
		Quaternion_T(Vector_T<T, 3> const & vec, T const & s)
		{
			this->x() = vec.x();
			this->y() = vec.y();
			this->z() = vec.z();
			this->w() = s;
		}
		Quaternion_T(Quaternion_T const & rhs)
			: quat_(rhs.quat_)
			{ }
		template <typename U>
		Quaternion_T(Quaternion_T<U> const & rhs)
			: quat_(rhs.quat_)
			{ }
		Quaternion_T(T const & x, T const & y, T const & z, T const & w)
		{
			this->x() = x;
			this->y() = y;
			this->z() = z;
			this->w() = w;
		}

		static Quaternion_T const & Identity()
		{
			static Quaternion_T const out(0, 0, 0, 1);
			return out;
		}

		// 取向量
		iterator begin()
			{ return quat_.begin(); }
		const_iterator begin() const
			{ return quat_.begin(); }
		iterator end()
			{ return quat_.end(); }
		const_iterator end() const
			{ return quat_.end(); }
		reference operator[](size_t index)
			{ return quat_[index]; }
		const_reference operator[](size_t index) const
			{ return quat_[index]; }

		reference x()
			{ return quat_[0]; }
		const_reference x() const
			{ return quat_[0]; }
		reference y()
			{ return quat_[1]; }
		const_reference y() const
			{ return quat_[1]; }
		reference z()
			{ return quat_[2]; }
		const_reference z() const
			{ return quat_[2]; }
		reference w()
			{ return quat_[3]; }
		const_reference w() const
			{ return quat_[3]; }

		// 赋值操作符
		template <typename U>
		Quaternion_T const & operator+=(Quaternion_T<U> const & rhs)
		{
			quat_ += rhs.quat_;
			return *this;
		}
		template <typename U>
		Quaternion_T const & operator-=(Quaternion_T<U> const & rhs)
		{
			quat_ -= rhs.quat_;
			return *this;
		}

		template <typename U>
		Quaternion_T const & operator*=(Quaternion_T<U> const & rhs)
		{
			*this = MathLib::mul(*this, rhs);
			return *this;
		}
		template <typename U>
		Quaternion_T const & operator*=(U const & rhs)
		{
			quat_ *= rhs;
			return *this;
		}
		template <typename U>
		Quaternion_T const & operator/=(U const & rhs)
		{
			quat_ /= rhs;
			return *this;
		}

		Quaternion_T& operator=(Quaternion_T const & rhs)
		{
			if (this != &rhs)
			{
				quat_ = rhs.quat_;
			}
			return *this;
		}
		template <typename U>
		Quaternion_T& operator=(Quaternion_T<U> const & rhs)
		{
			if (this != &rhs)
			{
				quat_ = rhs.quat_;
			}
			return *this;
		}

		// 一元操作符
		Quaternion_T const operator+() const
			{ return *this; }
		Quaternion_T const operator-() const
			{ return Quaternion_T(-this->x(), -this->y(), -this->z(), -this->w()); }

		// 取方向向量
		Vector_T<T, 3> const v() const
			{ return Vector_T<T, 3>(this->x(), this->y(), this->z()); }
		void v(Vector_T<T, 3> const & rhs)
		{
			this->x() = rhs.x();
			this->y() = rhs.y();
			this->z() = rhs.z();
		}

		bool operator==(Quaternion_T<T> const & rhs)
		{
			return quat_ == rhs.quat_;
		}

	private:
		Vector_T<T, elem_num> quat_;
	};
}

#endif			// _KFL_QUATERNION_HPP
