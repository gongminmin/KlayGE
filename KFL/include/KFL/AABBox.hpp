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
	class AABBox_T final : boost::addable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::subtractable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::multipliable2<AABBox_T<T>, T,
						boost::dividable2<AABBox_T<T>, T,
						boost::andable<AABBox_T<T>,
						boost::orable<AABBox_T<T>,
						boost::equality_comparable<AABBox_T<T>>>>>>>>,
				public Bound_T<T>
	{
	public:
		constexpr AABBox_T() noexcept
		{
		}
		AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax) noexcept;
		AABBox_T(Vector_T<T, 3>&& vMin, Vector_T<T, 3>&& vMax) noexcept;
		AABBox_T(AABBox_T<T> const & rhs) noexcept;
		AABBox_T(AABBox_T<T>&& rhs) noexcept;

		// 赋值操作符
		AABBox_T<T>& operator+=(Vector_T<T, 3> const & rhs) noexcept;
		AABBox_T<T>& operator-=(Vector_T<T, 3> const & rhs) noexcept;
		AABBox_T<T>& operator*=(T rhs) noexcept;
		AABBox_T<T>& operator/=(T rhs) noexcept;
		AABBox_T<T>& operator&=(AABBox_T<T> const & rhs) noexcept;
		AABBox_T<T>& operator|=(AABBox_T<T> const & rhs) noexcept;

		AABBox_T<T>& operator=(AABBox_T<T> const & rhs) noexcept;
		AABBox_T<T>& operator=(AABBox_T<T>&& rhs) noexcept;

		// 一元操作符
		AABBox_T<T> const operator+() const noexcept;
		AABBox_T<T> const operator-() const noexcept;

		// 属性
		T Width() const noexcept;
		T Height() const noexcept;
		T Depth() const noexcept;
		virtual bool IsEmpty() const noexcept override;

		Vector_T<T, 3> const LeftBottomNear() const noexcept;
		Vector_T<T, 3> const LeftTopNear() const noexcept;
		Vector_T<T, 3> const RightBottomNear() const noexcept;
		Vector_T<T, 3> const RightTopNear() const noexcept;
		Vector_T<T, 3> const LeftBottomFar() const noexcept;
		Vector_T<T, 3> const LeftTopFar() const noexcept;
		Vector_T<T, 3> const RightBottomFar() const noexcept;
		Vector_T<T, 3> const RightTopFar() const noexcept;

		Vector_T<T, 3>& Min() noexcept
		{
			return min_;
		}
		constexpr Vector_T<T, 3> const & Min() const noexcept
		{
			return min_;
		}
		Vector_T<T, 3>& Max() noexcept
		{
			return max_;
		}
		constexpr Vector_T<T, 3> const & Max() const noexcept
		{
			return max_;
		}
		Vector_T<T, 3> Center() const noexcept;
		Vector_T<T, 3> HalfSize() const noexcept;

		virtual bool VecInBound(Vector_T<T, 3> const & v) const noexcept override;
		virtual T MaxRadiusSq() const noexcept override;

		bool Intersect(AABBox_T<T> const & aabb) const noexcept;
		bool Intersect(OBBox_T<T> const & obb) const noexcept;
		bool Intersect(Sphere_T<T> const & sphere) const noexcept;
		bool Intersect(Frustum_T<T> const & frustum) const noexcept;

		Vector_T<T, 3> Corner(size_t index) const noexcept;

		bool operator==(AABBox_T<T> const & rhs) const noexcept;

	private:
		Vector_T<T, 3> min_, max_;
	};
}

#endif			// _KFL_AABBOX_HPP
