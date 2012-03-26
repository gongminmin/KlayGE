// Sphere.cpp
// KlayGE Sphere implement file
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

#include <KlayGE/Sphere.hpp>

namespace KlayGE
{
	template class KLAYGE_CORE_API Sphere_T<float>;

	template <typename T>
	Sphere_T<T>::Sphere_T()
	{
	}

	template <typename T>
	Sphere_T<T>::Sphere_T(Vector_T<T, 3> const & center, T const & radius)
		: center_(center),
			radius_(radius)
	{
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator+=(Vector_T<T, 3> const & rhs)
	{
		this->Center() += rhs;
		return *this;
	}
	
	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator-=(Vector_T<T, 3> const & rhs)
	{
		this->Center() -= rhs;
		return *this;
	}
	
	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator*=(T const & rhs)
	{
		this->Radius() *= rhs;
		return *this;
	}
	
	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator/=(T const & rhs)
	{
		return this->operator*=(1.0f / rhs);
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator=(Sphere_T const & rhs)
	{
		if (this != &rhs)
		{
			this->Center() = rhs.Center();
			this->Radius() = rhs.Radius();
		}
		return *this;
	}

	template <typename T>
	Sphere_T<T> const & Sphere_T<T>::operator+() const
	{
		return *this;
	}
	
	template <typename T>
	Sphere_T<T> const & Sphere_T<T>::operator-() const
	{
		return *this;
	}

	template <typename T>
	Vector_T<T, 3>& Sphere_T<T>::Center()
	{
		return center_;
	}
	
	template <typename T>
	Vector_T<T, 3> const & Sphere_T<T>::Center() const
	{
		return center_;
	}
	
	template <typename T>
	T& Sphere_T<T>::Radius()
	{
		return radius_;
	}
	
	template <typename T>
	T Sphere_T<T>::Radius() const
	{
		return radius_;
	}

	template <typename T>
	bool Sphere_T<T>::IsEmpty() const
	{
		return MathLib::equal(radius_, 0.0f);
	}

	template <typename T>
	bool Sphere_T<T>::VecInBound(Vector_T<T, 3> const & v) const
	{
		return MathLib::vec_in_sphere(*this, v);
	}
	
	template <typename T>
	T Sphere_T<T>::MaxRadiusSq() const
	{
		return this->Radius() * this->Radius();
	}

	template <typename T>
	BoundOverlap Sphere_T<T>::CollisionDet(AABBox_T<T> const & aabb) const
	{
		UNREF_PARAM(aabb);
		BOOST_ASSERT(false);

		return BO_No;
	}

	template <typename T>
	BoundOverlap Sphere_T<T>::CollisionDet(OBBox_T<T> const & obb) const
	{
		UNREF_PARAM(obb);
		BOOST_ASSERT(false);

		return BO_No;
	}

	template <typename T>
	BoundOverlap Sphere_T<T>::CollisionDet(Sphere_T<T> const & sphere) const
	{
		UNREF_PARAM(sphere);
		BOOST_ASSERT(false);

		return BO_No;
	}

	template <typename T>
	BoundOverlap Sphere_T<T>::CollisionDet(Frustum_T<T> const & frustum) const
	{
		UNREF_PARAM(frustum);
		BOOST_ASSERT(false);

		return BO_No;
	}
}
