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

			//  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
			for (int i = 0; i < 6; ++ i)
			{
				vertex_lut_[i] = ((planes_[i].a() < 0) ? 1 : 0) | ((planes_[i].b() < 0) ? 2 : 0) | ((planes_[i].c() < 0) ? 4 : 0);
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
			vertex_lut_[index] = ((planes_[index].a() < 0) ? 1 : 0) | ((planes_[index].b() < 0) ? 2 : 0) | ((planes_[index].c() < 0) ? 4 : 0);
		}
		Plane_T<T> const & FrustumPlane(uint32_t index) const
		{
			return planes_[index];
		}

		BoundOverlap Intersect(AABBox_T<T> const & aabb) const
		{
			bool intersect = false;
			for (int i = 0; i < 6; ++ i)
			{
				int const n = vertex_lut_[i];

				// v1 is diagonally opposed to v0
				Vector_T<T, 3> v0((n & 1) ? aabb.Min().x() : aabb.Max().x(), (n & 2) ? aabb.Min().y() : aabb.Max().y(), (n & 4) ? aabb.Min().z() : aabb.Max().z());
				Vector_T<T, 3> v1((n & 1) ? aabb.Max().x() : aabb.Min().x(), (n & 2) ? aabb.Max().y() : aabb.Min().y(), (n & 4) ? aabb.Max().z() : aabb.Min().z());

				if (MathLib::dot_coord(planes_[i], v0) < 0)
				{
					return BO_No;
				}
				if (MathLib::dot_coord(planes_[i], v1) < 0)
				{
					intersect = true;
				}
			}

			return intersect ? BO_Partial : BO_Yes;
		}
		BoundOverlap Intersect(OBBox_T<T> const & obb) const
		{
			Vector_T<T, 3> const & center = obb.Center();
			Vector_T<T, 3> const & extent = obb.HalfSize();
			Vector_T<T, 3> const extent_x = extent.x() * obb.Axis(0);
			Vector_T<T, 3> const extent_y = extent.y() * obb.Axis(1);
			Vector_T<T, 3> const extent_z = extent.z() * obb.Axis(2);

			Vector_T<T, 3> min_pt(+1e10f, +1e10f, +1e10f);
			Vector_T<T, 3> max_pt(-1e10f, -1e10f, -1e10f);
			for (int i = 0; i < 8; ++ i)
			{
				Vector_T<T, 3> corner = center + ((i & 1) ? extent_x : -extent_x)
					+ ((i & 2) ? extent_y : -extent_y) + ((i & 4) ? extent_z : -extent_z);

				min_pt = MathLib::minimize(min_pt, corner);
				max_pt = MathLib::maximize(max_pt, corner);
			}

			bool intersect = false;
			for (int i = 0; i < 6; ++ i)
			{
				int const n = vertex_lut_[i];

				// v1 is diagonally opposed to v0
				Vector_T<T, 3> v0((n & 1) ? min_pt.x() : max_pt.x(), (n & 2) ? min_pt.y() : max_pt.y(), (n & 4) ? min_pt.z() : max_pt.z());
				Vector_T<T, 3> v1((n & 1) ? max_pt.x() : min_pt.x(), (n & 2) ? max_pt.y() : min_pt.y(), (n & 4) ? max_pt.z() : min_pt.z());

				if (MathLib::dot_coord(planes_[i], v0) < 0)
				{
					return BO_No;
				}
				if (MathLib::dot_coord(planes_[i], v1) < 0)
				{
					intersect = true;
				}
			}

			return intersect ? BO_Partial : BO_Yes;
		}	
		BoundOverlap Intersect(Sphere_T<T> const & sphere) const
		{
			bool intersect = false;
			for (int i = 0; i < 6; ++ i)
			{
				float d = MathLib::dot_coord(planes_[i], sphere.Center());
				if (d <= -sphere.Radius())
				{
					return BO_No;
				}
				if (d > sphere.Radius())
				{
					intersect = true;
				}
			}

			return intersect ? BO_Partial : BO_Yes;
		}
		BoundOverlap Intersect(Frustum_T<T> const & frustum) const
		{
			bool outside = false;
			bool inside_all = true;
			for (int i = 0; i < 6; ++ i)
			{
				T min_p, max_p;
				min_p = max_p = MathLib::dot_coord(frustum.FrustumPlane(i), corners_[0]);
				for (int j = 1; j < 8; ++ j)
				{
					T tmp = MathLib::dot_coord(frustum.FrustumPlane(i), corners_[j]);
					min_p = std::min(min_p, tmp);
					max_p = std::max(max_p, tmp);
				}

				outside |= (min_p > 0);
				inside_all &= (max_p <= 0);
			}
			if (outside)
			{
				return BO_No;
			}
			if (inside_all)
			{
				return BO_Yes;
			}

			for (int i = 0; i < 6; ++ i)
			{
				T min_p = MathLib::dot_coord(this->FrustumPlane(i), frustum.corners_[0]);
				for (int j = 1; j < 8; ++ j)
				{
					T tmp = MathLib::dot_coord(this->FrustumPlane(i), frustum.corners_[j]);
					min_p = std::min(min_p, tmp);
				}

				outside |= (min_p > 0);
			}
			if (outside)
			{
				return BO_No;
			}

			Vector_T<T, 3> edge_axis_l[6];
			edge_axis_l[0] = corners_[6];
			edge_axis_l[1] = corners_[4];
			edge_axis_l[2] = corners_[5];
			edge_axis_l[3] = corners_[7];
			edge_axis_l[4] = corners_[6] - corners_[5];
			edge_axis_l[5] = corners_[7] - corners_[5];

			Vector_T<T, 3> edge_axis_r[6];
			edge_axis_r[0] = frustum.corners_[6];
			edge_axis_r[1] = frustum.corners_[4];
			edge_axis_r[2] = frustum.corners_[5];
			edge_axis_r[3] = frustum.corners_[7];
			edge_axis_r[4] = frustum.corners_[6] - frustum.corners_[5];
			edge_axis_r[5] = frustum.corners_[7] - frustum.corners_[5];

			for (int i = 0; i < 6; ++ i)
			{
				for (int j = 0; j < 6; ++ j)
				{
					Vector_T<T, 3> Axis = MathLib::cross(edge_axis_l[i], edge_axis_r[j]);

					T min_l, max_l, min_r, max_r;
					min_l = max_l = MathLib::dot(Axis, corners_[0]);
					min_r = max_r = MathLib::dot(Axis, frustum.corners_[0]);
					for (int k = 1; k < 8; ++ k)
					{
						T tmp = MathLib::dot(Axis, corners_[k]);
						min_l = std::min(min_l, tmp);
						max_l = std::max(max_l, tmp);

						tmp = MathLib::dot(Axis, frustum.corners_[k]);
						min_r = std::min(min_r, tmp);
						max_r = std::max(max_r, tmp);
					}

					outside |= min_l > max_r;
					outside |= min_r > max_l;
				}
			}
			if (outside)
			{
				return BO_No;
			}

			return BO_Partial;
		}

	private:
		typedef boost::array<Plane_T<T>, 6> planes_t;
		planes_t planes_;

		// Look-Up Table
		typedef boost::array<int, 6> vertex_lut_t;
		vertex_lut_t vertex_lut_;

		typedef boost::array<Vector_T<T, 3>, 8> corners_t;
		corners_t corners_;
	};
}

#endif			// _KFL_FRUSTUM_HPP
