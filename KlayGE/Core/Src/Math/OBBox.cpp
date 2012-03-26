// OBBox.cpp
// KlayGE OBB implement file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2012
// Homepage: http://www.klayge.org
//
// 4.1.0
// First release (2012.3.26)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OBBox.hpp>

namespace KlayGE
{
	template class KLAYGE_CORE_API OBBox_T<float>;

	template <typename T>
	OBBox_T<T>::OBBox_T()
		: r_(0, 0, 0)
	{
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(AABBox_T<T> const & aabb)
	{
		center_ = aabb.Center();
		r_ = aabb.HalfSize();
		axis_[0] = Vector_T<T, 3>(1, 0, 0);
		axis_[1] = Vector_T<T, 3>(0, 1, 0);
		axis_[2] = Vector_T<T, 3>(0, 0, 1);
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(Vector_T<T, 3> const & center,
			Vector_T<T, 3> const & x_axis, Vector_T<T, 3> const & y_axis, Vector_T<T, 3> const & z_axis,
			Vector_T<T, 3> const & r)
		: center_(center), r_(r)
	{
		axis_[0] = x_axis;
		axis_[1] = y_axis;
		axis_[2] = z_axis;
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(OBBox_T<T> const & rhs)
		: Bound_T<T>(rhs),
			center_(rhs.center_), r_(rhs.r_)
	{
		axis_[0] = rhs.axis_[0];
		axis_[1] = rhs.axis_[1];
		axis_[2] = rhs.axis_[2];
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator+=(Vector_T<T, 3> const & rhs)
	{
		center_ += rhs;
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator-=(Vector_T<T, 3> const & rhs)
	{
		center_ -= rhs;
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator*=(T const & rhs)
	{
		r_ *= rhs;
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator/=(T const & rhs)
	{
		return this->operator*=(1.0f / rhs);
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator=(OBBox_T<T> const & rhs)
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

	template <typename T>
	OBBox_T<T> const OBBox_T<T>::operator+() const
	{
		return *this;
	}

	template <typename T>
	OBBox_T<T> const OBBox_T<T>::operator-() const
	{
		OBBox_T<T> ret;
		ret.center_ = -center_;
		ret.axis_[0] = -axis_[0];
		ret.axis_[1] = -axis_[1];
		ret.axis_[2] = -axis_[2];
		ret.r_ = r_;
		return ret;
	}

	template <typename T>
	bool OBBox_T<T>::IsEmpty() const
	{
		return MathLib::length_sq(r_) < T(1e-6);
	}

	template <typename T>
	bool OBBox_T<T>::VecInBound(Vector_T<T, 3> const & v) const
	{
		Vector_T<T, 3> d = v - center_;
		if ((MathLib::dot(d, axis_[0]) <= r_[0])
			&& (MathLib::dot(d, axis_[1]) <= r_[1])
			&& (MathLib::dot(d, axis_[2]) <= r_[2]))
		{
			return true;
		}
		return false;
	}

	template <typename T>
	T OBBox_T<T>::MaxRadiusSq() const
	{
		return MathLib::length_sq(r_);
	}

	template <typename T>
	Vector_T<T, 3> const & OBBox_T<T>::Center() const
	{
		return center_;
	}

	template <typename T>
	Vector_T<T, 3> const & OBBox_T<T>::Axis(uint32_t index) const
	{
		return axis_[index];
	}
	
	template <typename T>
	Vector_T<T, 3> const & OBBox_T<T>::HalfSize() const
	{
		return r_;
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(AABBox_T<T> const & aabb) const
	{
		return aabb.Intersect(*this);
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(OBBox_T<T> const & obb) const
	{
		// From Real-Time Collision Detection, p. 101-106. See http://realtimecollisiondetection.net/

		T epsilon = T(1e-3);

		Matrix4_T<T> r_mat = Matrix4_T<T>::Identity();
		for (int i = 0; i < 3; ++ i)
		{
			for (int j = 0; j < 3; ++ j)
			{
				r_mat(i, j) = MathLib::dot(axis_[i], obb.axis_[j]);
			}
		}

		Vector_T<T, 3> t = obb.center_ - center_;
		t = Vector_T<T, 3>(MathLib::dot(t, axis_[0]), MathLib::dot(t, axis_[1]), MathLib::dot(t, axis_[2]));

		Matrix4_T<T> abs_r_mat = Matrix4_T<T>::Identity();
		for (int i = 0; i < 3; ++ i)
		{
			for (int j = 0; j < 3; ++ j)
			{
				abs_r_mat(i, j) = MathLib::abs(r_mat(i, j)) + epsilon;
			}
		}

		// Test the three major axes of this OBB.
		for (int i = 0; i < 3; ++ i)
		{
			T ra = r_[i];
			T rb = obb.r_[0] * abs_r_mat(i, 0) +  obb.r_[1] * abs_r_mat(i, 1) + obb.r_[2] * abs_r_mat(i, 2);
			if (MathLib::abs(t[i]) > ra + rb) 
			{
				return false;
			}
		}

		// Test the three major axes of the OBB b.
		for (int i = 0; i < 3; ++ i)
		{
			T ra = r_[0] * abs_r_mat(0, i) + r_[1] * abs_r_mat(1, i) + r_[2] * abs_r_mat(2, i);
			T rb = obb.r_[i];
			if (MathLib::abs(t.x() + r_mat(0, i) + t.y() * r_mat(1, i) + t.z() * r_mat(2, i)) > ra + rb)
			{
				return false;
			}
		}

		// Test the 9 different cross-axes.

		// A.x <cross> B.x
		T ra = r_.y() * abs_r_mat(2, 0) + r_.z() * abs_r_mat(1, 0);
		T rb = obb.r_.y() * abs_r_mat(0, 2) + obb.r_.z() * abs_r_mat(0, 1);
		if (MathLib::abs(t.z() * r_mat(1, 0) - t.y() * r_mat(2, 0)) > ra + rb)
		{
			return false;
		}

		// A.x < cross> B.y
		ra = r_.y() * abs_r_mat(2, 1) + r_.z() * abs_r_mat(1, 1);
		rb = obb.r_.x() * abs_r_mat(0, 2) + obb.r_.z() * abs_r_mat(0, 0);
		if (MathLib::abs(t.z() * r_mat(1, 1) - t.y() * r_mat(2, 1)) > ra + rb)
		{
			return false;
		}

		// A.x <cross> B.z
		ra = r_.y() * abs_r_mat(2, 2) + r_.z() * abs_r_mat(1, 2);
		rb = obb.r_.x() * abs_r_mat(0, 1) + obb.r_.y() * abs_r_mat(0, 0);
		if (MathLib::abs(t.z() * r_mat(1, 2) - t.y() * r_mat(2, 2)) > ra + rb)
		{
			return false;
		}

		// A.y <cross> B.x
		ra = r_.x() * abs_r_mat(2, 0) + r_.z() * abs_r_mat(0, 0);
		rb = obb.r_.y() * abs_r_mat(1, 2) + obb.r_.z() * abs_r_mat(1, 1);
		if (MathLib::abs(t.x() * r_mat(2, 0) - t.z() * r_mat(0, 0)) > ra + rb)
		{
			return false;
		}

		// A.y <cross> B.y
		ra = r_.x() * abs_r_mat(2, 1) + r_.z() * abs_r_mat(0, 1);
		rb = obb.r_.x() * abs_r_mat(1, 2) + obb.r_.z() * abs_r_mat(1, 0);
		if (MathLib::abs(t.x() * r_mat(2, 1) - t.z() * r_mat(0, 1)) > ra + rb)
		{
			return false;
		}

		// A.y <cross> B.z
		ra = r_.x() * abs_r_mat(2, 2) + r_.z() * abs_r_mat(0, 2);
		rb = obb.r_.x() * abs_r_mat(1, 1) + obb.r_.y() * abs_r_mat(1, 0);
		if (MathLib::abs(t.x() * r_mat(2, 2) - t.z() * r_mat(0, 2)) > ra + rb)
		{
			return false;
		}

		// A.z <cross> B.x
		ra = r_.x() * abs_r_mat(1, 0) + r_.y() * abs_r_mat(0, 0);
		rb = obb.r_.y() * abs_r_mat(2, 2) + obb.r_.z() * abs_r_mat(2, 1);
		if (MathLib::abs(t.y() * r_mat(0, 0) - t.x() * r_mat(1, 0)) > ra + rb)
		{
			return false;
		}

		// A.z <cross> B.y
		ra = r_.x() * abs_r_mat(1, 1) + r_.y() * abs_r_mat(0, 1);
		rb = obb.r_.x() * abs_r_mat(2, 2) + obb.r_.z() * abs_r_mat(2, 0);
		if (MathLib::abs(t.y() * r_mat(0, 1) - t.x() * r_mat(1, 1)) > ra + rb)
		{
			return false;
		}

		// A.z <cross> B.z
		ra = r_.x() * abs_r_mat(1, 2) + r_.y() * abs_r_mat(0, 2);
		rb = obb.r_.x() * abs_r_mat(2, 1) + obb.r_.y() * abs_r_mat(2, 0);
		if (MathLib::abs(t.y() * r_mat(0, 2) - t.x() * r_mat(1, 2)) > ra + rb)
		{
			return false;
		}

		return true;
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(Sphere_T<T> const & sphere) const
	{
		Vector_T<T, 3> d = sphere.Center() - center_;
		Vector_T<T, 3> closest_point_on_obb = center_;
		for (int i = 0; i < 3; ++ i)
		{
			T dist = MathLib::dot(d, axis_[i]);
			if (dist > r_[i])
			{
				dist = r_[i];
			}
			if (dist < -r_[i])
			{
				dist = -r_[i];
			}
			closest_point_on_obb += dist * axis_[i];
		}

		Vector_T<T, 3> v = closest_point_on_obb - sphere.Center();
		return MathLib::length_sq(v) <= sphere.Radius() * sphere.Radius();
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(Frustum_T<T> const & frustum) const
	{
		return frustum.Intersect(*this) != BO_No;
	}
}
