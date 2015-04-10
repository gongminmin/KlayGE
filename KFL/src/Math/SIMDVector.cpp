/**
 * @file SIMDVector.cpp
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
	SIMDVectorF4::SIMDVectorF4(SIMDVectorF4 const & rhs)
		: vec_(rhs.vec_)
	{
	}

	SIMDVectorF4 const & SIMDVectorF4::Zero()
	{
		static SIMDVectorF4 const zero = SIMDMathLib::SetVector(0.0f);
		return zero;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator+=(SIMDVectorF4 const & rhs)
	{
		*this = SIMDMathLib::Add(*this, rhs);
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator+=(float rhs)
	{
		*this += SIMDMathLib::SetVector(rhs);
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator-=(SIMDVectorF4 const & rhs)
	{
		*this = SIMDMathLib::Substract(*this, rhs);
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator-=(float rhs)
	{
		*this -= SIMDMathLib::SetVector(rhs);
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator*=(SIMDVectorF4 const & rhs)
	{
		*this = SIMDMathLib::Multiply(*this, rhs);
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator*=(float rhs)
	{
		*this = SIMDMathLib::Multiply(*this, SIMDMathLib::SetVector(rhs));
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator/=(SIMDVectorF4 const & rhs)
	{
		*this = SIMDMathLib::Divide(*this, rhs);
		return *this;
	}

	SIMDVectorF4 const & SIMDVectorF4::operator/=(float rhs)
	{
		return this->operator*=(1.0f / rhs);
	}

	SIMDVectorF4& SIMDVectorF4::operator=(SIMDVectorF4 const & rhs)
	{
		if (this != &rhs)
		{
			vec_ = rhs.vec_;
		}
		return *this;
	}

	SIMDVectorF4 const SIMDVectorF4::operator+() const
	{
		return *this;
	}
	SIMDVectorF4 const SIMDVectorF4::operator-() const
	{
		return SIMDMathLib::Negative(*this);
	}

	void SIMDVectorF4::swap(SIMDVectorF4& rhs)
	{
		std::swap(vec_, rhs.vec_);
	}
}
