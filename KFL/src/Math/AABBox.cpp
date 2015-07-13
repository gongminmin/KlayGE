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
	template AABBox_T<float>::AABBox_T(float3 const & vMin, float3 const & vMax);
	template AABBox_T<float>::AABBox_T(float3&& vMin, float3&& vMax);
	template AABBox_T<float>::AABBox_T(AABBox const & rhs);
	template AABBox_T<float>::AABBox_T(AABBox&& rhs);
	template AABBox& AABBox_T<float>::operator+=(float3 const & rhs);
	template AABBox& AABBox_T<float>::operator-=(float3 const & rhs);
	template AABBox& AABBox_T<float>::operator*=(float rhs);
	template AABBox& AABBox_T<float>::operator/=(float rhs);
	template AABBox& AABBox_T<float>::operator&=(AABBox const & rhs);
	template AABBox& AABBox_T<float>::operator|=(AABBox const & rhs);
	template AABBox& AABBox_T<float>::operator=(AABBox const & rhs);
	template AABBox& AABBox_T<float>::operator=(AABBox&& rhs);
	template AABBox const AABBox_T<float>::operator+() const;
	template AABBox const AABBox_T<float>::operator-() const;
	template float AABBox_T<float>::Width() const;
	template float AABBox_T<float>::Height() const;
	template float AABBox_T<float>::Depth() const;
	template bool AABBox_T<float>::IsEmpty() const;
	template float3 const AABBox_T<float>::LeftBottomNear() const;
	template float3 const AABBox_T<float>::LeftTopNear() const;
	template float3 const AABBox_T<float>::RightBottomNear() const;
	template float3 const AABBox_T<float>::RightTopNear() const;
	template float3 const AABBox_T<float>::LeftBottomFar() const;
	template float3 const AABBox_T<float>::LeftTopFar() const;
	template float3 const AABBox_T<float>::RightBottomFar() const;
	template float3 const AABBox_T<float>::RightTopFar() const;
	template float3 AABBox_T<float>::Center() const;
	template float3 AABBox_T<float>::HalfSize() const;
	template bool AABBox_T<float>::VecInBound(float3 const & v) const;
	template float AABBox_T<float>::MaxRadiusSq() const;
	template bool AABBox_T<float>::Intersect(AABBox const & aabb) const;
	template bool AABBox_T<float>::Intersect(OBBox const & obb) const;
	template bool AABBox_T<float>::Intersect(Sphere const & sphere) const;
	template bool AABBox_T<float>::Intersect(Frustum const & frustum) const;
	template float3 AABBox_T<float>::Corner(size_t index) const;
	template bool AABBox_T<float>::operator==(AABBox const & rhs) const;


	template <typename T>
	AABBox_T<T>::AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax)
				: min_(vMin), max_(vMax)
	{
		BOOST_ASSERT(vMin.x() <= vMax.x());
		BOOST_ASSERT(vMin.y() <= vMax.y());
		BOOST_ASSERT(vMin.z() <= vMax.z());
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(Vector_T<T, 3>&& vMin, Vector_T<T, 3>&& vMax)
		: min_(std::move(vMin)), max_(std::move(vMax))
	{
		BOOST_ASSERT(vMin.x() <= vMax.x());
		BOOST_ASSERT(vMin.y() <= vMax.y());
		BOOST_ASSERT(vMin.z() <= vMax.z());
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(AABBox_T<T> const & rhs)
			: Bound_T<T>(rhs),
				min_(rhs.min_), max_(rhs.max_)
	{
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(AABBox_T<T>&& rhs)
		: Bound_T<T>(rhs),
			min_(std::move(rhs.min_)), max_(std::move(rhs.max_))
	{
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator+=(Vector_T<T, 3> const & rhs)
	{
		min_ += rhs;
		max_ += rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator-=(Vector_T<T, 3> const & rhs)
	{
		min_ -= rhs;
		max_ -= rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator*=(T rhs)
	{
		this->Min() *= rhs;
		this->Max() *= rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator/=(T rhs)
	{
		return this->operator*=(1.0f / rhs);
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator&=(AABBox_T<T> const & rhs)
	{
		min_ = MathLib::maximize(this->Min(), rhs.Min());
		max_ = MathLib::minimize(this->Max(), rhs.Max());
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator|=(AABBox_T<T> const & rhs)
	{
		min_ = MathLib::minimize(this->Min(), rhs.Min());
		max_ = MathLib::maximize(this->Max(), rhs.Max());
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T> const & rhs)
	{
		if (this != &rhs)
		{
			this->Min() = rhs.Min();
			this->Max() = rhs.Max();
		}
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T>&& rhs)
	{
		min_ = std::move(rhs.min_);
		max_ = std::move(rhs.max_);
		return *this;
	}

	template <typename T>
	AABBox_T<T> const AABBox_T<T>::operator+() const
	{
		return *this;
	}

	template <typename T>
	AABBox_T<T> const AABBox_T<T>::operator-() const
	{
		return AABBox_T<T>(-this->Max(), -this->Min());
	}

	template <typename T>
	T AABBox_T<T>::Width() const
	{
		return this->Max().x() - this->Min().x();
	}

	template <typename T>
	T AABBox_T<T>::Height() const
	{
		return this->Max().y() - this->Min().y();
	}

	template <typename T>
	T AABBox_T<T>::Depth() const
	{
		return this->Max().z() - this->Min().z();
	}

	template <typename T>
	bool AABBox_T<T>::IsEmpty() const
	{
		return this->Min() == this->Max();
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftBottomNear() const
	{
		return this->Min();
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftTopNear() const
	{
		return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Min().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightBottomNear() const
	{
		return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Min().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightTopNear() const
	{
		return Vector_T<T, 3>(this->Max().x(), this->Max().y(), this->Min().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftBottomFar() const
	{
		return Vector_T<T, 3>(this->Min().x(), this->Min().y(), this->Max().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::LeftTopFar() const
	{
		return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Max().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightBottomFar() const
	{
		return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Max().z());
	}

	template <typename T>
	Vector_T<T, 3> const AABBox_T<T>::RightTopFar() const
	{
		return this->Max();
	}

	template <typename T>
	Vector_T<T, 3> AABBox_T<T>::Center() const
	{
		return (min_ + max_) / 2.0f;
	}

	template <typename T>
	Vector_T<T, 3> AABBox_T<T>::HalfSize() const
	{
		return (max_ - min_) / 2.0f;
	}

	template <typename T>
	bool AABBox_T<T>::VecInBound(Vector_T<T, 3> const & v) const
	{
		return MathLib::intersect_point_aabb(v, *this);
	}

	template <typename T>
	T AABBox_T<T>::MaxRadiusSq() const
	{
		return std::max<T>(MathLib::length_sq(this->Max()), MathLib::length_sq(this->Min()));
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(AABBox_T<T> const & aabb) const
	{
		return MathLib::intersect_aabb_aabb(*this, aabb);
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(OBBox_T<T> const & obb) const
	{
		return MathLib::intersect_aabb_obb(*this, obb);
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(Sphere_T<T> const & sphere) const
	{
		return MathLib::intersect_aabb_sphere(*this, sphere);
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(Frustum_T<T> const & frustum) const
	{
		return MathLib::intersect_aabb_frustum(*this, frustum) != BO_No;
	}

	template <typename T>
	Vector_T<T, 3> AABBox_T<T>::Corner(size_t index) const
	{
		BOOST_ASSERT(index < 8);

		return Vector_T<T, 3>((index & 1UL) ? this->Max().x() : this->Min().x(),
			(index & 2UL) ? this->Max().y() : this->Min().y(),
			(index & 4UL) ? this->Max().z() : this->Min().z());
	}

	template <typename T>
	bool AABBox_T<T>::operator==(AABBox_T<T> const & rhs) const
	{
		return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
	}
}
