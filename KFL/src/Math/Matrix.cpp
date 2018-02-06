/**
 * @file Matrix.cpp
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

#include <KFL/Matrix.hpp>

namespace KlayGE
{
	template <typename T>
	Matrix4_T<T>::Matrix4_T(T const * rhs) noexcept
	{
		for (size_t i = 0; i < row_num; ++ i)
		{
			m_[i] = Vector_T<T, col_num>(rhs);
			rhs += col_num;
		}
	}

	template <typename T>
	Matrix4_T<T>::Matrix4_T(Matrix4_T const & rhs) noexcept
		: m_(rhs.m_)
	{
	}

	template <typename T>
	Matrix4_T<T>::Matrix4_T(Matrix4_T&& rhs) noexcept
		: m_(std::move(rhs.m_))
	{
	}

	template <typename T>
	Matrix4_T<T>::Matrix4_T(T f11, T f12, T f13, T f14,
		T f21, T f22, T f23, T f24,
		T f31, T f32, T f33, T f34,
		T f41, T f42, T f43, T f44) noexcept
	{
		m_[0][0] = f11;	m_[0][1] = f12;	m_[0][2] = f13;	m_[0][3] = f14;
		m_[1][0] = f21;	m_[1][1] = f22;	m_[1][2] = f23;	m_[1][3] = f24;
		m_[2][0] = f31;	m_[2][1] = f32;	m_[2][2] = f33;	m_[2][3] = f34;
		m_[3][0] = f41;	m_[3][1] = f42;	m_[3][2] = f43;	m_[3][3] = f44;
	}

	template <typename T>
	Matrix4_T<T> const & Matrix4_T<T>::Zero() noexcept
	{
		static Matrix4_T const out(
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0);
		return out;
	}

	template <typename T>
	Matrix4_T<T> const & Matrix4_T<T>::Identity() noexcept
	{
		static Matrix4_T const out(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		return out;
	}

	template <typename T>
	void Matrix4_T<T>::Row(size_t index, Vector_T<T, col_num> const & rhs) noexcept
	{
		m_[index] = rhs;
	}

	template <typename T>
	Vector_T<T, 4> const & Matrix4_T<T>::Row(size_t index) const noexcept
	{
		return m_[index];
	}

	template <typename T>
	void Matrix4_T<T>::Col(size_t index, Vector_T<T, row_num> const & rhs) noexcept
	{
		for (size_t i = 0; i < row_num; ++ i)
		{
			m_[i][index] = rhs[i];
		}
	}

	template <typename T>
	Vector_T<T, 4> const Matrix4_T<T>::Col(size_t index) const noexcept
	{
		Vector_T<T, row_num> ret;
		for (size_t i = 0; i < row_num; ++ i)
		{
			ret[i] = m_[i][index];
		}
		return ret;
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator+=(Matrix4_T<T> const & rhs) noexcept
	{
		m_ += rhs.m_;
		return *this;
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator-=(Matrix4_T<T> const & rhs) noexcept
	{
		m_ -= rhs.m_;
		return *this;
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator*=(Matrix4_T<T> const & rhs) noexcept
	{
		*this = MathLib::mul(*this, rhs);
		return *this;
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator*=(T rhs) noexcept
	{
		for (size_t i = 0; i < row_num; ++ i)
		{
			m_[i] *= rhs;
		}
		return *this;
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator/=(T rhs) noexcept
	{
		return this->operator*=(1 / rhs);
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator=(Matrix4_T<T> const & rhs) noexcept
	{
		if (this != &rhs)
		{
			m_ = rhs.m_;
		}
		return *this;
	}

	template <typename T>
	Matrix4_T<T>& Matrix4_T<T>::operator=(Matrix4_T<T>&& rhs) noexcept
	{
		m_ = std::move(rhs.m_);
		return *this;
	}

	template <typename T>
	Matrix4_T<T> const Matrix4_T<T>::operator+() const noexcept
	{
		return *this;
	}

	template <typename T>
	Matrix4_T<T> const Matrix4_T<T>::operator-() const noexcept
	{
		Matrix4_T temp(*this);
		temp.m_ = -m_;
		return temp;
	}

	template <typename T>
	bool Matrix4_T<T>::operator==(Matrix4_T<T> const & rhs) const noexcept
	{
		return m_ == rhs.m_;
	}


	template class Matrix4_T<float>;
}
