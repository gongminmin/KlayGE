// Sphere.hpp
// KlayGE 边框球体 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// 改为模板 (2005.4.12)
//
// 2.1.1
// 初次建立 (2004.4.30)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _SPHERE_HPP
#define _SPHERE_HPP

#include <KlayGE/PreDeclare.hpp>

#include <boost/operators.hpp>

#include <KlayGE/Bound.hpp>

#pragma once

namespace KlayGE
{
	template <typename T>
	class Sphere_T : boost::addable2<Sphere_T<T>, Vector_T<T, 3>,
						boost::subtractable2<Sphere_T<T>, Vector_T<T, 3>,
						boost::multipliable2<Sphere_T<T>, T,
						boost::dividable2<Sphere_T<T>, T,
						boost::equality_comparable<Sphere_T<T> > > > > >,
				public Bound_T<T>
	{
	public:
		Sphere_T();
		Sphere_T(Vector_T<T, 3> const & center, T const & radius);

		// 赋值操作符
		Sphere_T& operator+=(Vector_T<T, 3> const & rhs);
		Sphere_T& operator-=(Vector_T<T, 3> const & rhs);
		Sphere_T& operator*=(T const & rhs);
		Sphere_T& operator/=(T const & rhs);

		Sphere_T& operator=(Sphere_T const & rhs);

		// 一元操作符
		Sphere_T const & operator+() const;
		Sphere_T const & operator-() const;

		// 属性
		Vector_T<T, 3>& Center();
		Vector_T<T, 3> const & Center() const;
		T& Radius();
		T Radius() const;

		bool IsEmpty() const;

		bool VecInBound(Vector_T<T, 3> const & v) const;
		T MaxRadiusSq() const;

		bool Intersect(AABBox_T<T> const & aabb) const;
		bool Intersect(OBBox_T<T> const & obb) const;
		bool Intersect(Sphere_T<T> const & sphere) const;
		bool Intersect(Frustum_T<T> const & frustum) const;

		friend bool
		operator==(Sphere_T<T> const & lhs, Sphere_T<T> const & rhs)
		{
			return (lhs.center_ == rhs.center_) && (rhs.radius_ == rhs.radius_);
		}

	private:
		Vector_T<T, 3> center_;
		T radius_;
	};

	typedef Sphere_T<float> Sphere;
}

#endif			// _SPHERE_HPP
