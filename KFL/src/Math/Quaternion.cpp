/**
 * @file Quaternion.cpp
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

#include <KFL/Quaternion.hpp>

namespace KlayGE
{
	template Quaternion_T<float>::Quaternion_T(float const * rhs);
	template Quaternion_T<float>::Quaternion_T(float3 const & vec, float s);
	template Quaternion_T<float>::Quaternion_T(Quaternion const & rhs);
	template Quaternion_T<float>::Quaternion_T(Quaternion&& rhs);
	template Quaternion_T<float>::Quaternion_T(float x, float y, float z, float w);
	template Quaternion const & Quaternion_T<float>::Identity();
	template Quaternion const & Quaternion_T<float>::operator+=(Quaternion const & rhs);
	template Quaternion const & Quaternion_T<float>::operator-=(Quaternion const & rhs);
	template Quaternion const & Quaternion_T<float>::operator*=(Quaternion const & rhs);
	template Quaternion const & Quaternion_T<float>::operator*=(float rhs);
	template Quaternion const & Quaternion_T<float>::operator/=(float rhs);
	template Quaternion& Quaternion_T<float>::operator=(Quaternion const & rhs);
	template Quaternion& Quaternion_T<float>::operator=(Quaternion&& rhs);
	template Quaternion const Quaternion_T<float>::operator+() const;
	template Quaternion const Quaternion_T<float>::operator-() const;
	template float3 const Quaternion_T<float>::v() const;
	template void Quaternion_T<float>::v(float3 const & rhs);
	template bool Quaternion_T<float>::operator==(Quaternion const & rhs) const;


	template <typename T>
	Quaternion_T<T>::Quaternion_T(T const * rhs)
		: quat_(rhs)
	{
	}

	template <typename T>
	Quaternion_T<T>::Quaternion_T(Vector_T<T, 3> const & vec, T s)
	{
		this->x() = vec.x();
		this->y() = vec.y();
		this->z() = vec.z();
		this->w() = s;
	}

	template <typename T>
	Quaternion_T<T>::Quaternion_T(Quaternion_T const & rhs)
		: quat_(rhs.quat_)
	{
	}

	template <typename T>
	Quaternion_T<T>::Quaternion_T(Quaternion_T&& rhs)
		: quat_(std::move(rhs.quat_))
	{
	}

	template <typename T>
	Quaternion_T<T>::Quaternion_T(T x, T y, T z, T w)
	{
		this->x() = x;
		this->y() = y;
		this->z() = z;
		this->w() = w;
	}

	template <typename T>
	Quaternion_T<T> const & Quaternion_T<T>::Identity()
	{
		static Quaternion_T const out(0, 0, 0, 1);
		return out;
	}

	template <typename T>
	Quaternion_T<T> const & Quaternion_T<T>::operator+=(Quaternion_T<T> const & rhs)
	{
		quat_ += rhs.quat_;
		return *this;
	}

	template <typename T>
	Quaternion_T<T> const & Quaternion_T<T>::operator-=(Quaternion_T<T> const & rhs)
	{
		quat_ -= rhs.quat_;
		return *this;
	}

	template <typename T>
	Quaternion_T<T> const & Quaternion_T<T>::operator*=(Quaternion_T<T> const & rhs)
	{
		*this = MathLib::mul(*this, rhs);
		return *this;
	}

	template <typename T>
	Quaternion_T<T> const & Quaternion_T<T>::operator*=(T rhs)
	{
		quat_ *= static_cast<T>(rhs);
		return *this;
	}

	template <typename T>
	Quaternion_T<T> const & Quaternion_T<T>::operator/=(T rhs)
	{
		quat_ /= static_cast<T>(rhs);
		return *this;
	}

	template <typename T>
	Quaternion_T<T>& Quaternion_T<T>::operator=(Quaternion_T const & rhs)
	{
		if (this != &rhs)
		{
			quat_ = rhs.quat_;
		}
		return *this;
	}

	template <typename T>
	Quaternion_T<T>& Quaternion_T<T>::operator=(Quaternion_T&& rhs)
	{
		quat_ = std::move(rhs.quat_);
		return *this;
	}

	template <typename T>
	Quaternion_T<T> const Quaternion_T<T>::operator+() const
	{
		return *this;
	}

	template <typename T>
	Quaternion_T<T> const Quaternion_T<T>::operator-() const
	{
		return Quaternion_T(-this->x(), -this->y(), -this->z(), -this->w());
	}

	template <typename T>
	Vector_T<T, 3> const Quaternion_T<T>::v() const
	{
		return Vector_T<T, 3>(this->x(), this->y(), this->z());
	}

	template <typename T>
	void Quaternion_T<T>::v(Vector_T<T, 3> const & rhs)
	{
		this->x() = rhs.x();
		this->y() = rhs.y();
		this->z() = rhs.z();
	}

	template <typename T>
	bool Quaternion_T<T>::operator==(Quaternion_T<T> const & rhs) const
	{
		return quat_ == rhs.quat_;
	}
}
