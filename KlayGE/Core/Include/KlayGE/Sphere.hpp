// Sphere.hpp
// KlayGE 边框球体 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.30)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _SPHERE_HPP
#define _SPHERE_HPP

#include <boost/operators.hpp>

#include <KlayGE/Bound.hpp>

namespace KlayGE
{
	class Sphere : boost::addable2<Sphere, Vector3, 
						boost::subtractable2<Sphere, Vector3,
						boost::andable<Sphere,
						boost::orable<Sphere,
						boost::equality_comparable<Sphere> > > > >,
				public Bound
	{
	public:
		Sphere()
		{
		}
		Sphere(Vector3 const & center, float radius)
			: center_(center),
				radius_(radius)
		{
		}

		// 赋值操作符
		Sphere& operator+=(Vector3 const & rhs)
		{
			this->Center() += rhs;
			return *this;
		}
		Sphere& operator-=(Vector3 const & rhs)
		{
			this->Center() -= rhs;
			return *this;
		}
		Sphere& operator*=(float rhs)
		{
			this->Radius() *= rhs;
			return *this;
		}
		Sphere& operator/=(float rhs)
		{
			return this->operator*=(1.0f / rhs);
		}

		Sphere& operator=(Sphere const & rhs)
		{
			if (this != &rhs)
			{
				this->Center() = rhs.Center();
				this->Radius() = rhs.Radius();
			}
			return *this;
		}

		// 一元操作符
		Sphere const & operator+() const
		{
			return *this;
		}
		Sphere const & operator-() const
		{
			return *this;
		}

		// 属性
		Vector3& Center()
		{
			return center_;
		}
		Vector3 const & Center() const
		{
			return center_;
		}
		float& Radius()
		{
			return radius_;
		}
		float Radius() const
		{
			return radius_;
		}

		bool IsEmpty() const
		{
			return MathLib::Eq(radius_, 0.0f);
		}

		bool VecInBound(Vector3 const & v) const
		{
			return MathLib::VecInSphere(*this, v);
		}
		float MaxRadiusSq() const
		{
			return this->Radius() * this->Radius();
		}

		bool operator==(Sphere const & rhs)
		{
			return (center_ == rhs.center_) && (radius_ == rhs.radius_);
		}

	private:
		Vector3 center_;
		float radius_;
	};
}

#endif			// _SPHERE_HPP
