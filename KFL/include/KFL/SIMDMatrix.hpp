/**
 * @file SIMDMatrix.hpp
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

#ifndef _KFL_SIMDMATRIX_HPP
#define _KFL_SIMDMATRIX_HPP

#pragma once

#include <array>
#include <boost/operators.hpp>

namespace KlayGE
{
	class SIMDMatrixF4 final : boost::addable<SIMDMatrixF4,
								boost::subtractable<SIMDMatrixF4,
								boost::dividable2<SIMDMatrixF4, float,
								boost::multipliable2<SIMDMatrixF4, float,
								boost::multipliable<SIMDMatrixF4>>>>>
	{
	public:
		SIMDMatrixF4();
		explicit SIMDMatrixF4(float const * rhs);
		SIMDMatrixF4(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4(SIMDVectorF4 const & v1, SIMDVectorF4 const & v2,
			SIMDVectorF4 const & v3, SIMDVectorF4 const & v4);
		SIMDMatrixF4(float f11, float f12, float f13, float f14,
			float f21, float f22, float f23, float f24,
			float f31, float f32, float f33, float f34,
			float f41, float f42, float f43, float f44);

		static size_t size()
		{
			return 16;
		}

		static SIMDMatrixF4 const & Zero();
		static SIMDMatrixF4 const & Identity();

		void Row(size_t index, SIMDVectorF4 const & rhs);
		SIMDVectorF4 const & Row(size_t index) const;
		void Col(size_t index, SIMDVectorF4 const & rhs);
		SIMDVectorF4 const Col(size_t index) const;

		void Set(size_t row, size_t col, float v);
		float operator()(size_t row, size_t col) const;

		SIMDMatrixF4& operator+=(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4& operator-=(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4& operator*=(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4& operator*=(float rhs);
		SIMDMatrixF4& operator/=(float rhs);

		SIMDMatrixF4& operator=(SIMDMatrixF4 const & rhs);

		SIMDMatrixF4 const operator+() const;
		SIMDMatrixF4 const operator-() const;

	private:
		std::array<SIMDVectorF4, 4> m_;
	};
}

#endif			// _KFL_SIMDMATRIX_HPP
