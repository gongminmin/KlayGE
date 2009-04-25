// Plane.hpp
// KlayGE 平面 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _PLANE_HPP
#define _PLANE_HPP

#include <boost/operators.hpp>

#include <KlayGE/Vector.hpp>

#pragma once

namespace KlayGE
{
	// 描述一个平面 ax + by + cz + d = 0
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Plane_T : boost::equality_comparable<Plane_T<T> >
	{
		template <typename U>
		friend class Plane_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		enum { elem_num = 4 };

	public:
		Plane_T()
			{ }
		explicit Plane_T(T const * rhs)
			: plane_(rhs)
			{ }
		Plane_T(Plane_T const & rhs)
			: plane_(rhs.plane_)
			{ }
		template <typename U>
		Plane_T(Plane_T<U> const & rhs)
			: plane_(rhs.plane_)
			{ }
		template <typename U>
		Plane_T(Vector_T<U, elem_num> const & rhs)
		{
			plane_ = rhs;
		}
		Plane_T(T const & a, T const & b, T const & c, T const & d)
		{
			this->a() = a;
			this->b() = b;
			this->c() = c;
			this->d() = d;
		}

		// 取向量
		iterator begin()
			{ return plane_.begin(); }
		const_iterator begin() const
			{ return plane_.begin(); }
		iterator end()
			{ return plane_.end(); }
		const_iterator end() const
			{ return plane_.end(); }
		reference operator[](size_t index)
			{ return plane_[index]; }
		const_reference operator[](size_t index) const
			{ return plane_[index]; }

		reference a()
			{ return plane_[0]; }
		const_reference a() const
			{ return plane_[0]; }
		reference b()
			{ return plane_[1]; }
		const_reference b() const
			{ return plane_[1]; }
		reference c()
			{ return plane_[2]; }
		const_reference c() const
			{ return plane_[2]; }
		reference d()
			{ return plane_[3]; }
		const_reference d() const
			{ return plane_[3]; }

		// 赋值操作符
		Plane_T& operator=(Plane_T const & rhs)
		{
			if (this != &rhs)
			{
				plane_ = rhs.plane_;
			}
			return *this;
		}
		template <typename U>
		Plane_T& operator=(Plane_T<U> const & rhs)
		{
			if (this != &rhs)
			{
				plane_ = rhs.plane_;
			}
			return *this;
		}
		template <typename U>
		Plane_T& operator=(Vector_T<U, elem_num> const & rhs)
		{
			plane_ = rhs;
			return *this;
		}

		// 一元操作符
		Plane_T const operator+() const
			{ return *this; }
		Plane_T const operator-() const
			{ return Plane_T<T>(-this->a(), -this->b(), -this->c(), -this->d()); }

		// 取法向向量
		Vector_T<T, 3> const Normal() const
			{ return Vector_T<T, 3>(this->a(), this->b(), this->c()); }
		template <typename U>
		void Normal(Vector_T<U, 3> const & rhs)
		{
			this->a() = rhs.x();
			this->b() = rhs.y();
			this->c() = rhs.z();
		}

		bool operator==(Plane_T<T> const & rhs)
		{
			return plane_ == rhs.plane_;
		}

	private:
		Vector_T<T, elem_num> plane_;
	};

	typedef Plane_T<float> Plane;
}

#endif			// _PLANE_HPP
