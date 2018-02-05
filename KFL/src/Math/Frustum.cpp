/**
 * @file Frustum.cpp
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

#include <KFL/Frustum.hpp>

namespace KlayGE
{
	template <typename T>
	Frustum_T<T>::Frustum_T(Frustum_T<T> const & rhs) noexcept
		: planes_(rhs.planes_), corners_(rhs.corners_)
	{
	}

	template <typename T>
	Frustum_T<T>::Frustum_T(Frustum_T<T>&& rhs) noexcept
		: planes_(std::move(rhs.planes_)), corners_(std::move(rhs.corners_))
	{
	}

	template <typename T>
	Frustum_T<T>& Frustum_T<T>::operator=(Frustum_T<T> const & rhs) noexcept
	{
		if (this != &rhs)
		{
			planes_ = rhs.planes_;
			corners_ = rhs.corners_;
		}
		return *this;
	}

	template <typename T>
	Frustum_T<T>& Frustum_T<T>::operator=(Frustum_T<T>&& rhs) noexcept
	{
		planes_ = std::move(rhs.planes_);
		corners_ = std::move(rhs.corners_);
		return *this;
	}

	template <typename T>
	void Frustum_T<T>::ClipMatrix(Matrix4_T<T> const & clip, Matrix4_T<T> const & inv_clip) noexcept
	{
		corners_[0] = MathLib::transform_coord(Vector_T<T, 3>(-1, -1, 0), inv_clip); // left bottom near
		corners_[1] = MathLib::transform_coord(Vector_T<T, 3>(+1, -1, 0), inv_clip); // right bottom near
		corners_[2] = MathLib::transform_coord(Vector_T<T, 3>(-1, +1, 0), inv_clip); // left top near
		corners_[3] = MathLib::transform_coord(Vector_T<T, 3>(+1, +1, 0), inv_clip); // right top near
		corners_[4] = MathLib::transform_coord(Vector_T<T, 3>(-1, -1, 1), inv_clip); // left bottom far
		corners_[5] = MathLib::transform_coord(Vector_T<T, 3>(+1, -1, 1), inv_clip); // right bottom far
		corners_[6] = MathLib::transform_coord(Vector_T<T, 3>(-1, +1, 1), inv_clip); // left top far
		corners_[7] = MathLib::transform_coord(Vector_T<T, 3>(+1, +1, 1), inv_clip); // right top far

		Vector_T<T, 4> const & column1(clip.Col(0));
		Vector_T<T, 4> const & column2(clip.Col(1));
		Vector_T<T, 4> const & column3(clip.Col(2));
		Vector_T<T, 4> const & column4(clip.Col(3));

		planes_[0] = column4 - column1;	// right
		planes_[1] = column4 + column1;	// left
		planes_[2] = column4 - column2;	// top
		planes_[3] = column4 + column2;	// bottom
		planes_[4] = column4 - column3;	// far
		// TODO: Should be column3 only
		planes_[5] = column4 + column3;	// near

		// Loop through each side of the frustum and normalize it.
		for (auto& plane : planes_)
		{
			plane = MathLib::normalize(plane);
		}
	}

	template <typename T>
	bool Frustum_T<T>::IsEmpty() const noexcept
	{
		return false;
	}

	template <typename T>
	bool Frustum_T<T>::VecInBound(Vector_T<T, 3> const & v) const noexcept
	{
		return MathLib::intersect_point_frustum(v, *this);
	}

	template <typename T>
	float Frustum_T<T>::MaxRadiusSq() const noexcept
	{
		return 0;
	}

	template <typename T>
	BoundOverlap Frustum_T<T>::Intersect(AABBox_T<T> const & aabb) const noexcept
	{
		return MathLib::intersect_aabb_frustum(aabb, *this);
	}

	template <typename T>
	BoundOverlap Frustum_T<T>::Intersect(OBBox_T<T> const & obb) const noexcept
	{
		return MathLib::intersect_obb_frustum(obb, *this);
	}

	template <typename T>
	BoundOverlap Frustum_T<T>::Intersect(Sphere_T<T> const & sphere) const noexcept
	{
		return MathLib::intersect_sphere_frustum(sphere, *this);
	}

	template <typename T>
	BoundOverlap Frustum_T<T>::Intersect(Frustum_T<T> const & frustum) const noexcept
	{
		return MathLib::intersect_frustum_frustum(frustum, *this);
	}


	template class Frustum_T<float>;
}
