// Box.hpp
// KlayGE AABB边框盒 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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
	class Box : boost::addable2<Box, Vector3, 
						boost::subtractable2<Box, Vector3,
						boost::andable<Box,
						boost::orable<Box,
						boost::equality_comparable<Box> > > > >,
				public Bound
	{
	public:
		Box()
		{
		}
		Box(Vector3 const & vMin, Vector3 const & vMax)
		{
			MathLib::Minimize(min_, vMin, vMax);
			MathLib::Maximize(max_, vMin, vMax);
		}

		// 赋值操作符
		Box& operator+=(Vector3 const & rhs)
		{
			min_ += rhs;
			max_ += rhs;
			return *this;
		}
		Box& operator-=(Vector3 const & rhs)
		{
			min_ -= rhs;
			max_ -= rhs;
			return *this;
		}
		Box& operator*=(float rhs)
		{
			this->Min() *= rhs;
			this->Max() *= rhs;
			return *this;
		}
		Box& operator/=(float rhs)
		{
			return this->operator*=(1.0f / rhs);
		}
		Box& operator&=(Box const & rhs)
		{
			MathLib::Maximize(this->Min(), this->Min(), rhs.Min());
			MathLib::Minimize(this->Max(), this->Max(), rhs.Max());
			return *this;
		}
		Box& operator|=(Box const & rhs)
		{
			MathLib::Minimize(this->Min(), this->Min(), rhs.Min());
			MathLib::Maximize(this->Max(), this->Max(), rhs.Max());
			return *this;
		}

		Box& operator=(Box const & rhs)
		{
			if (this != &rhs)
			{
				this->Min() = rhs.Min();
				this->Max() = rhs.Max();
			}
			return *this;
		}

		bool operator==(Box const & rhs)
		{
			return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
		}

		// 一元操作符
		Box const operator+() const
		{
			return *this;
		}
		Box const operator-() const
		{
			return Box(-this->Min(), -this->Max());
		}

		Vector3 operator[](size_t i) const
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
				return Vector3::Zero();
			}
		}

		// 属性
		float Width() const
		{
			return this->Max().x() - this->Min().x();
		}
		float Height() const
		{
			return this->Max().y() - this->Min().y();
		}
		float Depth() const
		{
			return this->Max().z() - this->Min().z();
		}
		bool IsEmpty() const
		{
			return this->Min() == this->Max();
		}

		Vector3 const LeftBottomNear() const
		{
			return this->Min();
		}
		Vector3 const LeftTopNear() const
		{
			return Vector3(this->Min().x(), this->Max().y(), this->Min().z());
		}
		Vector3 const RightBottomNear() const
		{
			return Vector3(this->Max().x(), this->Min().y(), this->Min().z());
		}
		Vector3 const RightTopNear() const
		{
			return Vector3(this->Max().x(), this->Max().y(), this->Min().z());
		}
		Vector3 const LeftBottomFar() const
		{
			return Vector3(this->Min().x(), this->Min().y(), this->Max().z());
		}
		Vector3 const LeftTopFar() const
		{
			return Vector3(this->Min().x(), this->Max().y(), this->Max().z());
		}
		Vector3 const RightBottomFar() const
		{
			return Vector3(this->Max().x(), this->Min().y(), this->Max().z());
		}
		Vector3 const RightTopFar() const
		{
			return this->Max();
		}

		Vector3& Min()
		{
			return min_;
		}
		Vector3 const & Min() const
		{
			return min_;
		}
		Vector3& Max()
		{
			return max_;
		}
		Vector3 const & Max() const
		{
			return max_;
		}
		Vector3 Center() const
		{
			return (min_ + max_) / 2;
		}

		bool VecInBound(Vector3 const & v) const
		{
			return MathLib::VecInBox(*this, v);
		}
		float MaxRadiusSq() const
		{
			return std::max<float>(MathLib::LengthSq(this->Max()), MathLib::LengthSq(this->Min()));
		}

	private:
		Vector3 min_, max_;
	};
}

#endif			// _BOX_HPP
