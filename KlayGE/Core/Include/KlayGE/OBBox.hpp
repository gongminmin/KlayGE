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
			: r_(0, 0, 0)
		{
		}
		OBBox_T(AABBox_T<T> const & aabb)
		{
			center_ = aabb.Center();
			r_ = aabb.HalfSize();
			axis_[0] = Vector_T<T, 3>(1, 0, 0);
			axis_[1] = Vector_T<T, 3>(0, 1, 0);
			axis_[2] = Vector_T<T, 3>(0, 0, 1);
		}
		OBBox_T(Vector_T<T, 3> const & center,
			Vector_T<T, 3> const & x_axis, Vector_T<T, 3> const & y_axis, Vector_T<T, 3> const & z_axis,
			Vector_T<T, 3> const & r)
			: center_(center), r_(r)
		{
			axis_[0] = x_axis;
			axis_[1] = y_axis;
			axis_[2] = z_axis;
		}
		OBBox_T(OBBox_T<T> const & rhs)
			: Bound_T<T>(rhs),
				center_(rhs.center_), r_(rhs.r_)
		{
			axis_[0] = rhs.axis_[0];
			axis_[1] = rhs.axis_[1];
			axis_[2] = rhs.axis_[2];
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
			r_ *= rhs;
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
				axis_[0] = rhs.axis_[0];
				axis_[1] = rhs.axis_[1];
				axis_[2] = rhs.axis_[2];
				r_ = rhs.r_;
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
			ret.axis_[0] = -axis_[0];
			ret.axis_[1] = -axis_[1];
			ret.axis_[2] = -axis_[2];
			ret.r_ = r_;
			return ret;
		}

		bool IsEmpty() const
		{
			return MathLib::length_sq(r_) < T(1e-6);
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::intersect_point_obb(v, *this);
		}
		T MaxRadiusSq() const
		{
			return MathLib::length_sq(r_);
		}

		Vector_T<T, 3> const & Center() const
		{
			return center_;
		}
		Vector_T<T, 3> const & Axis(uint32_t index) const
		{
			return axis_[index];
		}
		Vector_T<T, 3> const & HalfSize() const
		{
			return r_;
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
				&& (lhs.axis_[0] == rhs.axis_[0])
				&& (lhs.axis_[1] == rhs.axis_[1])
				&& (lhs.axis_[2] == rhs.axis_[2])
				&& (rhs.r_ == rhs.r_);
		}

	private:
		Vector_T<T, 3> center_;
		Vector_T<T, 3> axis_[3];
		Vector_T<T, 3> r_;
	};
}

#endif		// _OBBOX_HPP
