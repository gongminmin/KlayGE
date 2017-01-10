/**
 * @file OBBox.cpp
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

#include <KFL/KFL.hpp>

#include <boost/assert.hpp>

#include <KFL/OBBox.hpp>

namespace KlayGE
{
	template OBBox_T<float>::OBBox_T() noexcept;
	template OBBox_T<float>::OBBox_T(float3 const & center,
		float3 const & x_axis, float3 const & y_axis, float3 const & z_axis,
		float3 const & extent) noexcept;
	template OBBox_T<float>::OBBox_T(float3 const & center,
		Quaternion const & rotation,
		float3 const & extent) noexcept;
	template OBBox_T<float>::OBBox_T(float3&& center,
		Quaternion&& rotation,
		float3&& extent) noexcept;
	template OBBox_T<float>::OBBox_T(OBBox const & rhs) noexcept;
	template OBBox_T<float>::OBBox_T(OBBox&& rhs) noexcept;
	template OBBox& OBBox_T<float>::operator+=(float3 const & rhs) noexcept;
	template OBBox& OBBox_T<float>::operator-=(float3 const & rhs) noexcept;
	template OBBox& OBBox_T<float>::operator*=(float rhs) noexcept;
	template OBBox& OBBox_T<float>::operator/=(float rhs) noexcept;
	template OBBox& OBBox_T<float>::operator=(OBBox const & rhs) noexcept;
	template OBBox& OBBox_T<float>::operator=(OBBox&& rhs) noexcept;
	template OBBox const OBBox_T<float>::operator+() const noexcept;
	template OBBox const OBBox_T<float>::operator-() const noexcept;
	template bool OBBox_T<float>::IsEmpty() const noexcept;
	template bool OBBox_T<float>::VecInBound(float3 const & v) const noexcept;
	template float OBBox_T<float>::MaxRadiusSq() const noexcept;
	template float3 OBBox_T<float>::Axis(uint32_t index) const noexcept;
	template bool OBBox_T<float>::Intersect(AABBox const & aabb) const noexcept;
	template bool OBBox_T<float>::Intersect(OBBox const & obb) const noexcept;
	template bool OBBox_T<float>::Intersect(Sphere const & sphere) const noexcept;
	template bool OBBox_T<float>::Intersect(Frustum const & frustum) const noexcept;
	template float3 OBBox_T<float>::Corner(uint32_t index) const noexcept;
	template bool OBBox_T<float>::operator==(OBBox const & rhs) const noexcept;


	template <typename T>
	OBBox_T<T>::OBBox_T() noexcept
		: extent_(0, 0, 0)
	{
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(Vector_T<T, 3> const & center,
		Vector_T<T, 3> const & x_axis, Vector_T<T, 3> const & y_axis, Vector_T<T, 3> const & z_axis,
		Vector_T<T, 3> const & extent) noexcept
		: center_(center), extent_(extent)
	{
		rotation_ = MathLib::to_quaternion(x_axis, y_axis, z_axis, 0);
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(Vector_T<T, 3> const & center,
		Quaternion_T<T> const & rotation,
		Vector_T<T, 3> const & extent) noexcept
		: center_(center), rotation_(rotation), extent_(extent)
	{
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(Vector_T<T, 3>&& center,
		Quaternion_T<T>&& rotation,
		Vector_T<T, 3>&& extent) noexcept
		: center_(std::move(center)), rotation_(std::move(rotation)), extent_(std::move(extent))
	{
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(OBBox_T<T> const & rhs) noexcept
		: Bound_T<T>(rhs),
			center_(rhs.center_), rotation_(rhs.rotation_), extent_(rhs.extent_)
	{
	}

	template <typename T>
	OBBox_T<T>::OBBox_T(OBBox_T<T>&& rhs) noexcept
		: Bound_T<T>(rhs),
			center_(std::move(rhs.center_)), rotation_(std::move(rhs.rotation_)), extent_(std::move(rhs.extent_))
	{
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator+=(Vector_T<T, 3> const & rhs) noexcept
	{
		center_ += rhs;
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator-=(Vector_T<T, 3> const & rhs) noexcept
	{
		center_ -= rhs;
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator*=(T rhs) noexcept
	{
		extent_ *= rhs;
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator/=(T rhs) noexcept
	{
		return this->operator*=(1.0f / rhs);
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator=(OBBox_T<T> const & rhs) noexcept
	{
		if (this != &rhs)
		{
			center_ = rhs.center_;
			rotation_ = rhs.rotation_;
			extent_ = rhs.extent_;
		}
		return *this;
	}

	template <typename T>
	OBBox_T<T>& OBBox_T<T>::operator=(OBBox_T<T>&& rhs) noexcept
	{
		center_ = std::move(rhs.center_);
		rotation_ = std::move(rhs.rotation_);
		extent_ = std::move(rhs.extent_);
		return *this;
	}

	template <typename T>
	OBBox_T<T> const OBBox_T<T>::operator+() const noexcept
	{
		return *this;
	}

	template <typename T>
	OBBox_T<T> const OBBox_T<T>::operator-() const noexcept
	{
		OBBox_T<T> ret;
		ret.center_ = -center_;
		ret.rotation_ = -rotation_;
		ret.extent_ = extent_;
		return ret;
	}

	template <typename T>
	bool OBBox_T<T>::IsEmpty() const noexcept
	{
		return MathLib::length_sq(extent_) < T(1e-6);
	}

	template <typename T>
	bool OBBox_T<T>::VecInBound(Vector_T<T, 3> const & v) const noexcept
	{
		return MathLib::intersect_point_obb(v, *this);
	}

	template <typename T>
	T OBBox_T<T>::MaxRadiusSq() const noexcept
	{
		return MathLib::length_sq(extent_);
	}

	template <typename T>
	Vector_T<T, 3> OBBox_T<T>::Axis(uint32_t index) const noexcept
	{
		Vector_T<T, 3> v(0, 0, 0);
		v[index] = 1;
		return MathLib::transform_quat(v, rotation_);
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(AABBox_T<T> const & aabb) const noexcept
	{
		return MathLib::intersect_aabb_obb(aabb, *this);
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(OBBox_T<T> const & obb) const noexcept
	{
		return MathLib::intersect_obb_obb(*this, obb);
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(Sphere_T<T> const & sphere) const noexcept
	{
		return MathLib::intersect_obb_sphere(*this, sphere);
	}

	template <typename T>
	bool OBBox_T<T>::Intersect(Frustum_T<T> const & frustum) const noexcept
	{
		return MathLib::intersect_obb_frustum(*this, frustum) != BO_No;
	}

	template <typename T>
	Vector_T<T, 3> OBBox_T<T>::Corner(uint32_t index) const noexcept
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

	template <typename T>
	bool OBBox_T<T>::operator==(OBBox_T<T> const & rhs) const noexcept
	{
		return (center_ == rhs.center_)
			&& (rotation_ == rhs.rotation_)
			&& (extent_ == rhs.extent_);
	}
}
