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

#include <KlayGE/Vector.hpp>

namespace KlayGE
{
	// 描述一个平面 ax + by + cz + d = 0
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Plane_T
	{
		template <typename U>
		friend class Plane_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { elem_num = 4 };

	public:
		Plane_T()
			{ }
		explicit Plane_T(const T* rhs)
			: plane_(rhs)
			{ }
		Plane_T(const Plane_T& rhs)
			: plane_(rhs.plane_)
			{ }
		template <typename U>
		Plane_T(const Plane_T<U>& rhs)
			: plane_(rhs.plane_)
			{ }
		Plane_T(const T& a, const T& b, const T& c, const T& d)
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
		Plane_T& operator=(const Plane_T& rhs)
		{
			if (this != &rhs)
			{
				plane_ = rhs.plane_;
			}
			return *this;
		}
		template <typename U>
		Plane_T& operator=(const Plane_T<U>& rhs)
		{
			if (this != &rhs)
			{
				plane_ = rhs.plane_;
			}
			return *this;
		}

		// 一元操作符
		const Plane_T operator+() const
			{ return *this; }
		const Plane_T operator-() const
			{ return Plane_T<T>(-this->a(), -this->b(), -this->c(), -this->d()); }

		// 取法向向量
		const Vector_T<T, 3> Normal() const
			{ return Vector_T<T, 3>(this->a(), this->b(), this->c()); }
		template <typename U>
		void Normal(const Vector_T<U, 3>& rhs)
		{
			this->a() = rhs.x();
			this->b() = rhs.y();
			this->c() = rhs.z();
		}

	private:
		Vector_T<T, elem_num> plane_;
	};

	template <typename T>
	inline bool
	operator==(const Plane_T<T>& lhs, const Plane_T<T>& rhs)
	{
		return (lhs.a() == rhs.a()) && (lhs.b() == rhs.b()) && (lhs.c() == rhs.c()) && (lhs.d() == rhs.d());
	}
	template <typename T>
	inline bool
	operator!=(const Plane_T<T>& lhs, const Plane_T<T>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Plane_T<float> Plane;
}

#endif			// _PLANE_HPP
