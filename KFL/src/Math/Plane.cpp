/**
 * @file Plane.cpp
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

#include <KFL/Plane.hpp>

namespace KlayGE
{
	template Plane_T<float>::Plane_T(float const * rhs);
	template Plane_T<float>::Plane_T(Plane_T const & rhs);
	template Plane_T<float>::Plane_T(float4 const & rhs);
	template Plane_T<float>::Plane_T(float a, float b, float c, float d);
	template Plane_T<float>& Plane_T<float>::operator=(Plane const & rhs);
	template Plane_T<float>& Plane_T<float>::operator=(float4 const & rhs);
	template Plane_T<float> const Plane_T<float>::operator+() const;
	template Plane_T<float> const Plane_T<float>::operator-() const;
	template float3 const Plane_T<float>::Normal() const;
	template void Plane_T<float>::Normal(Vector_T<float, 3> const & rhs);
	template bool Plane_T<float>::operator==(Plane_T<float> const & rhs) const;


	template <typename T>
	Plane_T<T>::Plane_T(T const * rhs)
		: plane_(rhs)
	{
	}
	
	template <typename T>
	Plane_T<T>::Plane_T(Plane_T const & rhs)
		: plane_(rhs.plane_)
	{
	}
	
	template <typename T>
	Plane_T<T>::Plane_T(Vector_T<T, elem_num> const & rhs)
	{
		plane_ = rhs;
	}
	
	template <typename T>
	Plane_T<T>::Plane_T(T a, T b, T c, T d)
	{
		this->a() = a;
		this->b() = b;
		this->c() = c;
		this->d() = d;
	}

	template <typename T>
	Plane_T<T>& Plane_T<T>::operator=(Plane_T<T> const & rhs)
	{
		if (this != &rhs)
		{
			plane_ = rhs.plane_;
		}
		return *this;
	}
	
	template <typename T>
	Plane_T<T>& Plane_T<T>::operator=(Vector_T<T, elem_num> const & rhs)
	{
		plane_ = rhs;
		return *this;
	}

	template <typename T>
	Plane_T<T> const Plane_T<T>::operator+() const
	{
		return *this;
	}
	
	template <typename T>
	Plane_T<T> const Plane_T<T>::operator-() const
	{
		return Plane_T<T>(-this->a(), -this->b(), -this->c(), -this->d());
	}

	template <typename T>
	Vector_T<T, 3> const Plane_T<T>::Normal() const
	{
		return Vector_T<T, 3>(this->a(), this->b(), this->c());
	}
	
	template <typename T>
	void Plane_T<T>::Normal(Vector_T<T, 3> const & rhs)
	{
		this->a() = rhs.x();
		this->b() = rhs.y();
		this->c() = rhs.z();
	}

	template <typename T>
	bool Plane_T<T>::operator==(Plane_T<T> const & rhs) const
	{
		return plane_ == rhs.plane_;
	}
}
