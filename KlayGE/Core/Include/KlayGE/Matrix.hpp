// Matrix.hpp
// KlayGE 矩阵 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _MATRIX_HPP
#define _MATRIX_HPP

#include <boost/operators.hpp>

#include <KlayGE/Vector.hpp>

namespace KlayGE
{
	// 4D 矩阵
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Matrix4_T : boost::addable<Matrix4_T<T>,
						boost::subtractable<Matrix4_T<T>,
						boost::dividable2<Matrix4_T<T>, T,
						boost::multipliable2<Matrix4_T<T>, T> > > >
	{
		template <typename U>
		friend class Matrix4_T;

	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { row_num = 4, col_num = 4 };
		enum { elem_num = row_num * col_num };

	public:
		Matrix4_T()
			{ }
		explicit Matrix4_T(const T* rhs)
		{
			for (size_t i = 0; i < row_num; ++ i)
			{
				m_[i] = Vector_T<T, col_num>(rhs);
				rhs += col_num;
			}
		}
		Matrix4_T(const Matrix4_T& rhs)
			: m_(rhs.m_)
			{ }
		template <typename U>
		Matrix4_T(const Matrix4_T<U>& rhs)
			: m_(rhs.m_)
			{ }
		Matrix4_T(const T& f11, const T& f12, const T& f13, const T& f14,
			const T& f21, const T& f22, const T& f23, const T& f24,
			const T& f31, const T& f32, const T& f33, const T& f34,
			const T& f41, const T& f42, const T& f43, const T& f44)
		{
			m_[0][0] = f11;	m_[0][1] = f12;	m_[0][2] = f13;	m_[0][3] = f14;
			m_[1][0] = f21;	m_[1][1] = f22;	m_[1][2] = f23;	m_[1][3] = f24;
			m_[2][0] = f31;	m_[2][1] = f32;	m_[2][2] = f33;	m_[2][3] = f34;
			m_[3][0] = f41;	m_[3][1] = f42;	m_[3][2] = f43;	m_[3][3] = f44;
		}

		static size_t size()
			{ return elem_num; }

		static Matrix4_T Identity()
		{
			Matrix4_T out;
			for (size_t y = 0; y < row_num; ++ y)
			{
				for (size_t x = 0; x < col_num; ++ x)
				{
					if (x == y)
					{
						out.m_[y][x] = 1;
					}
					else
					{
						out.m_[y][x] = 0;
					}
				}
			}
			return out;
		}

		reference operator()(size_t row, size_t col)
			{ return m_[row][col]; }
		const_reference operator()(size_t row, size_t col) const
			{ return m_[row][col]; }
		iterator begin()
			{ return &m_[0][0]; }
		const_iterator begin() const
			{ return &m_[0][0]; }
		iterator end()
			{ return this->begin() + elem_num; }
		const_iterator end() const
			{ return this->begin() + elem_num; }
		reference operator[](size_t index)
			{ return *(this->begin() + index); }
		const_reference operator[](size_t index) const
			{ return *(this->begin() + index); }

		Vector_T<T, col_num>& Row(size_t index)
			{ return m_[index]; }
		const Vector_T<T, col_num>& Row(size_t index) const
			{ return m_[index]; }

		// 赋值操作符
		template <typename U>
		Matrix4_T& operator+=(const Matrix4_T<U>& rhs)
		{
			m_ += rhs.m_;
			return *this;
		}
		template <typename U>
		Matrix4_T& operator-=(const Matrix4_T<U>& rhs)
		{
			m_ -= rhs.m_;
			return *this;
		}
		template <typename U>
		Matrix4_T& operator*=(const Matrix4_T<U>& rhs)
		{
			return MathLib::Multiply(*this, *this, rhs);
		}
		template <typename U>
		Matrix4_T& operator*=(const U& rhs)
		{
			m_ *= rhs;
			return *this;
		}
		template <typename U>
		Matrix4_T& operator/=(const U& rhs)
			{ return this->operator*=(1.0f / rhs); }

		Matrix4_T& operator=(const Matrix4_T& rhs)
		{
			if (this != &rhs)
			{
				m_ = rhs.m_;
			}
			return *this;
		}
		template <typename U>
		Matrix4_T& operator=(const Matrix4_T<U>& rhs)
		{
			if (this != &rhs)
			{
				m_ = rhs.m_;
			}
			return *this;
		}

		// 一元操作符
		const Matrix4_T operator+() const
			{ return *this; }
		const Matrix4_T operator-() const
		{
			Matrix4_T temp(*this);
			temp.m_ = -m_;
			return temp;
		}

	private:
		Vector_T<Vector_T<T, col_num>, row_num> m_;
	};

	template <typename T>
	inline const Matrix4_T<T>
	operator*(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs)
	{
		return Matrix4_T<T>(lhs) *= rhs;
	}

	template <typename T>
	inline bool
	operator==(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs)
	{
		return std::equal(rhs.begin(), rhs.end(), lhs.begin());
	}
	template <typename T>
	inline bool
	operator!=(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Matrix4_T<float> Matrix4;
}

#endif			// _MATRIX_HPP
