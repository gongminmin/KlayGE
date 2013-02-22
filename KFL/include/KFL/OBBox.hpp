/**
 * @file OBBox.hpp
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

#ifndef _KFL_OBBOX_HPP
#define _KFL_OBBOX_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <boost/assert.hpp>
#include <boost/operators.hpp>

#include <KFL/Bound.hpp>

namespace KlayGE
{
	template <typename T>
	class OBBox_T : boost::addable2<OBBox_T<T>, Vector_T<T, 3>,
						boost::subtractable2<OBBox_T<T>, Vector_T<T, 3>,
						boost::multipliable2<OBBox_T<T>, T,
						boost::dividable2<OBBox_T<T>, T,
						boost::equality_comparable<OBBox_T<T> > > > > >,
				public Bound_T<T>
	{
	public:
		OBBox_T()
			: extent_(0, 0, 0)
		{
		}
		OBBox_T(Vector_T<T, 3> const & center,
			Vector_T<T, 3> const & x_axis, Vector_T<T, 3> const & y_axis, Vector_T<T, 3> const & z_axis,
			Vector_T<T, 3> const & extent)
			: center_(center), extent_(extent)
		{
			UNREF_PARAM(x_axis);
			UNREF_PARAM(y_axis);
			rotation_ = MathLib::unit_axis_to_unit_axis(Vector_T<T, 3>(0, 0, 1), MathLib::normalize(z_axis));
		}
		OBBox_T(Vector_T<T, 3> const & center,
			Quaternion_T<T> const & rotation,
			Vector_T<T, 3> const & extent)
			: center_(center), rotation_(rotation), extent_(extent)
		{
		}
		OBBox_T(OBBox_T<T> const & rhs)
			: Bound_T<T>(rhs),
				center_(rhs.center_), rotation_(rhs.rotation_), extent_(rhs.extent_)
		{
		}

		OBBox_T<T>& operator+=(Vector_T<T, 3> const & rhs)
		{
			center_ += rhs;
			return *this;
		}
		OBBox_T<T>& operator-=(Vector_T<T, 3> const & rhs)
		{
			center_ -= rhs;
			return *this;
		}
		OBBox_T<T>& operator*=(T const & rhs)
		{
			extent_ *= rhs;
			return *this;
		}
		OBBox_T<T>& operator/=(T const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}

		OBBox_T<T>& operator=(OBBox_T<T> const & rhs)
		{
			if (this != &rhs)
			{
				center_ = rhs.center_;
				rotation_ = rhs.rotation_;
				extent_ = rhs.extent_;
			}
			return *this;
		}

		OBBox_T<T> const operator+() const
		{
			return *this;
		}
		OBBox_T<T> const operator-() const
		{
			OBBox_T<T> ret;
			ret.center_ = -center_;
			ret.rotation_ = -rotation_;
			ret.extent_ = extent_;
			return ret;
		}

		bool IsEmpty() const
		{
			return MathLib::length_sq(extent_) < T(1e-6);
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::intersect_point_obb(v, *this);
		}
		T MaxRadiusSq() const
		{
			return MathLib::length_sq(extent_);
		}

		Vector_T<T, 3> const & Center() const
		{
			return center_;
		}
		Quaternion_T<T> const & Rotation() const
		{
			return rotation_;
		}
		Vector_T<T, 3> Axis(uint32_t index) const
		{
			Vector_T<T, 3> v(0, 0, 0);
			v[index] = 1;
			return MathLib::transform_quat(v, rotation_);
		}
		Vector_T<T, 3> const & HalfSize() const
		{
			return extent_;
		}

		bool Intersect(AABBox_T<T> const & aabb) const
		{
			return MathLib::intersect_aabb_obb(aabb, *this);
		}
		bool Intersect(OBBox_T<T> const & obb) const
		{
			return MathLib::intersect_obb_obb(*this, obb);
		}
		bool Intersect(Sphere_T<T> const & sphere) const
		{
			return MathLib::intersect_obb_sphere(*this, sphere);
		}
		bool Intersect(Frustum_T<T> const & frustum) const
		{
			return MathLib::intersect_obb_frustum(*this, frustum) != BO_No;
		}

		Vector_T<T, 3> Corner(uint32_t index) const
		{
			BOOST_ASSERT(index < 8);

			float3 const & center = this->Center();
			float3 const & extent = this->HalfSize();
			float3 const extent_x = MathLib::abs(extent.x() * this->Axis(0));
			float3 const extent_y = MathLib::abs(extent.y() * this->Axis(1));
			float3 const extent_z = MathLib::abs(extent.z() * this->Axis(2));

			return center + ((index & 1UL) ? +extent_x : -extent_x)
				+ ((index & 2UL) ? +extent_y : -extent_y)
				+ ((index & 4UL) ? +extent_z : -extent_z);
		}

		friend bool
		operator==(OBBox_T<T> const & lhs, OBBox_T<T> const & rhs)
		{
			return (lhs.center_ == rhs.center_)
				&& (lhs.rotation_ == rhs.rotation_)
				&& (rhs.extent_ == rhs.extent_);
		}

	private:
		Vector_T<T, 3> center_;
		Quaternion_T<T> rotation_;
		Vector_T<T, 3> extent_;
	};
}

#endif		// _KFL_OBBOX_HPP
