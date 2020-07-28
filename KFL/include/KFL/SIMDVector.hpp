/**
 * @file SIMDVector.hpp
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

#ifndef _KFL_SIMDVECTOR_HPP
#define _KFL_SIMDVECTOR_HPP

#pragma once

#include <boost/operators.hpp>

namespace KlayGE
{
#if defined(SIMD_MATH_SSE)
	typedef __m128 V4TYPE;
#else
	typedef std::array<float, 4> V4TYPE;
#endif

	class SIMDVectorF4 final : boost::addable<SIMDVectorF4,
								boost::subtractable<SIMDVectorF4,
								boost::multipliable<SIMDVectorF4,
								boost::dividable<SIMDVectorF4,
								boost::dividable2<SIMDVectorF4, float,
								boost::multipliable2<SIMDVectorF4, float,
								boost::addable2<SIMDVectorF4, float,
								boost::subtractable2<SIMDVectorF4, float>>>>>>>>
	{
	public:
		SIMDVectorF4()
		{
		}
		SIMDVectorF4(SIMDVectorF4 const & rhs);

		static size_t size()
		{
			return 4;
		}

		static SIMDVectorF4 const & Zero();

		V4TYPE& Vec()
		{
			return vec_;
		}
		V4TYPE const & Vec() const
		{
			return vec_;
		}

		SIMDVectorF4 const & operator+=(SIMDVectorF4 const & rhs);
		SIMDVectorF4 const & operator+=(float rhs);
		SIMDVectorF4 const & operator-=(SIMDVectorF4 const & rhs);
		SIMDVectorF4 const & operator-=(float rhs);
		SIMDVectorF4 const & operator*=(SIMDVectorF4 const & rhs);
		SIMDVectorF4 const & operator*=(float rhs);
		SIMDVectorF4 const & operator/=(SIMDVectorF4 const & rhs);
		SIMDVectorF4 const & operator/=(float rhs);

		SIMDVectorF4& operator=(SIMDVectorF4 const & rhs);

		SIMDVectorF4 const operator+() const;
		SIMDVectorF4 const operator-() const;

		void swap(SIMDVectorF4& rhs);

	private:
		V4TYPE vec_{};
	};

	inline void swap(SIMDVectorF4& lhs, SIMDVectorF4& rhs)
	{
		lhs.swap(rhs);
	}
}

#endif			// _KFL_SIMDVECTOR_HPP
