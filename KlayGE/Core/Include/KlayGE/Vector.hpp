// Vector.hpp
// KlayGE 向量 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _VECTOR_HPP
#define _VECTOR_HPP

#include <algorithm>
#include <functional>

#include <boost/static_assert.hpp>
#include <boost/array.hpp>
#include <boost/operators.hpp>

namespace KlayGE
{
	template <typename T, int N>
	class Vector_T : boost::addable<Vector_T<T, N>,
						boost::subtractable<Vector_T<T, N>,
						boost::dividable2<Vector_T<T, N>, T,
						boost::multipliable2<Vector_T<T, N>, T> > > >
	{
		template <typename U, int M>
		friend class Vector_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { elem_num = N };

	public:
		Vector_T()
			{ }
		explicit Vector_T(const T* rhs)
			{ std::copy(rhs, rhs + N, this->begin()); }
		Vector_T(const Vector_T& rhs)
			: vec_(rhs.vec_)
			{ }
		template <typename U>
		Vector_T(const Vector_T<U, N>& rhs)
			: vec_(rhs.vec_)
			{ }

		static size_t size()
			{ return elem_num; }

		static const Vector_T& Zero()
		{
			static Vector_T<T, N> zero;
			zero.vec_.assign(T(0));
			return zero;
		}

		// 取向量
		iterator begin()
			{ return vec_.begin(); }
		const_iterator begin() const
			{ return vec_.begin(); }
		iterator end()
			{ return vec_.end(); }
		const_iterator end() const
			{ return vec_.end(); }
		reference operator[](size_t index)
			{ return vec_[index]; }
		const_reference operator[](size_t index) const
			{ return vec_[index]; }

		reference x()
		{
			BOOST_STATIC_ASSERT(elem_num >= 1);
			return vec_[0];
		}
		const_reference x() const
		{
			BOOST_STATIC_ASSERT(elem_num >= 1);
			return vec_[0];
		}

		reference y()
		{
			BOOST_STATIC_ASSERT(elem_num >= 2);
			return vec_[1];
		}
		const_reference y() const
		{
			BOOST_STATIC_ASSERT(elem_num >= 2);
			return vec_[1];
		}

		reference z()
		{
			BOOST_STATIC_ASSERT(elem_num >= 3);
			return vec_[2];
		}
		const_reference z() const
		{
			BOOST_STATIC_ASSERT(elem_num >= 3);
			return vec_[2];
		}

		reference w()
		{
			BOOST_STATIC_ASSERT(elem_num >= 4);
			return vec_[3];
		}
		const_reference w() const
		{
			BOOST_STATIC_ASSERT(elem_num >= 4);
			return vec_[3];
		}

		// 赋值操作符
		template <typename U>
		Vector_T& operator+=(const Vector_T<U, N>& rhs)
		{
			std::transform(this->begin(), this->end(), rhs.begin(), this->begin(), std::plus<T>());
			return *this;
		}
		template <typename U>
		Vector_T& operator-=(const Vector_T<U, N>& rhs)
		{
			std::transform(this->begin(), this->end(), rhs.begin(), this->begin(), std::minus<T>());
			return *this;
		}
		template <typename U>
		Vector_T& operator*=(const U& rhs)
		{
			std::transform(this->begin(), this->end(), this->begin(), std::bind2nd(std::multiplies<T>(), rhs));
			return *this;
		}
		template <typename U>
		Vector_T& operator/=(const U& rhs)
		{
			return this->operator*=(1.0f / rhs);
		}

		Vector_T& operator=(const Vector_T& rhs)
		{
			if (this != &rhs)
			{
				vec_ = rhs.vec_;
			}
			return *this;
		}
		template <typename U>
		Vector_T& operator=(const Vector_T<U, N>& rhs)
		{
			if (this != &rhs)
			{
				vec_ = rhs.vec_;
			}
			return *this;
		}

		// 一元操作符
		const Vector_T operator+() const
			{ return *this; }
		const Vector_T operator-() const
		{
			Vector_T temp(*this);
			std::transform(temp.begin(), temp.end(), temp.begin(), std::negate<T>());
			return temp;
		}

	private:
		boost::array<T, N> vec_;
	};

	template <typename T, int N>
	inline bool
	operator==(const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs)
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}
	template <typename T, int N>
	inline bool
	operator!=(const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Vector_T<float, 2> Vector2;
	typedef Vector_T<float, 3> Vector3;
	typedef Vector_T<float, 4> Vector4;

	template <typename T>
	Vector_T<T, 2> MakeVector(const T& x, const T& y)
	{
		T data[2] = { x, y };
		return Vector_T<T, 2>(data);
	}

	template <typename T>
	Vector_T<T, 3> MakeVector(const T& x, const T& y, const T& z)
	{
		T data[3] = { x, y, z };
		return Vector_T<T, 3>(data);
	}

	template <typename T>
	Vector_T<T, 4> MakeVector(const T& x, const T& y, const T& z, const T& w)
	{
		T data[4] = { x, y, z, w };
		return Vector_T<T, 4>(data);
	}
}

#endif			// _VECTOR_HPP
