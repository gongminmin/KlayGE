// MathTypes.hpp
// KlayGE 数学函数库 自定义类型 头文件
// Ver 1.4.8.5
// 版权所有(C) 龚敏敏, 2001--2003
// Homepage: http://www.enginedev.com
//
// 1.2.8.8
// 把inline放入类声明 (2002.9.25)
//
// 1.2.8.11
// 更改了类型名 (2002.11.7)
//
// 1.4.8.2
// 改用泛型 (2003.2.13)
//
// 1.4.8.5
// 去掉了隐式类型转换 (2003.4.30)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _MATHTYPES_HPP
#define _MATHTYPES_HPP

#define NOMINMAX

#include <algorithm>
#include <functional>

#include <KlayGE/MetaManip.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 向量
	///////////////////////////////////////////////////////////////////////////////
	template <int N, typename T>
	class Vector_T
	{
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
			{ std::copy(rhs, rhs + elem_num, this->Begin()); }
		Vector_T(const Vector_T& rhs)
			{ std::copy(rhs.Begin(), rhs.End(), this->Begin()); }
		template <typename U>
		Vector_T(const Vector_T<N, U>& rhs)
			{ std::copy(rhs.Begin(), rhs.End(), this->Begin()); }

		static size_t Size()
			{ return elem_num; }

		static const Vector_T& Zero()
		{
			static Vector_T<N, T> zero;
			std::fill(zero.Begin(), zero.End(), T(0));
			return zero;
		}

		// 取向量
		iterator Begin()
			{ return &vec_[0]; }
		const_iterator Begin() const
			{ return &vec_[0]; }
		iterator End()
			{ return this->Begin() + elem_num; }
		const_iterator End() const
			{ return this->Begin() + elem_num; }
		reference operator[](size_t index)
			{ return *(this->Begin() + index); }
		const_reference operator[](size_t index) const
			{ return *(this->Begin() + index); }
		reference x()
		{
			StaticAssert<elem_num >= 1>();
			return this->operator[](0);
		}

		const_reference x() const
		{
			StaticAssert<elem_num >= 1>();
			return this->operator[](0);
		}

		reference y()
		{
			StaticAssert<elem_num >= 2>();
			return this->operator[](1);
		}

		const_reference y() const
		{
			StaticAssert<elem_num >= 2>();
			return this->operator[](1);
		}

		reference z()
		{
			StaticAssert<elem_num >= 3>();
			return this->operator[](2);
		}

		const_reference z() const
		{
			StaticAssert<elem_num >= 3>();
			return this->operator[](2);
		}

		reference w()
		{
			StaticAssert<elem_num >= 4>();
			return this->operator[](3);
		}

		const_reference w() const
		{
			StaticAssert<elem_num >= 4>();
			return this->operator[](3);
		}

		// 赋值操作符
		template <typename U>
		Vector_T& operator+=(const Vector_T<N, U>& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::plus<T>());
			return *this;
		}
		template <typename U>
		Vector_T& operator-=(const Vector_T<N, U>& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::minus<T>());
			return *this;
		}
		template <typename U>
		Vector_T& operator*=(const U& rhs)
		{
			std::transform(this->Begin(), this->End(), this->Begin(), std::bind2nd(std::multiplies<T>(), rhs));
			return *this;
		}
		template <typename U>
		Vector_T& operator/=(const U& rhs)
		{
			return operator*=(1.0f / rhs);
		}

		Vector_T& operator=(const Vector_T& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}
		template <typename U>
		Vector_T& operator=(const Vector_T<N, U>& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}

		// 一元操作符
		const Vector_T operator+() const
			{ return *this; }
		const Vector_T operator-() const
		{
			Vector_T temp;
			std::transform(this->Begin(), this->End(), temp.Begin(), std::negate<T>());
			return temp;
		}

	private:
		T vec_[elem_num];
	};

	template <int N, typename T>
	inline const Vector_T<N, T>
	operator+(const Vector_T<N, T>& lhs, const Vector_T<N, T>& rhs)
	{
		return Vector_T<N, T>(lhs) += rhs;
	}
	template <int N, typename T>
	inline const Vector_T<N, T>
	operator-(const Vector_T<N, T>& lhs, const Vector_T<N, T>& rhs)
	{
		return Vector_T<N, T>(lhs) -= rhs;
	}
	template <int N, typename T>
	inline const Vector_T<N, T>
	operator*(const Vector_T<N, T>& lhs, const T& rhs)
	{
		return Vector_T<N, T>(lhs) *= rhs;
	}
	template <int N, typename T>
	inline const Vector_T<N, T>
	operator*(const T& lhs, const Vector_T<N, T>& rhs)
	{
		return rhs * lhs;
	}
	template <int N, typename T>
	inline const Vector_T<N, T>
	operator/(const Vector_T<N, T>& lhs, const T& rhs)
	{
		return lhs * (1.0f / rhs);
	}

	template <int N, typename T>
	inline bool
	operator==(const Vector_T<N, T>& lhs, const Vector_T<N, T>& rhs)
	{
		for (int i = 0; i < N; ++ i)
		{
			if (lhs[i] != rhs[i])
			{
				return false;
			}
		}
		return true;
	}
	template <int N, typename T>
	inline bool
	operator!=(const Vector_T<N, T>& lhs, const Vector_T<N, T>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Vector_T<2, float> Vector2;
	typedef Vector_T<3, float> Vector3;
	typedef Vector_T<4, float> Vector4;

	template <typename T>
	Vector_T<2, T> MakeVector(const T& x, const T& y)
	{
		T data[2] = { x, y };
		return Vector_T<2, T>(data);
	}

	template <typename T>
	Vector_T<3, T> MakeVector(const T& x, const T& y, const T& z)
	{
		T data[3] = { x, y, z };
		return Vector_T<3, T>(data);
	}

	template <typename T>
	Vector_T<4, T> MakeVector(const T& x, const T& y, const T& z, const T& w)
	{
		T data[4] = { x, y, z, w };
		return Vector_T<4, T>(data);
	}


	// 矩形大小
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Size_T
	{
	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { elem_num = 2 };

	public:
		Size_T()
			{ }
		explicit Size_T(const T* rhs)
		{
			this->cx() = rhs[0];
			this->cy() = rhs[1];
		}
		Size_T(const Size_T& rhs)
		{
			this->cx() = rhs.cx();
			this->cy() = rhs.cy();
		}
		template <typename U>
		Size_T(const Size_T<U>& rhs)
		{
			this->cx() = rhs.cx();
			this->cy() = rhs.cy();
		}
		Size_T(const T& _cx, const T& _cy)
		{
			this->cx() = _cx;
			this->cy() = _cy;
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
		Size_T& operator+=(const Size_T<U>& rhs)
		{
			this->cx() += rhs.cx();
			this->cy() += rhs.cy();
			return *this;
		}
		template <typename U>
		Size_T& operator-=(const Size_T<U>& rhs)
		{
			this->cx() -= rhs.cx();
			this->cy() -= rhs.cy();
			return *this;
		}

		Size_T& operator=(const Size_T& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.size_, rhs.size_ + elem_num, size_);
			}
			return *this;
		}
		template <typename U>
		Size_T& operator=(const Size_T<U>& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.size_, rhs.size_ + elem_num, size_);
			}
			return *this;
		}

		// 一元操作符
		const Size_T<T> operator+() const
			{ return *this; }
		const Size_T<T> operator-() const
			{ return Size_T<T>(-this->cx(), -this->cy()); }

	private:
		T size_[elem_num];
	};

	template <typename T>
	inline const Size_T<T>
	operator+(const Size_T<T>& lhs, const Size_T<T>& rhs)
	{
		return Size_T<T>(lhs.cx() + rhs.cx(), lhs.cy() + rhs.cy());
	}
	template <typename T>
	inline const Size_T<T>
	operator-(const Size_T<T>& lhs, const Size_T<T>& rhs)
	{
		return Size_T<T>(lhs.cx() - rhs.cx(), lhs.cy() - rhs.cy());
	}

	template <typename T>
	inline bool
	operator==(const Size_T<T>& lhs, const Size_T<T>& rhs)
	{
		return (lhs.cx() == rhs.cx()) && (lhs.cy() == rhs.cy());
	}
	template <typename T>
	inline bool
	operator!=(const Size_T<T>& lhs, const Size_T<T>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Size_T<float> Size;


	// 矩形
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Rect_T
	{
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
		Rect_T()
			{ }
		explicit Rect_T(const T* rhs)
		{
			this->left()	= rhs[0];
			this->top()		= rhs[1];
			this->right()	= rhs[2];
			this->bottom()	= rhs[3];
		}
		Rect_T(const Rect_T& rhs)
		{
			this->left()	= rhs.left();
			this->top()		= rhs.top();
			this->right()	= rhs.right();
			this->bottom()	= rhs.bottom();
		}
		template <typename U>
		Rect_T(const Rect_T<U>& rhs)
		{
			this->left()	= rhs.left();
			this->top()		= rhs.top();
			this->right()	= rhs.right();
			this->bottom()	= rhs.bottom();
		}
		Rect_T(const T& _left, const T& _top, const T& _right, const T& _bottom)
		{
			this->left()	= _left;
			this->top()		= _top;
			this->right()	= _right;
			this->bottom()	= _bottom;
		}

		// 取向量
		reference left()
			{ return rect_[0]; }
		const_reference left() const
			{ return rect_[0]; }
		reference top()
			{ return rect_[1]; }
		const_reference top() const
			{ return rect_[1]; }
		reference right()
			{ return rect_[2]; }
		const_reference right() const
			{ return rect_[2]; }
		reference bottom()
			{ return rect_[3]; }
		const_reference bottom() const
			{ return rect_[3]; }

		// 赋值操作符
		template <typename U>
		Rect_T& operator+=(const Vector_T<2, U>& rhs)
		{
			this->left()	+= rhs.x();
			this->top()		+= rhs.y();
			this->right()	+= rhs.x();
			this->bottom()	+= rhs.y();
			return *this;
		}
		template <typename U>
		Rect_T& operator-=(const Vector_T<2, U>& rhs)
		{
			this->left()	-= rhs.x();
			this->top()		-= rhs.y();
			this->right()	-= rhs.x();
			this->bottom()	-= rhs.y();
			return *this;
		}
		template <typename U>
		Rect_T& operator+=(const Rect_T<U>& rhs)
		{
			this->left()	+= rhs.left();
			this->top()		+= rhs.top();
			this->right()	+= rhs.right();
			this->bottom()	+= rhs.bottom();
			return *this;
		}
		template <typename U>
		Rect_T& operator-=(const Rect_T<U>& rhs)
		{
			this->left()	-= rhs.left();
			this->top()		-= rhs.top();
			this->right()	-= rhs.right();
			this->bottom()	-= rhs.bottom();
			return *this;
		}
		template <typename U>
		Rect_T& operator&=(const Rect_T<U>& rhs)
		{
			this->left()	= std::max(this->left(),	rhs.left());
			this->top()		= std::max(this->top(),		rhs.top());
			this->right()	= std::min(this->right(),	rhs.right());
			this->bottom()	= std::min(this->bottom(),	rhs.bottom());
			return *this;
		}
		template <typename U>
		Rect_T& operator|=(const Rect_T<U>& rhs)
		{
			this->left()	= std::min(this->left(),	rhs.left());
			this->top()		= std::min(this->top(),		rhs.top());
			this->right()	= std::max(this->right(),	rhs.right());
			this->bottom()	= std::max(this->bottom(),	rhs.bottom());
			return *this;
		}

		Rect_T& operator=(const Rect_T& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.rect_, rhs.rect_ + elem_num, rect_);
			}
			return *this;
		}
		template <typename U>
		Rect_T& operator=(const Rect_T<U>& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.rect_, rhs.rect_ + elem_num, rect_);
			}
			return *this;
		}

		// 一元操作符
		const Rect_T operator+() const
			{ return *this; }
		const Rect_T operator-() const
			{ return Rect_T<T>(-this->left(), -this->top(), -this->right(), -this->bottom()); }

		// 属性
		T Width() const
			{ return this->right() - this->left(); }
		T Height() const
			{ return this->bottom() - this->top(); }
		const Size_T<T> Size() const
			{ return Size_T<T>(this->Width(), this->Height()); }
		bool IsEmpty() const
			{ return (this->left() == this->right()) && (this->top() == this->bottom()); }

	private:
		T rect_[elem_num];
	};

	template <typename T>
	inline const Rect_T<T>
	operator+(const Rect_T<T>& lhs, const Vector_T<2, T>& rhs)
	{
		return Rect_T<T>(lhs.left() + rhs.x(), lhs.top() + rhs.y(),
			lhs.right() + rhs.x(), lhs.bottom() + rhs.y());
	}
	template <typename T>
	inline const Rect_T<T>
	operator-(const Rect_T<T>& lhs, const Vector_T<2, T>& rhs)
	{
		return Rect_T<T>(lhs.left() - rhs.x(), lhs.top() - rhs.y(),
			lhs.right() - rhs.x(), lhs.bottom() - rhs.y());
	}
	template <typename T>
	inline const Rect_T<T>
	operator+(const Rect_T<T>& lhs, const Rect_T<T>& rhs)
	{
		return Rect_T<T>(lhs.left() + rhs.left(), lhs.top() + rhs.top(),
			lhs.right() + rhs.right(), lhs.bottom() + rhs.bottom());
	}
	template <typename T>
	inline const Rect_T<T>
	operator-(const Rect_T<T>& lhs, const Rect_T<T>& rhs)
	{
		return Rect_T<T>(lhs.left() - rhs.left(), lhs.top() - rhs.top(),
			lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom());
	}

	template <typename T>
	inline const Rect_T<T>
	operator&(const Rect_T<T>& lhs, const Rect_T<T>& rhs)
	{
		return Rect_T<T>(std::max(lhs.left(), rhs.left()),
			std::max(lhs.top(), rhs.top()),
			std::min(lhs.right(), rhs.right()),
			std::min(lhs.bottom(), rhs.bottom()));
	}
	template <typename T>
	inline const Rect_T<T>
	operator|(const Rect_T<T>& lhs, const Rect_T<T>& rhs)
	{
		return Rect_T<T>(std::min(lhs.left(), rhs.left()),
			std::min(lhs.top(), rhs.top()),
			std::max(lhs.right(), rhs.right()),
			std::max(lhs.bottom(), rhs.bottom()));
	}

	template <typename T>
	inline bool
	operator==(const Rect_T<T>& lhs, const Rect_T<T>& rhs)
	{
		return (lhs.left() == rhs.left()) && (lhs.top() == rhs.top())
			&& (lhs.right() == rhs.right()) && (lhs.bottom() == rhs.bottom());
	}
	template <typename T>
	inline bool
	operator!=(const Rect_T<T>& lhs, const Rect_T<T>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Rect_T<float> Rect;


	// 4D 矩阵
	///////////////////////////////////////////////////////////////////////////////
	class Matrix4
	{
	public:
		typedef float				value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { elem_num = 4 * 4 };

	public:
		Matrix4()
			{ }
		explicit Matrix4(const float* rhs)
			{ std::copy(rhs, rhs + elem_num, this->Begin()); }
		Matrix4(const Matrix4& rhs)
			{ std::copy(rhs.Begin(), rhs.End(), this->Begin()); }
		Matrix4(float f11, float f12, float f13, float f14,
			float f21, float f22, float f23, float f24,
			float f31, float f32, float f33, float f34,
			float f41, float f42, float f43, float f44)
		{
			m_[0][0] = f11;	m_[0][1] = f12;	m_[0][2] = f13;	m_[0][3] = f14;
			m_[1][0] = f21;	m_[1][1] = f22;	m_[1][2] = f23;	m_[1][3] = f24;
			m_[2][0] = f31;	m_[2][1] = f32;	m_[2][2] = f33;	m_[2][3] = f34;
			m_[3][0] = f41;	m_[3][1] = f42;	m_[3][2] = f43;	m_[3][3] = f44;
		}

		static const Matrix4& Identity();

		reference operator()(size_t row, size_t col)
			{ return m_[row][col]; }
		const_reference operator()(size_t row, size_t col) const
			{ return m_[row][col]; }
		iterator Begin()
			{ return &m_[0][0]; }
		const_iterator Begin() const
			{ return &m_[0][0]; }
		iterator End()
			{ return this->Begin() + elem_num; }
		const_iterator End() const
			{ return this->Begin() + elem_num; }

		// 赋值操作符
		Matrix4& operator*=(const Matrix4& rhs);
		Matrix4& operator+=(const Matrix4& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::plus<float>());
			return *this;
		}
		Matrix4& operator-=(const Matrix4& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::minus<float>());
			return *this;
		}
		Matrix4& operator*=(float rhs)
		{
			std::transform(this->Begin(), this->End(), this->Begin(), std::bind2nd(std::multiplies<float>(), rhs));
			return *this;
		}
		Matrix4& operator/=(float rhs)
			{ return this->operator*=(1.0f / rhs); }

		Matrix4& operator=(const Matrix4& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}

		// 一元操作符
		const Matrix4 operator+() const
			{ return *this; }
		const Matrix4 operator-() const
		{
			Matrix4 temp(*this);
			std::transform(temp.Begin(), temp.End(), temp.Begin(), std::negate<float>());
			return temp;
		}

	private:
		float m_[4][4];
	};

	inline const Matrix4
	operator*(const Matrix4& lhs, const Matrix4& rhs)
	{
		return Matrix4(lhs) *= rhs;
	}
	inline const Matrix4
	operator+(const Matrix4& lhs, const Matrix4& rhs)
	{
		return Matrix4(lhs) += rhs;
	}
	inline const Matrix4
	operator-(const Matrix4& lhs, const Matrix4& rhs)
	{
		return Matrix4(lhs) -= rhs;
	}
	inline const Matrix4
	operator*(const Matrix4& lhs, float rhs)
	{
		return Matrix4(lhs) *= rhs;
	}
	inline const Matrix4
	operator*(float lhs, const Matrix4& rhs)
	{
		return rhs * lhs;
	}
	inline const Matrix4
	operator/(const Matrix4& lhs, float rhs)
	{
		return lhs * (1.0f / rhs);
	}

	bool operator==(const Matrix4& lhs, const Matrix4& rhs);
	inline bool
	operator!=(const Matrix4& lhs, const Matrix4& rhs)
	{
		return !(lhs == rhs);
	}


	// 四元数
	///////////////////////////////////////////////////////////////////////////////
	class Quaternion
	{
	public:
		typedef float				value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { elem_num = 4 };

	public:
		Quaternion()
			{ }
		explicit Quaternion(const float* rhs)
			{ std::copy(rhs, rhs + elem_num, this->Begin()); }
		Quaternion(const Vector3& vec, float s)
		{
			this->x() = vec.x();
			this->y() = vec.y();
			this->z() = vec.z();
			this->w() = s;
		}
		Quaternion(const Quaternion& rhs)
			{ std::copy(rhs.Begin(), rhs.End(), this->Begin()); }
		Quaternion(float _x, float _y, float _z, float _w)
		{
			this->x() = _x;
			this->y() = _y;
			this->z() = _z;
			this->w() = _w;
		}

		static const Quaternion& Identity();

		// 取向量
		iterator Begin()
			{ return &quat_[0]; }
		const_iterator Begin() const
			{ return &quat_[0]; }
		iterator End()
			{ return this->Begin() + elem_num; }
		const_iterator End() const
			{ return this->Begin() + elem_num; }
		reference operator[](size_t index)
			{ return *(this->Begin() + index); }
		const_reference operator[](size_t index) const
			{ return *(this->Begin() + index); }
		reference x()
			{ return this->operator[](0); }
		const_reference x() const
			{ return this->operator[](0); }
		reference y()
			{ return this->operator[](1); }
		const_reference y() const
			{ return this->operator[](1); }
		reference z()
			{ return this->operator[](2); }
		const_reference z() const
			{ return this->operator[](2); }
		reference w()
			{ return this->operator[](3); }
		const_reference w() const
			{ return this->operator[](3); }

		// 赋值操作符
		Quaternion& operator+=(const Quaternion& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::plus<float>());
			return *this;
		}
		Quaternion& operator-=(const Quaternion& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::minus<float>());
			return *this;
		}

		Quaternion& operator*=(const Quaternion& rhs);
		Quaternion& operator*=(float rhs)
		{
			std::transform(this->Begin(), this->End(), this->Begin(), std::bind2nd(std::multiplies<float>(), rhs));
			return *this;
		}
		Quaternion& operator/=(float rhs)
		{
			return operator*=(1.0f / rhs);
		}

		Quaternion& operator=(const Quaternion& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}

		// 一元操作符
		const Quaternion operator+() const
			{ return *this; }
		const Quaternion operator-() const
			{ return Quaternion(-this->x(), -this->y(), -this->z(), -this->w()); }

		// 取方向向量
		const Vector3 v() const
			{ return MakeVector(this->x(), this->y(), this->z()); }
		void v(const Vector3& rhs)
		{
			this->x() = rhs.x();
			this->y() = rhs.y();
			this->z() = rhs.z();
		}

	private:
		float quat_[elem_num];
	};

	inline const Quaternion
	operator+(const Quaternion& lhs, const Quaternion& rhs)
	{
		return Quaternion(lhs.x() + rhs.x(), lhs.y() + rhs.y(), lhs.z() + rhs.z(), lhs.w() + rhs.w());
	}
	inline const Quaternion
	operator-(const Quaternion& lhs, const Quaternion& rhs)
	{
		return Quaternion(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z(), lhs.w() - rhs.w());
	}
	inline const Quaternion
	operator*(const Quaternion& lhs, const Quaternion& rhs)
	{
		return Quaternion(lhs) *= rhs;
	}
	inline const Quaternion
	operator*(const Quaternion& lhs, float rhs)
	{
		return Quaternion(lhs.x() * rhs, lhs.y() * rhs, lhs.z() * rhs, lhs.w() * rhs);
	}
	inline const Quaternion
	operator*(float lhs, const Quaternion& rhs)
	{
		return rhs * lhs;
	}
	inline const Quaternion
	operator/(const Quaternion& lhs, float rhs)
	{
		return lhs * (1.0f / rhs);
	}

	inline bool
	operator==(const Quaternion& lhs, const Quaternion& rhs)
	{
		return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y()) && (lhs.z() == rhs.z()) && (lhs.w() == rhs.w());
	}
	inline bool
	operator!=(const Quaternion& lhs, const Quaternion& rhs)
	{
		return !(lhs == rhs);
	}


	// 描述一个平面 ax + by + cz + d = 0
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Plane_T
	{
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
		{
			this->a() = rhs[0];
			this->b() = rhs[1];
			this->c() = rhs[2];
			this->d() = rhs[3];
		}
		Plane_T(const Plane_T& rhs)
		{
			this->a() = rhs.a();
			this->b() = rhs.b();
			this->c() = rhs.c();
			this->d() = rhs.d();
		}
		template <typename U>
		Plane_T(const Plane_T<U>& rhs)
		{
			this->a() = rhs.a();
			this->b() = rhs.b();
			this->c() = rhs.c();
			this->d() = rhs.d();
		}
		Plane_T(const T& _a, const T& _b, const T& _c, const T& _d)
		{
			this->a() = _a;
			this->b() = _b;
			this->c() = _c;
			this->d() = _d;
		}

		// 取向量
		iterator Begin()
			{ return &plane_[0]; }
		const_iterator Begin() const
			{ return &plane_[0]; }
		iterator End()
			{ return this->Begin() + elem_num; }
		const_iterator End() const
			{ return this->Begin() + elem_num; }
		reference operator[](size_t index)
			{ return *(this->Begin() + index); }
		const_reference operator[](size_t index) const
			{ return *(this->Begin() + index); }
		reference a()
			{ return this->operator[](0); }
		const_reference a() const
			{ return this->operator[](0); }
		reference b()
			{ return this->operator[](1); }
		const_reference b() const
			{ return this->operator[](1); }
		reference c()
			{ return this->operator[](2); }
		const_reference c() const
			{ return this->operator[](2); }
		reference d()
			{ return this->operator[](3); }
		const_reference d() const
			{ return this->operator[](3); }

		// 赋值操作符
		Plane_T& operator=(const Plane_T& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}
		template <typename U>
		Plane_T& operator=(const Plane_T<U>& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}

		// 一元操作符
		const Plane_T operator+() const
			{ return *this; }
		const Plane_T operator-() const
			{ return Plane_T<T>(-this->a(), -this->b(), -this->c(), -this->d()); }

		// 取法向向量
		const Vector_T<3, T> Normal() const
			{ return MakeVector<T>(this->a(), this->b(), this->c()); }
		template <typename U>
		void Normal(const Vector_T<3, U>& rhs)
		{
			this->a() = rhs.x();
			this->b() = rhs.y();
			this->c() = rhs.z();
		}

	private:
		T plane_[elem_num];
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


	// 颜色RGBA，用4个浮点数表示r, g, b, a
	///////////////////////////////////////////////////////////////////////////////
	class Color
	{
	public:
		typedef float				value_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;

		enum { elem_num = 4 };

	public:
		Color()
			{ }
		explicit Color(const float* rhs)
		{
			this->r() = rhs[0];
			this->g() = rhs[1];
			this->b() = rhs[2];
			this->a() = rhs[3];
		}
		Color(const Color& rhs)
		{
			this->r() = rhs.r();
			this->g() = rhs.g();
			this->b() = rhs.b();
			this->a() = rhs.a();
		}
		Color(float _r, float _g, float _b, float _a)
		{
			this->r() = _r;
			this->g() = _g;
			this->b() = _b;
			this->a() = _a;
		}
		explicit Color(U32 dw)
		{
			const float f(1 / 255.0f);
			this->a() = f * (static_cast<float>(static_cast<U8>(dw >> 24)));
			this->r() = f * (static_cast<float>(static_cast<U8>(dw >> 16)));
			this->g() = f * (static_cast<float>(static_cast<U8>(dw >>  8)));
			this->b() = f * (static_cast<float>(static_cast<U8>(dw)));
		}

		// 取颜色
		iterator Begin()
			{ return &col_[0]; }
		const_iterator Begin() const
			{ return &col_[0]; }
		iterator End()
			{ return this->Begin() + elem_num; }
		const_iterator End() const
			{ return this->Begin() + elem_num; }
		reference operator[](size_t index)
			{ return *(this->Begin() + index); }
		const_reference operator[](size_t index) const
			{ return *(this->Begin() + index); }
		reference r()
			{ return this->operator[](0); }
		const_reference r() const
			{ return this->operator[](0); }
		reference g()
			{ return this->operator[](1); }
		const_reference g() const
			{ return this->operator[](1); }
		reference b()
			{ return this->operator[](2); }
		const_reference b() const
			{ return this->operator[](2); }
		reference a()
			{ return this->operator[](3); }
		const_reference a() const
			{ return this->operator[](3); }

		void RGBA(U8& R, U8& G, U8& B, U8& A) const
		{
			R = static_cast<U8>((this->r() >= 1) ? 255 : (this->r() <= 0 ? 0 : static_cast<U32>(this->r() * 255.0f + 0.5f)));
			G = static_cast<U8>((this->g() >= 1) ? 255 : (this->g() <= 0 ? 0 : static_cast<U32>(this->g() * 255.0f + 0.5f)));
			B = static_cast<U8>((this->b() >= 1) ? 255 : (this->b() <= 0 ? 0 : static_cast<U32>(this->b() * 255.0f + 0.5f)));
			A = static_cast<U8>((this->a() >= 1) ? 255 : (this->a() <= 0 ? 0 : static_cast<U32>(this->a() * 255.0f + 0.5f)));
		}

		// 赋值操作符
		Color& operator+=(const Color& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::plus<float>());
			return *this;
		}
		Color& operator-=(const Color& rhs)
		{
			std::transform(this->Begin(), this->End(), rhs.Begin(), this->Begin(), std::minus<float>());
			return *this;
		}
		Color& operator*=(float rhs)
		{
			std::transform(this->Begin(), this->End(), this->Begin(), std::bind2nd(std::multiplies<float>(), rhs));
			return *this;
		}
		Color& operator/=(float rhs)
		{
			return operator*=(1.0f / rhs);
		}

		Color& operator=(const Color& rhs)
		{
			if (this != &rhs)
			{
				std::copy(rhs.Begin(), rhs.End(), this->Begin());
			}
			return *this;
		}

		// 一元操作符
		const Color operator+() const
			{ return *this; }
		const Color operator-() const
			{ return Color(-this->r(), -this->g(), -this->b(), -this->a()); }

	private:
		float col_[elem_num];
	};

	inline const Color
	operator+(const Color& lhs, const Color& rhs)
	{
		return Color(lhs.r() + rhs.r(), lhs.g() + rhs.g(), lhs.b() + rhs.b(), lhs.a() + rhs.a());
	}
	inline const Color
	operator-(const Color& lhs, const Color& rhs)
	{
		return Color(lhs.r() - rhs.r(), lhs.g() - rhs.g(), lhs.b() - rhs.b(), lhs.a() - rhs.a());
	}
	inline const Color
	operator*(const Color& lhs, float rhs)
	{
		return Color(lhs.r() * rhs, lhs.g() * rhs, lhs.b() * rhs, lhs.a() * rhs);
	}
	inline const Color
	operator*(float lhs, const Color& rhs)
	{
		return rhs * lhs;
	}
	inline const Color
	operator/(const Color& lhs, float rhs)
	{
		return lhs * (1.0f / rhs);
	}
 
	inline bool
	operator==(const Color& lhs, const Color& rhs)
	{
		return (lhs.r() == rhs.r()) && (lhs.g() == rhs.g()) && (lhs.b() == rhs.b()) && (lhs.a() == rhs.a());
	}
	inline bool
	operator!=(const Color& lhs, const Color& rhs)
	{
		return !(lhs == rhs);
	}

	
	// 边框
	///////////////////////////////////////////////////////////////////////////////
	class Box
	{
	public:
		Box()
			{ }
		Box(const Vector3& vMin, const Vector3& vMax);

		// 赋值操作符
		Box& operator+=(const Vector3& rhs)
		{
			min_ += rhs;
			max_ += rhs;
			return *this;
		}
		Box& operator-=(const Vector3& rhs)
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
			{ return operator*=(1.0f / rhs); }
		Box& operator&=(const Box& rhs);
		Box& operator|=(const Box& rhs);

		Box& operator=(const Box& rhs)
		{
			if (this != &rhs)
			{
				this->Min() = rhs.Min();
				this->Max() = rhs.Max();
			}
			return *this;
		}

		// 一元操作符
		const Box operator+() const
			{ return *this; }
		const Box operator-() const
			{ return Box(-this->Min(), -this->Max()); }

		// 属性
		float Width() const
			{ return this->Max().x() - this->Min().x(); }
		float Height() const
			{ return this->Max().y() - this->Min().y(); }
		float Depth() const
			{ return this->Max().z() - this->Min().z(); }
		bool IsEmpty() const
			{ return this->Min() == this->Max(); }

		const Vector3 LeftBottomNear() const
			{ return this->Min(); }
		const Vector3 LeftTopNear() const
			{ return MakeVector(this->Min().x(), this->Max().y(), this->Min().z()); }
		const Vector3 RightBottomNear() const
			{ return MakeVector(this->Max().x(), this->Min().y(), this->Min().z()); }
		const Vector3 RightTopNear() const
			{ return MakeVector(this->Max().x(), this->Max().y(), this->Min().z()); }
		const Vector3 LeftBottomFar() const
			{ return MakeVector(this->Min().x(), this->Min().y(), this->Max().z()); }
		const Vector3 LeftTopFar() const
			{ return MakeVector(this->Min().x(), this->Max().y(), this->Max().z()); }
		const Vector3 RightBottomFar() const
			{ return MakeVector(this->Max().x(), this->Min().y(), this->Max().z()); }
		const Vector3 RightTopFar() const
			{ return this->Max(); }

		Vector3& Min()
			{ return min_; }
		const Vector3& Min() const
			{ return min_; }
		Vector3& Max()
			{ return max_; }
		const Vector3& Max() const
			{ return max_; }

		bool VecInBound(const Vector3& v) const;
		bool IsHit(const Box& box) const;
		float MaxRadiusSq() const;

	private:
		Vector3 min_, max_;
	};

	inline const Box
	operator+(const Box& lhs, const Vector3& rhs)
	{
		return Box(lhs.Min() + rhs, lhs.Max() + rhs);
	}
	inline const Box
	operator-(const Box& lhs, const Vector3& rhs)
	{
		return Box(lhs.Min() - rhs, lhs.Max() - rhs);
	}

	inline const Box
	operator&(const Box& lhs, const Box& rhs)
	{
		return Box(lhs) &= rhs;
	}
	inline const Box
	operator|(const Box& lhs, const Box& rhs)
	{
		return Box(lhs) |= rhs;
	}

	inline bool
	operator==(const Box& lhs, const Box& rhs)
	{
		return (lhs.Min() == rhs.Min()) && (lhs.Max() == rhs.Max());
	}
	inline bool
	operator!=(const Box& lhs, const Box& rhs)
	{
		return !(lhs == rhs);
	}
}

#endif			// _MATHTYPES_HPP