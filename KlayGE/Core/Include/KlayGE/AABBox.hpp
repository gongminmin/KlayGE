// AABBox.hpp
// KlayGE AABB边框盒 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2004-2005
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

#include <KlayGE/Vector.hpp>
#include <KlayGE/Bound.hpp>

#include <boost/assert.hpp>
#include <boost/operators.hpp>

namespace KlayGE
{
	template <typename T>
	class AABBox_T : boost::addable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::subtractable2<AABBox_T<T>, Vector_T<T, 3>,
						boost::andable<AABBox_T<T>,
						boost::orable<AABBox_T<T>,
						boost::equality_comparable<AABBox_T<T> > > > > >,
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
		AABBox_T(AABBox_T const & rhs)
			: Bound_T<T>(rhs),
				min_(rhs.min_), max_(rhs.max_)
		{
		}

		// 赋值操作符
		AABBox_T& operator+=(Vector_T<T, 3> const & rhs)
		{
			min_ += rhs;
			max_ += rhs;
			return *this;
		}
		AABBox_T& operator-=(Vector_T<T, 3> const & rhs)
		{
			min_ -= rhs;
			max_ -= rhs;
			return *this;
		}
		AABBox_T& operator*=(T const & rhs)
		{
			this->Min() *= rhs;
			this->Max() *= rhs;
			return *this;
		}
		AABBox_T& operator/=(T const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}
		AABBox_T& operator&=(AABBox_T const & rhs)
		{
			min_ = MathLib::maximize(this->Min(), rhs.Min());
			max_ = MathLib::minimize(this->Max(), rhs.Max());
			return *this;
		}
		AABBox_T& operator|=(AABBox_T const & rhs)
		{
			min_ = MathLib::minimize(this->Min(), rhs.Min());
			max_ = MathLib::maximize(this->Max(), rhs.Max());
			return *this;
		}

		AABBox_T& operator=(AABBox_T const & rhs)
		{
			if (this != &rhs)
			{
				this->Min() = rhs.Min();
				this->Max() = rhs.Max();
			}
			return *this;
		}

		bool operator==(AABBox_T const & rhs)
		{
			return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
		}

		// 一元操作符
		AABBox_T const operator+() const
		{
			return *this;
		}
		AABBox_T const operator-() const
		{
			return AABBox_T(-this->Max(), -this->Min());
		}

		Vector_T<T, 3> operator[](size_t i) const
		{
			BOOST_ASSERT(i < 8);

			return Vector_T<T, 3>((i & 1UL) ? this->Max().x() : this->Min().x(),
				(i & 2UL) ? this->Max().y() : this->Min().y(),
				(i & 4UL) ? this->Max().z() : this->Min().z());
		}

		// 属性
		T const & Width() const
		{
			return this->Max().x() - this->Min().x();
		}
		T const & Height() const
		{
			return this->Max().y() - this->Min().y();
		}
		T const & Depth() const
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
			return MathLib::vec_in_box(*this, v);
		}
		T MaxRadiusSq() const
		{
			return std::max<T>(MathLib::length_sq(this->Max()), MathLib::length_sq(this->Min()));
		}

		BoundOverlap CollisionDet(AABBox_T<T> const & aabb) const
		{
			if (((min_.x() <= aabb.Min().x()) && (min_.y() <= aabb.Min().y()) && (min_.z() <= aabb.Min().z())
				&& (max_.x() >= aabb.Max().x()) && (max_.y() >= aabb.Max().y()) && (max_.z() >= aabb.Max().z()))
				|| ((min_.x() >= aabb.Min().x()) && (min_.y() >= aabb.Min().y()) && (min_.z() >= aabb.Min().z())
					&& (max_.x() <= aabb.Max().x()) && (max_.y() <= aabb.Max().y()) && (max_.z() <= aabb.Max().z())))
			{
				return BO_Yes;
			}
			else
			{
				float3 const t = aabb.Center() - this->Center();
				float3 const e = this->HalfSize() + aabb.HalfSize();
				return ((MathLib::abs(t.x()) <= e.x()) && (MathLib::abs(t.y()) <= e.y()) && (MathLib::abs(t.z()) <= e.z())) ? BO_Partial : BO_No;
			}
		}

	private:
		Vector_T<T, 3> min_, max_;
	};

	typedef AABBox_T<float> AABBox;
}

#endif			// _BOX_HPP
