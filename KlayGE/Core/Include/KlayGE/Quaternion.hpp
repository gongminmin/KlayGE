// Quaternion.hpp
// KlayGE 四元数 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _QUATERNION_HPP
#define _QUATERNION_HPP

#include <boost/operators.hpp>

#include <KlayGE/Vector.hpp>

namespace KlayGE
{
	template <typename T>
	class Quaternion_T : boost::addable<Quaternion_T<T>,
						boost::subtractable<Quaternion_T<T>,
						boost::dividable2<Quaternion_T<T>, T,
						boost::multipliable<Quaternion_T<T>,
						boost::multipliable2<Quaternion_T<T>, T> > > > >
	{
		template <typename U>
		friend class Quaternion_T;

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
		Quaternion_T()
			{ }
		explicit Quaternion_T(const T* rhs)
			: quat_(rhs)
			{ }
		Quaternion_T(const Vector_T<T, 3>& vec, const T& s)
		{
			this->x() = vec.x();
			this->y() = vec.y();
			this->z() = vec.z();
			this->w() = s;
		}
		Quaternion_T(const Quaternion_T& rhs)
			: quat_(rhs.quat_)
			{ }
		template <typename U>
		Quaternion_T(const Quaternion_T<U>& rhs)
			: quat_(reinterpret_cast<const Quaternion_T<T>(rhs).quat_)
			{ }
		Quaternion_T(const T& _x, const T& _y, const T& _z, const T& _w)
		{
			this->x() = _x;
			this->y() = _y;
			this->z() = _z;
			this->w() = _w;
		}

		static const Quaternion_T& Identity()
		{
			static Quaternion_T out(0, 0, 0, 1);
			return out;
		}

		// 取向量
		iterator begin()
			{ return quat_.begin(); }
		const_iterator begin() const
			{ return quat_.begin(); }
		iterator end()
			{ return quat_.end(); }
		const_iterator end() const
			{ return quat_.end(); }
		reference operator[](size_t index)
			{ return quat_[index]; }
		const_reference operator[](size_t index) const
			{ return quat_[index]; }

		reference x()
			{ return quat_[0]; }
		const_reference x() const
			{ return quat_[0]; }
		reference y()
			{ return quat_[1]; }
		const_reference y() const
			{ return quat_[1]; }
		reference z()
			{ return quat_[2]; }
		const_reference z() const
			{ return quat_[2]; }
		reference w()
			{ return quat_[3]; }
		const_reference w() const
			{ return quat_[3]; }

		// 赋值操作符
		template <typename U>
		Quaternion_T& operator+=(const Quaternion_T<U>& rhs)
		{
			quat_ += rhs.quat_;
			return *this;
		}
		template <typename U>
		Quaternion_T& operator-=(const Quaternion_T<U>& rhs)
		{
			quat_ -= rhs.quat_;
			return *this;
		}

		template <typename U>
		Quaternion_T& operator*=(const Quaternion_T<U>& rhs)
		{
			return MathLib::Multiply(*this, *this, rhs);
		}
		template <typename U>
		Quaternion_T& operator*=(const U& rhs)
		{
			quat_ *= rhs;
			return *this;
		}
		template <typename U>
		Quaternion_T& operator/=(const U& rhs)
		{
			quat_ /= rhs;
			return *this;
		}

		Quaternion_T& operator=(const Quaternion_T& rhs)
		{
			if (this != &rhs)
			{
				quat_ = rhs.quat_;
			}
			return *this;
		}
		template <typename U>
		Quaternion_T& operator=(const Quaternion_T<U>& rhs)
		{
			if (this != &rhs)
			{
				quat_ = rhs.quat_;
			}
			return *this;
		}

		// 一元操作符
		const Quaternion_T operator+() const
			{ return *this; }
		const Quaternion_T operator-() const
			{ return Quaternion_T(-this->x(), -this->y(), -this->z(), -this->w()); }

		// 取方向向量
		const Vector_T<T, 3> v() const
			{ return MakeVector(this->x(), this->y(), this->z()); }
		void v(const Vector_T<T, 3>& rhs)
		{
			this->x() = rhs.x();
			this->y() = rhs.y();
			this->z() = rhs.z();
		}

	private:
		Vector_T<T, elem_num> quat_;
	};

	template <typename T>
	inline bool
	operator==(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs)
	{
		return (lhs.x() == rhs.x()) && (lhs.y() == rhs.y()) && (lhs.z() == rhs.z()) && (lhs.w() == rhs.w());
	}
	template <typename T>
	inline bool
	operator!=(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs)
	{
		return !(lhs == rhs);
	}

	typedef Quaternion_T<float> Quaternion;
}

#endif			// _QUATERNION_HPP
