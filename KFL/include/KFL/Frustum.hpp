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

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6385)
#endif
#include <boost/array.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	template <typename T>
	class Frustum_T : public Bound_T<T>
	{
	public:
		void ClipMatrix(Matrix4_T<T> const & clip)
		{
			corners_[0] = MathLib::transform_coord(Vector_T<T, 3>(-1, -1, 0), clip); // left bottom near
			corners_[1] = MathLib::transform_coord(Vector_T<T, 3>(+1, -1, 0), clip); // right bottom near
			corners_[2] = MathLib::transform_coord(Vector_T<T, 3>(-1, +1, 0), clip); // left top near
			corners_[3] = MathLib::transform_coord(Vector_T<T, 3>(+1, +1, 0), clip); // right top near
			corners_[4] = MathLib::transform_coord(Vector_T<T, 3>(-1, -1, 1), clip); // left bottom far
			corners_[5] = MathLib::transform_coord(Vector_T<T, 3>(+1, -1, 1), clip); // right bottom far
			corners_[6] = MathLib::transform_coord(Vector_T<T, 3>(-1, +1, 1), clip); // left top far
			corners_[7] = MathLib::transform_coord(Vector_T<T, 3>(+1, +1, 1), clip); // right top far

			Vector_T<T, 4> const & column1(clip.Col(0));
			Vector_T<T, 4> const & column2(clip.Col(1));
			Vector_T<T, 4> const & column3(clip.Col(2));
			Vector_T<T, 4> const & column4(clip.Col(3));

			planes_[0] = column4 - column1;  // left
			planes_[1] = column4 + column1;  // right
			planes_[2] = column4 - column2;  // bottom
			planes_[3] = column4 + column2;  // top
			planes_[4] = column4 - column3;  // near
			planes_[5] = column4 + column3;  // far

			// Loop through each side of the frustum and normalize it.
			KLAYGE_FOREACH(typename planes_t::reference plane, planes_)
			{
				plane = MathLib::normalize(plane);
			}
		}

		bool IsEmpty() const
		{
			return false;
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::intersect_point_frustum(v, *this);
		}
		float MaxRadiusSq() const
		{
			return 0;
		}

		void FrustumPlane(uint32_t index, Plane_T<T> const & plane)
		{
			planes_[index] = plane;
		}
		Plane_T<T> const & FrustumPlane(uint32_t index) const
		{
			return planes_[index];
		}

		void Corner(uint32_t index, Vector_T<T, 3> const & corner)
		{
			corners_[index] = corner;
		}
		Vector_T<T, 3> const & Corner(uint32_t index) const
		{
			return corners_[index];
		}

		BoundOverlap Intersect(AABBox_T<T> const & aabb) const
		{
			return MathLib::intersect_aabb_frustum(aabb, *this);
		}
		BoundOverlap Intersect(OBBox_T<T> const & obb) const
		{
			return MathLib::intersect_obb_frustum(obb, *this);
		}	
		BoundOverlap Intersect(Sphere_T<T> const & sphere) const
		{
			return MathLib::intersect_sphere_frustum(sphere, *this);
		}
		BoundOverlap Intersect(Frustum_T<T> const & frustum) const
		{
			return MathLib::intersect_frustum_frustum(frustum, *this);
		}

	private:
		typedef boost::array<Plane_T<T>, 6> planes_t;
		planes_t planes_;

		typedef boost::array<Vector_T<T, 3>, 8> corners_t;
		corners_t corners_;
	};
}

#endif			// _KFL_FRUSTUM_HPP
