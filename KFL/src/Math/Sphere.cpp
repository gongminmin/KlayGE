/**
 * @file Sphere.cpp
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

#include <KFL/Sphere.hpp>

namespace KlayGE
{
	template <typename T>
	Sphere_T<T>::Sphere_T(Sphere_T<T> const & rhs) noexcept
		: center_(rhs.center_),
			radius_(rhs.radius_)
	{
	}

	template <typename T>
	Sphere_T<T>::Sphere_T(Sphere_T<T>&& rhs) noexcept
		: center_(std::move(rhs.center_)),
			radius_(std::move(rhs.radius_))
	{
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator+=(Vector_T<T, 3> const & rhs) noexcept
	{
		this->Center() += rhs;
		return *this;
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator-=(Vector_T<T, 3> const & rhs) noexcept
	{
		this->Center() -= rhs;
		return *this;
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator*=(T rhs) noexcept
	{
		this->Radius() *= rhs;
		return *this;
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator/=(T rhs) noexcept
	{
		return this->operator*=(1.0f / rhs);
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator=(Sphere_T const & rhs) noexcept
	{
		if (this != &rhs)
		{
			this->Center() = rhs.Center();
			this->Radius() = rhs.Radius();
		}
		return *this;
	}

	template <typename T>
	Sphere_T<T>& Sphere_T<T>::operator=(Sphere_T&& rhs) noexcept
	{
		center_ = std::move(rhs.center_);
		radius_ = std::move(rhs.radius_);
		return *this;
	}

	template <typename T>
	Sphere_T<T> const & Sphere_T<T>::operator+() const noexcept
	{
		return *this;
	}

	template <typename T>
	Sphere_T<T> const & Sphere_T<T>::operator-() const noexcept
	{
		return *this;
	}

	template <typename T>
	bool Sphere_T<T>::IsEmpty() const noexcept
	{
		return MathLib::equal(radius_, 0.0f);
	}

	template <typename T>
	bool Sphere_T<T>::VecInBound(Vector_T<T, 3> const & v) const noexcept
	{
		return MathLib::intersect_point_sphere(v, *this);
	}

	template <typename T>
	T Sphere_T<T>::MaxRadiusSq() const noexcept
	{
		return this->Radius() * this->Radius();
	}

	template <typename T>
	bool Sphere_T<T>::Intersect(AABBox_T<T> const & aabb) const noexcept
	{
		return aabb.Intersect(*this);
	}

	template <typename T>
	bool Sphere_T<T>::Intersect(OBBox_T<T> const & obb) const noexcept
	{
		return obb.Intersect(*this);
	}

	template <typename T>
	bool Sphere_T<T>::Intersect(Sphere_T<T> const & sphere) const noexcept
	{
		return MathLib::intersect_sphere_sphere(*this, sphere);
	}

	template <typename T>
	bool Sphere_T<T>::Intersect(Frustum_T<T> const & frustum) const noexcept
	{
		return MathLib::intersect_sphere_frustum(*this, frustum) != BO_No;
	}

	template <typename T>
	bool Sphere_T<T>::operator==(Sphere_T<T> const & rhs) const noexcept
	{
		return (center_ == rhs.center_) && (radius_ == rhs.radius_);
	}


	template class Sphere_T<float>;
}
