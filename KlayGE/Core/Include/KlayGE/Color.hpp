// Color.hpp
// KlayGE 颜色 头文件
// Ver 2.1.1
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.1
// 初次建立 (2004.4.22)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _COLOR_HPP
#define _COLOR_HPP

#include <boost/operators.hpp>

#include <KlayGE/Vector.hpp>

namespace KlayGE
{
	// RGBA，用4个浮点数表示r, g, b, a
	///////////////////////////////////////////////////////////////////////////////
	class Color : boost::addable<Color,
						boost::subtractable<Color,
						boost::dividable2<Color, float,
						boost::multipliable2<Color, float> > > >
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
			: col_(rhs)
			{ }
		Color(const Color& rhs)
			: col_(rhs.col_)
			{ }
		Color(float r, float g, float b, float a)
		{
			this->r() = r;
			this->g() = g;
			this->b() = b;
			this->a() = a;
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
		iterator begin()
			{ return col_.begin(); }
		const_iterator begin() const
			{ return col_.begin(); }
		iterator end()
			{ return col_.end(); }
		const_iterator end() const
			{ return col_.end(); }
		reference operator[](size_t index)
			{ return col_[index]; }
		const_reference operator[](size_t index) const
			{ return col_[index]; }

		reference r()
			{ return col_[0]; }
		const_reference r() const
			{ return col_[0]; }
		reference g()
			{ return col_[1]; }
		const_reference g() const
			{ return col_[1]; }
		reference b()
			{ return col_[2]; }
		const_reference b() const
			{ return col_[2]; }
		reference a()
			{ return col_[3]; }
		const_reference a() const
			{ return col_[3]; }

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
			col_ += rhs.col_;
			return *this;
		}
		Color& operator-=(const Color& rhs)
		{
			col_ -= rhs.col_;
			return *this;
		}
		Color& operator*=(float rhs)
		{
			col_ *= rhs;
			return *this;
		}
		Color& operator/=(float rhs)
		{
			col_ /= rhs;
			return *this;
		}

		Color& operator=(const Color& rhs)
		{
			if (this != &rhs)
			{
				col_ = rhs.col_;
			}
			return *this;
		}

		// 一元操作符
		const Color operator+() const
			{ return *this; }
		const Color operator-() const
			{ return Color(-this->r(), -this->g(), -this->b(), -this->a()); }

	private:
		Vector_T<float, elem_num> col_;
	};

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
}

#endif			// _COLOR_HPP
