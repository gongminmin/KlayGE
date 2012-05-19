// OBBox.hpp
// KlayGE OBB header file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2004-2012
// Homepage: http://www.klayge.org
//
// 4.1.0
// First release (2012.3.26)
//
// CHANGE LIST
///////////////////////////////////////////////////////////////////////////////

#ifndef _OBBOX_HPP
#define _OBBOX_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <boost/assert.hpp>
#include <boost/operators.hpp>

#include <KlayGE/Bound.hpp>

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

#endif		// _OBBOX_HPP
