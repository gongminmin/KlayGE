// Box.hpp
// KlayGE AABB边框盒 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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

#ifndef _BOX_HPP
#define _BOX_HPP

#include <KlayGE/Math.hpp>
#include <KlayGE/Bound.hpp>
#include <boost/operators.hpp>

namespace KlayGE
{
	template <typename T>
	class Box_T : boost::addable2<Box_T<T>, Vector_T<T, 3>, 
						boost::subtractable2<Box, Vector_T<T, 3>,
						boost::andable<Box_T<T>,
						boost::orable<Box_T<T>,
						boost::equality_comparable<Box_T<T> > > > > >,
				public Bound_T<T>
	{
	public:
		Box_T()
		{
		}
		Box_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax)
		{
			min_ = MathLib::Minimize(vMin, vMax);
			max_ = MathLib::Maximize(vMin, vMax);
		}

		// 赋值操作符
		Box_T& operator+=(Vector_T<T, 3> const & rhs)
		{
			min_ += rhs;
			max_ += rhs;
			return *this;
		}
		Box_T& operator-=(Vector_T<T, 3> const & rhs)
		{
			min_ -= rhs;
			max_ -= rhs;
			return *this;
		}
		Box_T& operator*=(T const & rhs)
		{
			this->Min() *= rhs;
			this->Max() *= rhs;
			return *this;
		}
		Box_T& operator/=(T const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}
		Box_T& operator&=(Box_T const & rhs)
		{
			min_ = MathLib::Maximize(this->Min(), rhs.Min());
			max_ = MathLib::Minimize(this->Max(), rhs.Max());
			return *this;
		}
		Box_T& operator|=(Box_T const & rhs)
		{
			min_ = MathLib::Minimize(this->Min(), rhs.Min());
			max_ = MathLib::Maximize(this->Max(), rhs.Max());
			return *this;
		}

		Box_T& operator=(Box_T const & rhs)
		{
			if (this != &rhs)
			{
				this->Min() = rhs.Min();
				this->Max() = rhs.Max();
			}
			return *this;
		}

		bool operator==(Box_T const & rhs)
		{
			return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
		}

		// 一元操作符
		Box_T const operator+() const
		{
			return *this;
		}
		Box_T const operator-() const
		{
			return Box_T(-this->Min(), -this->Max());
		}

		Vector_T<T, 3> operator[](size_t i) const
		{
			switch (i)
			{
			case 0:
				return this->LeftBottomNear();

			case 1:
				return this->LeftTopNear();

			case 2:
				return this->RightTopNear();

			case 3:
				return this->RightBottomNear();

			case 4:
				return this->LeftBottomFar();

			case 5:
				return this->LeftTopFar();

			case 6:
				return this->RightTopFar();

			case 7:
				return this->RightBottomFar();

			default:
				assert(false);
				return Vector_T<T, 3>::Zero();
			}
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
			return (min_ + max_) / 2;
		}

		bool VecInBound(Vector_T<T, 3> const & v) const
		{
			return MathLib::VecInBox(*this, v);
		}
		T MaxRadiusSq() const
		{
			return std::max<T>(MathLib::LengthSq(this->Max()), MathLib::LengthSq(this->Min()));
		}

	private:
		Vector_T<T, 3> min_, max_;
	};

	typedef Box_T<float> Box;
}

#endif			// _BOX_HPP
