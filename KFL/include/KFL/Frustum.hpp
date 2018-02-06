/**
 * @file Frustum.hpp
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

#ifndef _KFL_FRUSTUM_HPP
#define _KFL_FRUSTUM_HPP

#pragma once

#include <KFL/PreDeclare.hpp>
#include <array>

namespace KlayGE
{
	template <typename T>
	class Frustum_T final : public Bound_T<T>
	{
	public:
		constexpr Frustum_T() noexcept
		{
		}
		Frustum_T(Frustum_T<T> const & rhs) noexcept;
		Frustum_T(Frustum_T<T>&& rhs) noexcept;

		Frustum_T& operator=(Frustum_T const & rhs) noexcept;
		Frustum_T& operator=(Frustum_T&& rhs) noexcept;

		void ClipMatrix(Matrix4_T<T> const & clip, Matrix4_T<T> const & inv_clip) noexcept;

		virtual bool IsEmpty() const noexcept override;

		virtual bool VecInBound(Vector_T<T, 3> const & v) const noexcept override;
		virtual float MaxRadiusSq() const noexcept override;

		void FrustumPlane(uint32_t index, Plane_T<T> const & plane) noexcept
		{
			planes_[index] = plane;
		}
		constexpr Plane_T<T> const & FrustumPlane(uint32_t index) const noexcept
		{
			return planes_[index];
		}

		void Corner(uint32_t index, Vector_T<T, 3> const & corner) noexcept
		{
			corners_[index] = corner;
		}
		constexpr Vector_T<T, 3> const & Corner(uint32_t index) const noexcept
		{
			return corners_[index];
		}

		BoundOverlap Intersect(AABBox_T<T> const & aabb) const noexcept;
		BoundOverlap Intersect(OBBox_T<T> const & obb) const noexcept;
		BoundOverlap Intersect(Sphere_T<T> const & sphere) const noexcept;
		BoundOverlap Intersect(Frustum_T<T> const & frustum) const noexcept;

	private:
		std::array<Plane_T<T>, 6> planes_;
		std::array<Vector_T<T, 3>, 8> corners_;
	};
}

#endif			// _KFL_FRUSTUM_HPP
