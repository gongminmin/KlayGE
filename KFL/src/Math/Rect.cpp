/**
 * @file Rect.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KFL/KFL.hpp>

#include <KFL/Rect.hpp>

namespace KlayGE
{
	template Rect_T<float>::Rect_T(float const * rhs);
	template Rect_T<float>::Rect_T(Rect const & rhs);
	template Rect_T<float>::Rect_T(Rect&& rhs);
	template Rect_T<float>::Rect_T(IRect const & rhs);
	template Rect_T<float>::Rect_T(UIRect const & rhs);
	template Rect_T<float>::Rect_T(float left, float top, float right, float bottom);
	template Rect const & Rect_T<float>::operator+=(float2 const & rhs);
	template Rect const & Rect_T<float>::operator+=(int2 const & rhs);
	template Rect const & Rect_T<float>::operator+=(uint2 const & rhs);
	template Rect const & Rect_T<float>::operator-=(float2 const & rhs);
	template Rect const & Rect_T<float>::operator-=(int2 const & rhs);
	template Rect const & Rect_T<float>::operator-=(uint2 const & rhs);
	template Rect const & Rect_T<float>::operator+=(Rect const & rhs);
	template Rect const & Rect_T<float>::operator+=(IRect const & rhs);
	template Rect const & Rect_T<float>::operator+=(UIRect const & rhs);
	template Rect const & Rect_T<float>::operator-=(Rect const & rhs);
	template Rect const & Rect_T<float>::operator-=(IRect const & rhs);
	template Rect const & Rect_T<float>::operator-=(UIRect const & rhs);
	template Rect const & Rect_T<float>::operator&=(Rect const & rhs);
	template Rect const & Rect_T<float>::operator&=(IRect const & rhs);
	template Rect const & Rect_T<float>::operator&=(UIRect const & rhs);
	template Rect const & Rect_T<float>::operator|=(Rect const & rhs);
	template Rect const & Rect_T<float>::operator|=(IRect const & rhs);
	template Rect const & Rect_T<float>::operator|=(UIRect const & rhs);
	template Rect& Rect_T<float>::operator=(Rect const & rhs);
	template Rect& Rect_T<float>::operator=(Rect&& rhs);
	template Rect& Rect_T<float>::operator=(IRect const & rhs);
	template Rect& Rect_T<float>::operator=(UIRect const & rhs);
	template Rect const Rect_T<float>::operator+() const;
	template Rect const Rect_T<float>::operator-() const;
	template float Rect_T<float>::Width() const;
	template float Rect_T<float>::Height() const;
	template Size const Rect_T<float>::Size() const;
	template bool Rect_T<float>::IsEmpty() const;
	template bool Rect_T<float>::operator==(Rect const & rhs) const;
	template bool Rect_T<float>::PtInRect(float2 const & pt) const;

	template Rect_T<int32_t>::Rect_T(int32_t const * rhs);
	template Rect_T<int32_t>::Rect_T(IRect const & rhs);
	template Rect_T<int32_t>::Rect_T(IRect&& rhs);
	template Rect_T<int32_t>::Rect_T(Rect const & rhs);
	template Rect_T<int32_t>::Rect_T(UIRect const & rhs);
	template Rect_T<int32_t>::Rect_T(int32_t left, int32_t top, int32_t right, int32_t bottom);
	template IRect const & Rect_T<int32_t>::operator+=(float2 const & rhs);
	template IRect const & Rect_T<int32_t>::operator+=(int2 const & rhs);
	template IRect const & Rect_T<int32_t>::operator+=(uint2 const & rhs);
	template IRect const & Rect_T<int32_t>::operator-=(float2 const & rhs);
	template IRect const & Rect_T<int32_t>::operator-=(int2 const & rhs);
	template IRect const & Rect_T<int32_t>::operator-=(uint2 const & rhs);
	template IRect const & Rect_T<int32_t>::operator+=(Rect const & rhs);
	template IRect const & Rect_T<int32_t>::operator+=(IRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator+=(UIRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator-=(Rect const & rhs);
	template IRect const & Rect_T<int32_t>::operator-=(IRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator-=(UIRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator&=(Rect const & rhs);
	template IRect const & Rect_T<int32_t>::operator&=(IRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator&=(UIRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator|=(Rect const & rhs);
	template IRect const & Rect_T<int32_t>::operator|=(IRect const & rhs);
	template IRect const & Rect_T<int32_t>::operator|=(UIRect const & rhs);
	template IRect& Rect_T<int32_t>::operator=(IRect const & rhs);
	template IRect& Rect_T<int32_t>::operator=(IRect&& rhs);
	template IRect& Rect_T<int32_t>::operator=(Rect const & rhs);
	template IRect& Rect_T<int32_t>::operator=(UIRect const & rhs);
	template IRect const Rect_T<int32_t>::operator+() const;
	template IRect const Rect_T<int32_t>::operator-() const;
	template int32_t Rect_T<int32_t>::Width() const;
	template int32_t Rect_T<int32_t>::Height() const;
	template ISize const Rect_T<int32_t>::Size() const;
	template bool Rect_T<int32_t>::IsEmpty() const;
	template bool Rect_T<int32_t>::operator==(IRect const & rhs) const;
	template bool Rect_T<int32_t>::PtInRect(int2 const & pt) const;

	template Rect_T<uint32_t>::Rect_T(uint32_t const * rhs);
	template Rect_T<uint32_t>::Rect_T(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	template Rect_T<uint32_t>::Rect_T(UIRect const & rhs);
	template Rect_T<uint32_t>::Rect_T(UIRect&& rhs);
	template Rect_T<uint32_t>::Rect_T(Rect const & rhs);
	template Rect_T<uint32_t>::Rect_T(IRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator+=(float2 const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator+=(int2 const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator+=(uint2 const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator-=(float2 const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator-=(int2 const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator-=(uint2 const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator+=(Rect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator+=(IRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator+=(UIRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator-=(Rect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator-=(IRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator-=(UIRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator&=(Rect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator&=(IRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator&=(UIRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator|=(Rect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator|=(IRect const & rhs);
	template UIRect const & Rect_T<uint32_t>::operator|=(UIRect const & rhs);
	template UIRect& Rect_T<uint32_t>::operator=(UIRect const & rhs);
	template UIRect& Rect_T<uint32_t>::operator=(UIRect&& rhs);
	template UIRect& Rect_T<uint32_t>::operator=(IRect const & rhs);
	template UIRect& Rect_T<uint32_t>::operator=(UIRect const & rhs);
	template UIRect const Rect_T<uint32_t>::operator+() const;
	template uint32_t Rect_T<uint32_t>::Width() const;
	template uint32_t Rect_T<uint32_t>::Height() const;
	template UISize const Rect_T<uint32_t>::Size() const;
	template bool Rect_T<uint32_t>::IsEmpty() const;
	template bool Rect_T<uint32_t>::operator==(UIRect const & rhs) const;
	template bool Rect_T<uint32_t>::PtInRect(uint2 const & pt) const;


	template <typename T>
	Rect_T<T>::Rect_T(T const * rhs)
		: rect_(rhs)
	{
	}

	template <typename T>
	Rect_T<T>::Rect_T(Rect_T const & rhs)
		: rect_(rhs.rect_)
	{
	}

	template <typename T>
	Rect_T<T>::Rect_T(Rect_T&& rhs)
		: rect_(std::move(rhs.rect_))
	{
	}

	template <typename T>
	template <typename U>
	Rect_T<T>::Rect_T(Rect_T<U> const & rhs)
		: rect_(rhs.rect_)
	{
	}

	template <typename T>
	Rect_T<T>::Rect_T(T left, T top, T right, T bottom)
	{
		this->left()	= left;
		this->top()		= top;
		this->right()	= right;
		this->bottom()	= bottom;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator+=(Vector_T<U, 2> const & rhs)
	{
		this->left() += static_cast<T>(rhs.x());
		this->right() += static_cast<T>(rhs.x());
		this->top() += static_cast<T>(rhs.y());
		this->bottom() += static_cast<T>(rhs.y());
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator-=(Vector_T<U, 2> const & rhs)
	{
		this->left() -= static_cast<T>(rhs.x());
		this->right() -= static_cast<T>(rhs.x());
		this->top() -= static_cast<T>(rhs.y());
		this->bottom() -= static_cast<T>(rhs.y());
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator+=(Rect_T<U> const & rhs)
	{
		rect_ += Rect_T<T>(rhs).rect_;
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator-=(Rect_T<U> const & rhs)
	{
		rect_ -= Rect_T<T>(rhs).rect_;
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator&=(Rect_T<U> const & rhs)
	{
		this->left()	= std::max(this->left(),	static_cast<T>(rhs.left()));
		this->top()		= std::max(this->top(),		static_cast<T>(rhs.top()));
		this->right()	= std::min(this->right(),	static_cast<T>(rhs.right()));
		this->bottom()	= std::min(this->bottom(),	static_cast<T>(rhs.bottom()));
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator|=(Rect_T<U> const & rhs)
	{
		this->left()	= std::min(this->left(),	static_cast<T>(rhs.left()));
		this->top()		= std::min(this->top(),		static_cast<T>(rhs.top()));
		this->right()	= std::max(this->right(),	static_cast<T>(rhs.right()));
		this->bottom()	= std::max(this->bottom(),	static_cast<T>(rhs.bottom()));
		return *this;
	}

	template <typename T>
	Rect_T<T>& Rect_T<T>::operator=(Rect_T<T> const & rhs)
	{
		if (this != &rhs)
		{
			rect_ = rhs.rect_;
		}
		return *this;
	}

	template <typename T>
	Rect_T<T>& Rect_T<T>::operator=(Rect_T<T>&& rhs)
	{
		rect_ = std::move(rhs.rect_);
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T>& Rect_T<T>::operator=(Rect_T<U> const & rhs)
	{
		rect_ = rhs.rect_;
		return *this;
	}

	template <typename T>
	Rect_T<T> const Rect_T<T>::operator+() const
	{
		return *this;
	}

	template <typename T>
	Rect_T<T> const Rect_T<T>::operator-() const
	{
		return Rect_T<T>(-this->left(), -this->top(), -this->right(), -this->bottom());
	}

	template <typename T>
	T Rect_T<T>::Width() const
	{
		return this->right() - this->left();
	}

	template <typename T>
	T Rect_T<T>::Height() const
	{
		return this->bottom() - this->top();
	}

	template <typename T>
	Size_T<T> const Rect_T<T>::Size() const
	{
		return Size_T<T>(this->Width(), this->Height());
	}

	template <typename T>
	bool Rect_T<T>::IsEmpty() const
	{
		return (this->left() == this->right()) && (this->top() == this->bottom());
	}

	template <typename T>
	bool Rect_T<T>::operator==(Rect_T<T> const & rhs) const
	{
		return rect_ == rhs.rect_;
	}

	template <typename T>
	bool Rect_T<T>::PtInRect(Vector_T<T, 2> const & pt) const
	{
		return MathLib::in_bound(pt.x(), this->left(), this->right())
			&& MathLib::in_bound(pt.y(), this->top(), this->bottom());
	}
}
