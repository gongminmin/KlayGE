// AABBox.hpp
// KlayGE AABB header file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2004-2012
// Homepage: http://www.klayge.org
//
// 2.5.0
// 改为模板 (2005.4.12)
//
// 2.4.0
// 增加了Center和operator[] (2005.3.20)
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _AABBOX_HPP
#define _AABBOX_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <boost/operators.hpp>

#include <KlayGE/Bound.hpp>

namespace KlayGE
{
	template <typename T>
	class AABBox_T : boost::addable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::subtractable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::multipliable2<AABBox_T<T>, T,
						boost::dividable2<AABBox_T<T>, T,
						boost::andable<AABBox_T<T>,
						boost::orable<AABBox_T<T>,
						boost::equality_comparable<AABBox_T<T> > > > > > > >,
				public Bound_T<T>
	{
	public:
		AABBox_T();
		AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax);
		AABBox_T(AABBox_T<T> const & rhs);

		// 赋值操作符
		AABBox_T<T>& operator+=(Vector_T<T, 3> const & rhs);
		AABBox_T<T>& operator-=(Vector_T<T, 3> const & rhs);
		AABBox_T<T>& operator*=(T const & rhs);
		AABBox_T<T>& operator/=(T const & rhs);
		AABBox_T<T>& operator&=(AABBox_T<T> const & rhs);
		AABBox_T<T>& operator|=(AABBox_T<T> const & rhs);

		AABBox_T<T>& operator=(AABBox_T<T> const & rhs);

		// 一元操作符
		AABBox_T<T> const operator+() const;
		AABBox_T<T> const operator-() const;

		Vector_T<T, 3> operator[](size_t i) const;

		// 属性
		T Width() const;
		T Height() const;
		T Depth() const;
		bool IsEmpty() const;

		Vector_T<T, 3> const LeftBottomNear() const;
		Vector_T<T, 3> const LeftTopNear() const;
		Vector_T<T, 3> const RightBottomNear() const;
		Vector_T<T, 3> const RightTopNear() const;
		Vector_T<T, 3> const LeftBottomFar() const;
		Vector_T<T, 3> const LeftTopFar() const;
		Vector_T<T, 3> const RightBottomFar() const;
		Vector_T<T, 3> const RightTopFar() const;

		Vector_T<T, 3>& Min();
		Vector_T<T, 3> const & Min() const;
		Vector_T<T, 3>& Max();
		Vector_T<T, 3> const & Max() const;
		Vector_T<T, 3> Center() const;
		Vector_T<T, 3> HalfSize() const;

		bool VecInBound(Vector_T<T, 3> const & v) const;
		T MaxRadiusSq() const;

		BoundOverlap CollisionDet(AABBox_T<T> const & aabb) const;
		BoundOverlap CollisionDet(OBBox_T<T> const & obb) const;
		BoundOverlap CollisionDet(Sphere_T<T> const & sphere) const;
		BoundOverlap CollisionDet(Frustum_T<T> const & frustum) const;

		friend bool
		operator==(AABBox_T<T> const & lhs, AABBox_T<T> const & rhs)
		{
			return (lhs.Min() == rhs.Min()) && (rhs.Max() == rhs.Max());
		}

	private:
		Vector_T<T, 3> min_, max_;
	};

	typedef AABBox_T<float> AABBox;
}

#endif			// _AABBOX_HPP
