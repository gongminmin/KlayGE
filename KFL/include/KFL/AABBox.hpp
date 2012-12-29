/**
 * @file AABBox.hpp
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

#ifndef _KFL_AABBOX_HPP
#define _KFL_AABBOX_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <boost/assert.hpp>
#include <boost/operators.hpp>

#include <KFL/Bound.hpp>

namespace KlayGE
{
	template <typename T>
	class AABBox_T : boost::addable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::subtractable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::multipliable2<AABBox_T<T>, T,
						boost::dividable2<AABBox_T<T>, T,
						boost::andable<AABBox_T<T>,
						boost::orable<AABBox_T<T>,
						boost::equality_comparable<AABBox_T<T> > > > > > > >,
				public Bound_T<T>
	{
	public:
		AABBox_T()
		{
		}
		AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax)
				: min_(vMin), max_(vMax)
		{
			BOOST_ASSERT(vMin.x() <= vMax.x());
			BOOST_ASSERT(vMin.y() <= vMax.y());
			BOOST_ASSERT(vMin.z() <= vMax.z());
		}
		AABBox_T(AABBox_T<T> const & rhs)
				: Bound_T<T>(rhs),
				min_(rhs.min_), max_(rhs.max_)
		{
		}

		// 赋值操作符
		AABBox_T<T>& operator+=(Vector_T<T, 3> const & rhs)
		{
			min_ += rhs;
			max_ += rhs;
			return *this;
		}
		AABBox_T<T>& operator-=(Vector_T<T, 3> const & rhs)
		{
			min_ -= rhs;
			max_ -= rhs;
			return *this;
		}
		AABBox_T<T>& operator*=(T const & rhs)
		{
			this->Min() *= rhs;
			this->Max() *= rhs;
			return *this;
		}
		AABBox_T<T>& operator/=(T const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}
		AABBox_T<T>& operator&=(AABBox_T<T> const & rhs)
		{
			min_ = MathLib::maximize(this->Min(), rhs.Min());
			max_ = MathLib::minimize(this->Max(), rhs.Max());
			return *this;
		}
		AABBox_T<T>& operator|=(AABBox_T<T> const & rhs)
		{
			min_ = MathLib::minimize(this->Min(), rhs.Min());
			max_ = MathLib::maximize(this->Max(), rhs.Max());
			return *this;
		}

		AABBox_T<T>& operator=(AABBox_T<T> const & rhs)
		{
			if (this != &rhs)
			{
				this->Min() = rhs.Min();
				this->Max() = rhs.Max();
			}
			return *this;
		}

		// 一元操作符
		AABBox_T<T> const operator+() const
		{
			return *this;
		}
		AABBox_T<T> const operator-() const
		{
			return AABBox_T<T>(-this->Max(), -this->Min());
		}

		Vector_T<T, 3> operator[](size_t i) const
		{
			BOOST_ASSERT(i < 8);

			return Vector_T<T, 3>((i & 1UL) ? this->Max().x() : this->Min().x(),
				(i & 2UL) ? this->Max().y() : this->Min().y(),
				(i & 4UL) ? this->Max().z() : this->Min().z());
		}

		// 属性
		T Width() const
		{
			return this->Max().x() - this->Min().x();
		}
		T Height() const
		{
			return this->Max().y() - this->Min().y();
		}
		T Depth() const
		{
			return this->Max().z() - this->Min().z();
		}
		bool IsEmpty() const
		{
			return this->Min() == this->Max();
		}

		Vector_T<T, 3> const LeftBottomNear() const
		{
			return this->Min();
		}
		Vector_T<T, 3> const LeftTopNear() const
		{
			return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Min().z());
		}
		Vector_T<T, 3> const RightBottomNear() const
		{
			return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Min().z());
		}
		Vector_T<T, 3> const RightTopNear() const
		{
			return Vector_T<T, 3>(this->Max().x(), this->Max().y(), this->Min().z());
		}
		Vector_T<T, 3> const LeftBottomFar() const
		{
			return Vector_T<T, 3>(this->Min().x(), this->Min().y(), this->Max().z());
		}
		Vector_T<T, 3> const LeftTopFar() const
		{
			return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Max().z());
		}
		Vector_T<T, 3> const RightBottomFar() const
		{
			return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Max().z());
		}
		Vector_T<T, 3> const RightTopFar() const
		{
			return this->Max();
		}

		Vector_T<T, 3>& Min()
		{
			return min_;
		}
		Vector_T<T, 3> const & Min() const
		{
			return min_;
		}
		Vector_T<T, 3>& Max()
		{
			return max_;
		}
		Vector_T<T, 3> const & Max() const
		{
			return max_;
		}
		Vector_T<T, 3> Center() const
		{
			return (min_ + max_) / 2.0f;
		}
		Vector_T<T, 3> HalfSize() const
		{
			return (max_ - min_) / 2.0f;
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::intersect_point_aabb(v, *this);
		}
		T MaxRadiusSq() const
		{
			return std::max<T>(MathLib::length_sq(this->Max()), MathLib::length_sq(this->Min()));
		}

		bool Intersect(AABBox_T<T> const & aabb) const
		{
			return MathLib::intersect_aabb_aabb(*this, aabb);
		}
		bool Intersect(OBBox_T<T> const & obb) const
		{
			return MathLib::intersect_aabb_obb(*this, obb);
		}
		bool Intersect(Sphere_T<T> const & sphere) const
		{
			return MathLib::intersect_aabb_sphere(*this, sphere);
		}
		bool Intersect(Frustum_T<T> const & frustum) const
		{
			return MathLib::intersect_aabb_frustum(*this, frustum) != BO_No;
		}

		friend bool
		operator==(AABBox_T<T> const & lhs, AABBox_T<T> const & rhs)
		{
			return (lhs.Min() == rhs.Min()) && (rhs.Max() == rhs.Max());
		}

	private:
		Vector_T<T, 3> min_, max_;
	};
}

#endif			// _KFL_AABBOX_HPP
