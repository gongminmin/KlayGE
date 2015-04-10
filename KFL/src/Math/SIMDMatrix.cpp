/**
 * @file SIMDMatrix.cpp
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
#include <KFL/SIMDMath.hpp>

namespace KlayGE
{
	SIMDMatrixF4::SIMDMatrixF4()
	{
	}
	
	SIMDMatrixF4::SIMDMatrixF4(float const * rhs)
	{
		m_[0] = SIMDMathLib::LoadVector4(rhs + 0);
		m_[1] = SIMDMathLib::LoadVector4(rhs + 4);
		m_[2] = SIMDMathLib::LoadVector4(rhs + 8);
		m_[3] = SIMDMathLib::LoadVector4(rhs + 12);
	}

	SIMDMatrixF4::SIMDMatrixF4(SIMDMatrixF4 const & rhs)
		: m_(rhs.m_)
	{
	}

	SIMDMatrixF4::SIMDMatrixF4(SIMDVectorF4 const & v1, SIMDVectorF4 const & v2,
		SIMDVectorF4 const & v3, SIMDVectorF4 const & v4)
	{
		m_[0] = v1;
		m_[1] = v2;
		m_[2] = v3;
		m_[3] = v4;
	}

	SIMDMatrixF4::SIMDMatrixF4(float f11, float f12, float f13, float f14,
		float f21, float f22, float f23, float f24,
		float f31, float f32, float f33, float f34,
		float f41, float f42, float f43, float f44)
	{
		m_[0] = SIMDMathLib::SetVector(f11, f12, f13, f14);
		m_[1] = SIMDMathLib::SetVector(f21, f22, f23, f24);
		m_[2] = SIMDMathLib::SetVector(f31, f32, f33, f34);
		m_[3] = SIMDMathLib::SetVector(f41, f42, f43, f44);
	}

	SIMDMatrixF4 const & SIMDMatrixF4::Zero()
	{
		static SIMDMatrixF4 const out(
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0);
		return out;
	}

	SIMDMatrixF4 const & SIMDMatrixF4::Identity()
	{
		static SIMDMatrixF4 const out(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		return out;
	}

	void SIMDMatrixF4::Row(size_t index, SIMDVectorF4 const & rhs)
	{
		m_[index] = rhs;
	}

	SIMDVectorF4 const & SIMDMatrixF4::Row(size_t index) const
	{
		return m_[index];
	}

	void SIMDMatrixF4::Col(size_t index, SIMDVectorF4 const & rhs)
	{
		m_[0] = SIMDMathLib::SetByIndex(m_[0], SIMDMathLib::GetByIndex(rhs, index), index);
		m_[1] = SIMDMathLib::SetByIndex(m_[1], SIMDMathLib::GetByIndex(rhs, index), index);
		m_[2] = SIMDMathLib::SetByIndex(m_[2], SIMDMathLib::GetByIndex(rhs, index), index);
		m_[3] = SIMDMathLib::SetByIndex(m_[3], SIMDMathLib::GetByIndex(rhs, index), index);
	}

	SIMDVectorF4 const SIMDMatrixF4::Col(size_t index) const
	{
		return SIMDMathLib::SetVector(SIMDMathLib::GetByIndex(m_[0], index),
			SIMDMathLib::GetByIndex(m_[1], index),
			SIMDMathLib::GetByIndex(m_[2], index),
			SIMDMathLib::GetByIndex(m_[3], index));
	}

	void SIMDMatrixF4::Set(size_t row, size_t col, float v)
	{
		this->Row(row, SIMDMathLib::SetByIndex(this->Row(row), v, col));
	}

	float SIMDMatrixF4::operator()(size_t row, size_t col) const
	{
		return SIMDMathLib::GetByIndex(this->Row(row), col);
	}

	SIMDMatrixF4& SIMDMatrixF4::operator+=(SIMDMatrixF4 const & rhs)
	{
		*this = SIMDMathLib::Add(*this, rhs);
		return *this;
	}

	SIMDMatrixF4& SIMDMatrixF4::operator-=(SIMDMatrixF4 const & rhs)
	{
		*this = SIMDMathLib::Substract(*this, rhs);
		return *this;
	}

	SIMDMatrixF4& SIMDMatrixF4::operator*=(SIMDMatrixF4 const & rhs)
	{
		*this = SIMDMathLib::Multiply(*this, rhs);
		return *this;
	}

	SIMDMatrixF4& SIMDMatrixF4::operator*=(float rhs)
	{
		*this = SIMDMathLib::Multiply(*this, rhs);
		return *this;
	}

	SIMDMatrixF4& SIMDMatrixF4::operator/=(float rhs)
	{
		*this = SIMDMathLib::Multiply(*this, 1.0f / rhs);
		return *this;
	}

	SIMDMatrixF4& SIMDMatrixF4::operator=(SIMDMatrixF4 const & rhs)
	{
		if (this != &rhs)
		{
			m_ = rhs.m_;
		}
		return *this;
	}

	SIMDMatrixF4 const SIMDMatrixF4::operator+() const
	{
		return *this;
	}

	SIMDMatrixF4 const SIMDMatrixF4::operator-() const
	{
		return SIMDMathLib::Negative(*this);
	}
}
