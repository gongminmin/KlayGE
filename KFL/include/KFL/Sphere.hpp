/**
 * @file Sphere.hpp
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

#ifndef _KFL_SPHERE_HPP
#define _KFL_SPHERE_HPP

#include <KFL/PreDeclare.hpp>

#include <boost/assert.hpp>
#include <boost/operators.hpp>

#include <KFL/Bound.hpp>

#pragma once

namespace KlayGE
{
	template <typename T>
	class Sphere_T : boost::addable2<Sphere_T<T>, Vector_T<T, 3>,
						boost::subtractable2<Sphere_T<T>, Vector_T<T, 3>,
						boost::multipliable2<Sphere_T<T>, T,
						boost::dividable2<Sphere_T<T>, T,
						boost::equality_comparable<Sphere_T<T> > > > > >,
				public Bound_T<T>
	{
	public:
		Sphere_T()
		{
		}
		Sphere_T(Vector_T<T, 3> const & center, T const & radius)
			: center_(center),
				radius_(radius)
		{
		}

		// 赋值操作符
		Sphere_T& operator+=(Vector_T<T, 3> const & rhs)
		{
			this->Center() += rhs;
			return *this;
		}
		Sphere_T& operator-=(Vector_T<T, 3> const & rhs)
		{
			this->Center() -= rhs;
			return *this;
		}
		Sphere_T& operator*=(T const & rhs)
		{
			this->Radius() *= rhs;
			return *this;
		}
		Sphere_T& operator/=(T const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}

		Sphere_T& operator=(Sphere_T const & rhs)
		{
			if (this != &rhs)
			{
				this->Center() = rhs.Center();
				this->Radius() = rhs.Radius();
			}
			return *this;
		}

		// 一元操作符
		Sphere_T const & operator+() const
		{
			return *this;
		}
		Sphere_T const & operator-() const
		{
			return *this;
		}

		// 属性
		Vector_T<T, 3>& Center()
		{
			return center_;
		}
		Vector_T<T, 3> const & Center() const
		{
			return center_;
		}
		T& Radius()
		{
			return radius_;
		}
		T Radius() const	
		{
			return radius_;
		}

		bool IsEmpty() const
		{
			return MathLib::equal(radius_, 0.0f);
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::intersect_point_sphere(v, *this);
		}
		T MaxRadiusSq() const
		{
			return this->Radius() * this->Radius();
		}

		bool Intersect(AABBox_T<T> const & aabb) const
		{
			return aabb.Intersect(*this);
		}
		bool Intersect(OBBox_T<T> const & obb) const
		{
			return obb.Intersect(*this);
		}
		bool Intersect(Sphere_T<T> const & sphere) const
		{
			return intersect_sphere_sphere(*this, sphere);
		}
		bool Intersect(Frustum_T<T> const & frustum) const
		{
			return intersect_sphere_frustum(*this, frustum) != BO_No;
		}

		friend bool
		operator==(Sphere_T<T> const & lhs, Sphere_T<T> const & rhs)
		{
			return (lhs.center_ == rhs.center_) && (rhs.radius_ == rhs.radius_);
		}

	private:
		Vector_T<T, 3> center_;
		T radius_;
	};
}

#endif			// _KFL_SPHERE_HPP
