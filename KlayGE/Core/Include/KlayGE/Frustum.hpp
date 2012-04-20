// Frustum.hpp
// KlayGE 视锥类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2004-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 移致Core中 (2010.5.23)
//
// 2.5.0
// 改为LUT实现 (2005.3.30)
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _FRUSTUM_HPP
#define _FRUSTUM_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6385)
#endif
#include <boost/array.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

namespace KlayGE
{
	template <typename T>
	class KLAYGE_CORE_API Frustum_T : public Bound_T<T>
	{
	public:
		void ClipMatrix(Matrix4_T<T> const & clip)
		{
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
			BOOST_FOREACH(typename planes_t::reference plane, planes_)
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
			Vector_T<T, 3> const & half_size = obb.HalfSize();
			Vector_T<T, 3> diag = half_size[0] * obb.Axis(0) + half_size[1] * obb.Axis(1) + half_size[2] * obb.Axis(2);
			for (int i = 0; i < 3; ++ i)
			{
				if (diag[i] < 0)
				{
					diag[i] = -diag[i];
				}
			}

			Vector_T<T, 3> const min_pt = center - diag;
			Vector_T<T, 3> const max_pt = center + diag;

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
			UNREF_PARAM(frustum);
			BOOST_ASSERT(false);

			return BO_No;
		}

	private:
		typedef boost::array<Plane_T<T>, 6> planes_t;
		planes_t planes_;

		// Look-Up Table
		typedef boost::array<int, 6> vertex_lut_t;
		vertex_lut_t vertex_lut_;
	};

	typedef Frustum_T<float> Frustum;
}

#endif			// _FRUSTUM_HPP
