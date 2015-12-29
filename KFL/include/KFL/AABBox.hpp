/**
 * @file AABBox.hpp
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

#ifndef _KFL_AABBOX_HPP
#define _KFL_AABBOX_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <boost/operators.hpp>

#include <KFL/Bound.hpp>

namespace KlayGE
{
	template <typename T>
	class AABBox_T : boost::addable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::subtractable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::multipliable2<AABBox_T<T>, T,
						boost::dividable2<AABBox_T<T>, T,
						boost::andable<AABBox_T<T>,
						boost::orable<AABBox_T<T>,
						boost::equality_comparable<AABBox_T<T>>>>>>>>,
				public Bound_T<T>
	{
	public:
		AABBox_T() KLAYGE_NOEXCEPT
		{
		}
		AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax) KLAYGE_NOEXCEPT;
		AABBox_T(Vector_T<T, 3>&& vMin, Vector_T<T, 3>&& vMax) KLAYGE_NOEXCEPT;
		AABBox_T(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT;
		AABBox_T(AABBox_T<T>&& rhs) KLAYGE_NOEXCEPT;

		// 赋值操作符
		AABBox_T<T>& operator+=(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT;
		AABBox_T<T>& operator-=(Vector_T<T, 3> const & rhs) KLAYGE_NOEXCEPT;
		AABBox_T<T>& operator*=(T rhs) KLAYGE_NOEXCEPT;
		AABBox_T<T>& operator/=(T rhs) KLAYGE_NOEXCEPT;
		AABBox_T<T>& operator&=(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT;
		AABBox_T<T>& operator|=(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT;

		AABBox_T<T>& operator=(AABBox_T<T> const & rhs) KLAYGE_NOEXCEPT;
		AABBox_T<T>& operator=(AABBox_T<T>&& rhs) KLAYGE_NOEXCEPT;

		// 一元操作符
		AABBox_T<T> const operator+() const KLAYGE_NOEXCEPT;
		AABBox_T<T> const operator-() const KLAYGE_NOEXCEPT;

		// 属性
		T Width() const KLAYGE_NOEXCEPT;
		T Height() const KLAYGE_NOEXCEPT;
		T Depth() const KLAYGE_NOEXCEPT;
		virtual bool IsEmpty() const KLAYGE_NOEXCEPT override;

		Vector_T<T, 3> const LeftBottomNear() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const LeftTopNear() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const RightBottomNear() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const RightTopNear() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const LeftBottomFar() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const LeftTopFar() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const RightBottomFar() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> const RightTopFar() const KLAYGE_NOEXCEPT;

		Vector_T<T, 3>& Min() KLAYGE_NOEXCEPT
		{
			return min_;
		}
		Vector_T<T, 3> const & Min() const KLAYGE_NOEXCEPT
		{
			return min_;
		}
		Vector_T<T, 3>& Max() KLAYGE_NOEXCEPT
		{
			return max_;
		}
		Vector_T<T, 3> const & Max() const KLAYGE_NOEXCEPT
		{
			return max_;
		}
		Vector_T<T, 3> Center() const KLAYGE_NOEXCEPT;
		Vector_T<T, 3> HalfSize() const KLAYGE_NOEXCEPT;

		virtual bool VecInBound(Vector_T<T, 3> const & v) const KLAYGE_NOEXCEPT override;
		virtual T MaxRadiusSq() const KLAYGE_NOEXCEPT override;

		bool Intersect(AABBox_T<T> const & aabb) const KLAYGE_NOEXCEPT;
		bool Intersect(OBBox_T<T> const & obb) const KLAYGE_NOEXCEPT;
		bool Intersect(Sphere_T<T> const & sphere) const KLAYGE_NOEXCEPT;
		bool Intersect(Frustum_T<T> const & frustum) const KLAYGE_NOEXCEPT;

		Vector_T<T, 3> Corner(size_t index) const KLAYGE_NOEXCEPT;

		bool operator==(AABBox_T<T> const & rhs) const KLAYGE_NOEXCEPT;

	private:
		Vector_T<T, 3> min_, max_;
	};
}

#endif			// _KFL_AABBOX_HPP
