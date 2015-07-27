/**
 * @file AABBox.cpp
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

#include <KFL/AABBox.hpp>

namespace KlayGE
{
	template AABBox_T<float>::AABBox_T(float3 const & vMin, float3 const & vMax) KLAYGE_NOEXCEPT;
	template AABBox_T<float>::AABBox_T(float3&& vMin, float3&& vMax) KLAYGE_NOEXCEPT;
	template AABBox_T<float>::AABBox_T(AABBox const & rhs) KLAYGE_NOEXCEPT;
	template AABBox_T<float>::AABBox_T(AABBox&& rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator+=(float3 const & rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator-=(float3 const & rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator*=(float rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator/=(float rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator&=(AABBox const & rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator|=(AABBox const & rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator=(AABBox const & rhs) KLAYGE_NOEXCEPT;
	template AABBox& AABBox_T<float>::operator=(AABBox&& rhs) KLAYGE_NOEXCEPT;
	template AABBox const AABBox_T<float>::operator+() const KLAYGE_NOEXCEPT;
	template AABBox const AABBox_T<float>::operator-() const KLAYGE_NOEXCEPT;
	template float AABBox_T<float>::Width() const KLAYGE_NOEXCEPT;
	template float AABBox_T<float>::Height() const KLAYGE_NOEXCEPT;
	template float AABBox_T<float>::Depth() const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::IsEmpty() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::LeftBottomNear() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::LeftTopNear() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::RightBottomNear() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::RightTopNear() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::LeftBottomFar() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::LeftTopFar() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::RightBottomFar() const KLAYGE_NOEXCEPT;
	template float3 const AABBox_T<float>::RightTopFar() const KLAYGE_NOEXCEPT;
	template float3 AABBox_T<float>::Center() const KLAYGE_NOEXCEPT;
	template float3 AABBox_T<float>::HalfSize() const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::VecInBound(float3 const & v) const KLAYGE_NOEXCEPT;
	template float AABBox_T<float>::MaxRadiusSq() const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::Intersect(AABBox const & aabb) const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::Intersect(OBBox const & obb) const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::Intersect(Sphere const & sphere) const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::Intersect(Frustum const & frustum) const KLAYGE_NOEXCEPT;
	template float3 AABBox_T<float>::Corner(size_t index) const KLAYGE_NOEXCEPT;
	template bool AABBox_T<float>::operator==(AABBox const & rhs) const KLAYGE_NOEXCEPT;


	template <typename T>
	AABBox_T<T>::AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax) KLAYGE_NOEXCEPT
				: min_(vMin), max_(vMax)
	{
		BOOST_ASSERT(vMin.x() <= vMax.x());
		BOOST_ASSERT(vMin.y() <= vMax.y());
		BOOST_ASSERT(vMin.z() <= vMax.z());
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(Vector_T<T, 3>&& vMin, Vector_T<T, 3>&& vMax) KLAYGE_NOEXCEPT
		: min_(std::move(vMin)), max_(std::move(vMax))
	{
		BOOST_ASSERT(vMin.x() <= vMax.x());
		BOOST_ASSERT(vMin.y() <= vMax.y());
		BOOST_ASSERT(vMin.z() <= vMax.z());
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT
			: Bound_T<T>(rhs),
				min_(rhs.min_), max_(rhs.max_)
	{
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(AABBox_T<T>&& rhs) KLAYGE_NOEXCEPT
		: Bound_T<T>(rhs),
			min_(std::move(rhs.min_)), max_(std::move(rhs.max_))
	{
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator+=(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT
	{
		min_ += rhs;
		max_ += rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator-=(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT
	{
		min_ -= rhs;
		max_ -= rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator*=(T rhs) KLAYGE_NOEXCEPT
	{
		this->Min() *= rhs;
		this->Max() *= rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator/=(T rhs) KLAYGE_NOEXCEPT
	{
		return this->operator*=(1.0f / rhs);
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator&=(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT
	{
		min_ = MathLib::maximize(this->Min(), rhs.Min());
		max_ = MathLib::minimize(this->Max(), rhs.Max());
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator|=(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT
	{
		min_ = MathLib::minimize(this->Min(), rhs.Min());
		max_ = MathLib::maximize(this->Max(), rhs.Max());
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT
	{
		if (this != &rhs)
		{
			this->Min() = rhs.Min();
			this->Max() = rhs.Max();
		}
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T>&& rhs) KLAYGE_NOEXCEPT
	{
		min_ = std::move(rhs.min_);
		max_ = std::move(rhs.max_);
		return *this;
	}

	template <typename T>
	AABBox_T<T> const AABBox_T<T>::operator+() const KLAYGE_NOEXCEPT
	{
		return *this;
	}

	template <typename T>
	AABBox_T<T> const AABBox_T<T>::operator-() const KLAYGE_NOEXCEPT
	{
		return AABBox_T<T>(-this->Max(), -this->Min());
	}

	template <typename T>
	T AABBox_T<T>::Width() const KLAYGE_NOEXCEPT
	{
		return this->Max().x() - this->Min().x();
	}

	template <typename T>
	T AABBox_T<T>::Height() const KLAYGE_NOEXCEPT
	{
		return this->Max().y() - this->Min().y();
	}

	template <typename T>
	T AABBox_T<T>::Depth() const KLAYGE_NOEXCEPT
	{
		return this->Max().z() - this->Min().z();
	}

	template <typename T>
	bool AABBox_T<T>::IsEmpty() const KLAYGE_NOEXCEPT
	{
		return this->Min() == this->Max();
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftBottomNear() const KLAYGE_NOEXCEPT
	{
		return this->Min();
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftTopNear() const KLAYGE_NOEXCEPT
	{
		return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Min().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightBottomNear() const KLAYGE_NOEXCEPT
	{
		return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Min().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightTopNear() const KLAYGE_NOEXCEPT
	{
		return Vector_T<T, 3>(this->Max().x(), this->Max().y(), this->Min().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftBottomFar() const KLAYGE_NOEXCEPT
	{
		return Vector_T<T, 3>(this->Min().x(), this->Min().y(), this->Max().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftTopFar() const KLAYGE_NOEXCEPT
	{
		return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Max().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightBottomFar() const KLAYGE_NOEXCEPT
	{
		return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Max().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightTopFar() const KLAYGE_NOEXCEPT
	{
		return this->Max();
	}

	template <typename T>
	Vector_T<T, 3> AABBox_T<T>::Center() const KLAYGE_NOEXCEPT
	{
		return (min_ + max_) / 2.0f;
	}

	template <typename T>
	Vector_T<T, 3> AABBox_T<T>::HalfSize() const KLAYGE_NOEXCEPT
	{
		return (max_ - min_) / 2.0f;
	}

	template <typename T>
	bool AABBox_T<T>::VecInBound(Vector_T<T, 3> const & v) const KLAYGE_NOEXCEPT
	{
		return MathLib::intersect_point_aabb(v, *this);
	}

	template <typename T>
	T AABBox_T<T>::MaxRadiusSq() const KLAYGE_NOEXCEPT
	{
		return std::max<T>(MathLib::length_sq(this->Max()), MathLib::length_sq(this->Min()));
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(AABBox_T<T> const & aabb) const KLAYGE_NOEXCEPT
	{
		return MathLib::intersect_aabb_aabb(*this, aabb);
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(OBBox_T<T> const & obb) const KLAYGE_NOEXCEPT
	{
		return MathLib::intersect_aabb_obb(*this, obb);
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(Sphere_T<T> const & sphere) const KLAYGE_NOEXCEPT
	{
		return MathLib::intersect_aabb_sphere(*this, sphere);
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(Frustum_T<T> const & frustum) const KLAYGE_NOEXCEPT
	{
		return MathLib::intersect_aabb_frustum(*this, frustum) != BO_No;
	}

	template <typename T>
	Vector_T<T, 3> AABBox_T<T>::Corner(size_t index) const KLAYGE_NOEXCEPT
	{
		BOOST_ASSERT(index < 8);

		return Vector_T<T, 3>((index & 1UL) ? this->Max().x() : this->Min().x(),
			(index & 2UL) ? this->Max().y() : this->Min().y(),
			(index & 4UL) ? this->Max().z() : this->Min().z());
	}

	template <typename T>
	bool AABBox_T<T>::operator==(AABBox_T<T> const & rhs) const KLAYGE_NOEXCEPT
	{
		return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
	}
}
