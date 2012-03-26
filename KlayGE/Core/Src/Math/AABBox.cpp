// AABBox.cpp
// KlayGE AABB implement file
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
#include <boost/operators.hpp>

#include <KlayGE/AABBox.hpp>

namespace KlayGE
{
	template class KLAYGE_CORE_API AABBox_T<float>;

	template <typename T>
	AABBox_T<T>::AABBox_T()
	{
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax)
			: min_(vMin), max_(vMax)
	{
		BOOST_ASSERT(vMin.x() <= vMax.x());
		BOOST_ASSERT(vMin.y() <= vMax.y());
		BOOST_ASSERT(vMin.z() <= vMax.z());
	}

	template <typename T>
	AABBox_T<T>::AABBox_T(AABBox_T const & rhs)
		: Bound_T<T>(rhs),
			min_(rhs.min_), max_(rhs.max_)
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
	AABBox_T<T>& AABBox_T<T>::operator*=(T const & rhs)
	{
		this->Min() *= rhs;
		this->Max() *= rhs;
		return *this;
	}

	template <typename T>
	AABBox_T<T>& AABBox_T<T>::operator/=(T const & rhs)
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
	Vector_T<T, 3> AABBox_T<T>::operator[](size_t i) const
	{
		BOOST_ASSERT(i < 8);

		return Vector_T<T, 3>((i & 1UL) ? this->Max().x() : this->Min().x(),
			(i & 2UL) ? this->Max().y() : this->Min().y(),
			(i & 4UL) ? this->Max().z() : this->Min().z());
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
	Vector_T<T, 3>& AABBox_T<T>::Min()
	{
		return min_;
	}

	template <typename T>
	Vector_T<T, 3> const & AABBox_T<T>::Min() const
	{
		return min_;
	}

	template <typename T>
	Vector_T<T, 3>& AABBox_T<T>::Max()
	{
		return max_;
	}

	template <typename T>
	Vector_T<T, 3> const & AABBox_T<T>::Max() const
	{
		return max_;
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
		return MathLib::vec_in_box(*this, v);
	}

	template <typename T>
	T AABBox_T<T>::MaxRadiusSq() const
	{
		return std::max<T>(MathLib::length_sq(this->Max()), MathLib::length_sq(this->Min()));
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(AABBox_T<T> const & aabb) const
	{
		float3 const t = aabb.Center() - this->Center();
		float3 const e = this->HalfSize() + aabb.HalfSize();
		return (MathLib::abs(t.x()) <= e.x()) && (MathLib::abs(t.y()) <= e.y()) && (MathLib::abs(t.z()) <= e.z());
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(OBBox_T<T> const & obb) const
	{
		return obb.Intersect(OBBox_T<T>(*this));
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(Sphere_T<T> const & sphere) const
	{
		UNREF_PARAM(sphere);
		BOOST_ASSERT(false);

		return false;
	}

	template <typename T>
	bool AABBox_T<T>::Intersect(Frustum_T<T> const & frustum) const
	{
		return frustum.Intersect(*this) != BO_No;
	}
}
