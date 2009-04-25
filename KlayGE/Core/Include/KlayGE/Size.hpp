// Plane.hpp
// KlayGE 矩形大小 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _SIZE_HPP
#define _SIZE_HPP

#include <boost/operators.hpp>

#include <KlayGE/Vector.hpp>

#pragma once

namespace KlayGE
{
	template <typename T>
	class Size_T : boost::addable<Size_T<T>,
						boost::subtractable<Size_T<T>,
						boost::equality_comparable<Size_T<T> > > >
	{
		template <typename U>
		friend class Size_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		enum { elem_num = 2 };

	public:
		Size_T()
			{ }
		explicit Size_T(T const * rhs)
			: size_(rhs)
			{ }
		Size_T(const Size_T& rhs)
			: size_(rhs.size_)
			{ }
		template <typename U>
		Size_T(Size_T<U> const & rhs)
			: size_(rhs.size_)
			{ }
		Size_T(T const & cx, T const & cy)
		{
			this->cx() = cx;
			this->cy() = cy;
		}

		// 取向量
		reference cx()
			{ return size_[0]; }
		const_reference cx() const
			{ return size_[0]; }
		reference cy()
			{ return size_[1]; }
		const_reference cy() const
			{ return size_[1]; }

		// 赋值操作符
		template <typename U>
		Size_T const & operator+=(Size_T<U> const & rhs)
		{
			size_ += rhs.size_;
			return *this;
		}
		template <typename U>
		Size_T const & operator-=(Size_T<U> const & rhs)
		{
			size_ -= rhs.size_;
			return *this;
		}

		Size_T& operator=(const Size_T& rhs)
		{
			if (this != &rhs)
			{
				size_ = rhs.size_;
			}
			return *this;
		}
		template <typename U>
		Size_T& operator=(Size_T<U> const & rhs)
		{
			if (this != &rhs)
			{
				size_ = rhs.size_;
			}
			return *this;
		}

		// 一元操作符
		Size_T<T> const operator+() const
			{ return *this; }
		Size_T<T> const operator-() const
			{ return Size_T<T>(-this->cx(), -this->cy()); }

		bool operator==(Size_T<T> const & rhs)
		{
			return size_ == rhs.size_;
		}

	private:
		Vector_T<T, elem_num> size_;
	};

	typedef Size_T<float> Size;
}

#endif			// _SIZE_HPP
