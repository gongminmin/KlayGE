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

#include <boost/assert.hpp>
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
		AABBox_T()
		{
		}
		AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax)
				: min_(vMin), max_(vMax)
		{
			BOOST_ASSERT(vMin.x() <= vMax.x());
			BOOST_ASSERT(vMin.y() <= vMax.y());
			BOOST_ASSERT(vMin.z() <= vMax.z());
		}
		AABBox_T(AABBox_T<T> const & rhs)
				: Bound_T<T>(rhs),
				min_(rhs.min_), max_(rhs.max_)
		{
		}

		// 赋值操作符
		AABBox_T<T>& operator+=(Vector_T<T, 3> const & rhs)
		{
			min_ += rhs;
			max_ += rhs;
			return *this;
		}
		AABBox_T<T>& operator-=(Vector_T<T, 3> const & rhs)
		{
			min_ -= rhs;
			max_ -= rhs;
			return *this;
		}
		AABBox_T<T>& operator*=(T const & rhs)
		{
			this->Min() *= rhs;
			this->Max() *= rhs;
			return *this;
		}
		AABBox_T<T>& operator/=(T const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}
		AABBox_T<T>& operator&=(AABBox_T<T> const & rhs)
		{
			min_ = MathLib::maximize(this->Min(), rhs.Min());
			max_ = MathLib::minimize(this->Max(), rhs.Max());
			return *this;
		}
		AABBox_T<T>& operator|=(AABBox_T<T> const & rhs)
		{
			min_ = MathLib::minimize(this->Min(), rhs.Min());
			max_ = MathLib::maximize(this->Max(), rhs.Max());
			return *this;
		}

		AABBox_T<T>& operator=(AABBox_T<T> const & rhs)
		{
			if (this != &rhs)
			{
				this->Min() = rhs.Min();
				this->Max() = rhs.Max();
			}
			return *this;
		}

		// 一元操作符
		AABBox_T<T> const operator+() const
		{
			return *this;
		}
		AABBox_T<T> const operator-() const
		{
			return AABBox_T<T>(-this->Max(), -this->Min());
		}

		Vector_T<T, 3> operator[](size_t i) const
		{
			BOOST_ASSERT(i < 8);

			return Vector_T<T, 3>((i & 1UL) ? this->Max().x() : this->Min().x(),
				(i & 2UL) ? this->Max().y() : this->Min().y(),
				(i & 4UL) ? this->Max().z() : this->Min().z());
		}

		// 属性
		T Width() const
		{
			return this->Max().x() - this->Min().x();
		}
		T Height() const
		{
			return this->Max().y() - this->Min().y();
		}
		T Depth() const
		{
			return this->Max().z() - this->Min().z();
		}
		bool IsEmpty() const
		{
			return this->Min() == this->Max();
		}

		Vector_T<T, 3> const LeftBottomNear() const
		{
			return this->Min();
		}
		Vector_T<T, 3> const LeftTopNear() const
		{
			return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Min().z());
		}
		Vector_T<T, 3> const RightBottomNear() const
		{
			return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Min().z());
		}
		Vector_T<T, 3> const RightTopNear() const
		{
			return Vector_T<T, 3>(this->Max().x(), this->Max().y(), this->Min().z());
		}
		Vector_T<T, 3> const LeftBottomFar() const
		{
			return Vector_T<T, 3>(this->Min().x(), this->Min().y(), this->Max().z());
		}
		Vector_T<T, 3> const LeftTopFar() const
		{
			return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Max().z());
		}
		Vector_T<T, 3> const RightBottomFar() const
		{
			return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Max().z());
		}
		Vector_T<T, 3> const RightTopFar() const
		{
			return this->Max();
		}

		Vector_T<T, 3>& Min()
		{
			return min_;
		}
		Vector_T<T, 3> const & Min() const
		{
			return min_;
		}
		Vector_T<T, 3>& Max()
		{
			return max_;
		}
		Vector_T<T, 3> const & Max() const
		{
			return max_;
		}
		Vector_T<T, 3> Center() const
		{
			return (min_ + max_) / 2.0f;
		}
		Vector_T<T, 3> HalfSize() const
		{
			return (max_ - min_) / 2.0f;
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::intersect_point_aabb(v, *this);
		}
		T MaxRadiusSq() const
		{
			return std::max<T>(MathLib::length_sq(this->Max()), MathLib::length_sq(this->Min()));
		}

		bool Intersect(AABBox_T<T> const & aabb) const
		{
			return MathLib::intersect_aabb_aabb(*this, aabb);
		}
		bool Intersect(OBBox_T<T> const & obb) const
		{
			return MathLib::intersect_aabb_obb(*this, obb);
		}
		bool Intersect(Sphere_T<T> const & sphere) const
		{
			return MathLib::intersect_aabb_sphere(*this, sphere);
		}
		bool Intersect(Frustum_T<T> const & frustum) const
		{
			return MathLib::intersect_aabb_frustum(*this, frustum) != BO_No;
		}

		friend bool
		operator==(AABBox_T<T> const & lhs, AABBox_T<T> const & rhs)
		{
			return (lhs.Min() == rhs.Min()) && (rhs.Max() == rhs.Max());
		}

	private:
		Vector_T<T, 3> min_, max_;
	};
}

#endif			// _AABBOX_HPP
