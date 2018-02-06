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
	template <typename T>
	Rect_T<T>::Rect_T(Rect_T&& rhs) noexcept
		: rect_(std::move(rhs.rect_))
	{
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator+=(Vector_T<U, 2> const & rhs) noexcept
	{
		this->left() += static_cast<T>(rhs.x());
		this->right() += static_cast<T>(rhs.x());
		this->top() += static_cast<T>(rhs.y());
		this->bottom() += static_cast<T>(rhs.y());
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator-=(Vector_T<U, 2> const & rhs) noexcept
	{
		this->left() -= static_cast<T>(rhs.x());
		this->right() -= static_cast<T>(rhs.x());
		this->top() -= static_cast<T>(rhs.y());
		this->bottom() -= static_cast<T>(rhs.y());
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator+=(Rect_T<U> const & rhs) noexcept
	{
		rect_ += Rect_T<T>(rhs).rect_;
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator-=(Rect_T<U> const & rhs) noexcept
	{
		rect_ -= Rect_T<T>(rhs).rect_;
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator&=(Rect_T<U> const & rhs) noexcept
	{
		this->left()	= std::max(this->left(),	static_cast<T>(rhs.left()));
		this->top()		= std::max(this->top(),		static_cast<T>(rhs.top()));
		this->right()	= std::min(this->right(),	static_cast<T>(rhs.right()));
		this->bottom()	= std::min(this->bottom(),	static_cast<T>(rhs.bottom()));
		return *this;
	}

	template <typename T>
	template <typename U>
	Rect_T<T> const & Rect_T<T>::operator|=(Rect_T<U> const & rhs) noexcept
	{
		this->left()	= std::min(this->left(),	static_cast<T>(rhs.left()));
		this->top()		= std::min(this->top(),		static_cast<T>(rhs.top()));
		this->right()	= std::max(this->right(),	static_cast<T>(rhs.right()));
		this->bottom()	= std::max(this->bottom(),	static_cast<T>(rhs.bottom()));
		return *this;
	}

	template <typename T>
	Rect_T<T>& Rect_T<T>::operator=(Rect_T<T>&& rhs) noexcept
	{
		rect_ = std::move(rhs.rect_);
		return *this;
	}

	template <typename T>
	Rect_T<T> const Rect_T<T>::operator+() const noexcept
	{
		return *this;
	}

	template <typename T>
	Rect_T<T> const Rect_T<T>::operator-() const noexcept
	{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4146) // Unary minus operator on uint32_t
#endif
		return Rect_T<T>(-this->left(), -this->top(), -this->right(), -this->bottom());
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}

	template <typename T>
	T Rect_T<T>::Width() const noexcept
	{
		return this->right() - this->left();
	}

	template <typename T>
	T Rect_T<T>::Height() const noexcept
	{
		return this->bottom() - this->top();
	}

	template <typename T>
	Size_T<T> const Rect_T<T>::Size() const noexcept
	{
		return Size_T<T>(this->Width(), this->Height());
	}

	template <typename T>
	bool Rect_T<T>::IsEmpty() const noexcept
	{
		return (this->left() == this->right()) && (this->top() == this->bottom());
	}

	template <typename T>
	bool Rect_T<T>::operator==(Rect_T<T> const & rhs) const noexcept
	{
		return rect_ == rhs.rect_;
	}

	template <typename T>
	bool Rect_T<T>::PtInRect(Vector_T<T, 2> const & pt) const noexcept
	{
		return MathLib::in_bound(pt.x(), this->left(), this->right())
			&& MathLib::in_bound(pt.y(), this->top(), this->bottom());
	}


	template class Rect_T<float>;
	template class Rect_T<int32_t>;
	template class Rect_T<uint32_t>;

	template Rect const & Rect_T<float>::operator+=(float2 const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator+=(int2 const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator+=(uint2 const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator-=(float2 const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator-=(int2 const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator-=(uint2 const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator+=(Rect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator+=(IRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator+=(UIRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator-=(Rect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator-=(IRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator-=(UIRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator&=(Rect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator&=(IRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator&=(UIRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator|=(Rect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator|=(IRect const & rhs) noexcept;
	template Rect const & Rect_T<float>::operator|=(UIRect const & rhs) noexcept;

	template IRect const & Rect_T<int32_t>::operator+=(float2 const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator+=(int2 const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator+=(uint2 const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator-=(float2 const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator-=(int2 const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator-=(uint2 const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator+=(Rect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator+=(IRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator+=(UIRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator-=(Rect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator-=(IRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator-=(UIRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator&=(Rect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator&=(IRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator&=(UIRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator|=(Rect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator|=(IRect const & rhs) noexcept;
	template IRect const & Rect_T<int32_t>::operator|=(UIRect const & rhs) noexcept;

	template UIRect const & Rect_T<uint32_t>::operator+=(float2 const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator+=(int2 const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator+=(uint2 const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator-=(float2 const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator-=(int2 const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator-=(uint2 const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator+=(Rect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator+=(IRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator+=(UIRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator-=(Rect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator-=(IRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator-=(UIRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator&=(Rect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator&=(IRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator&=(UIRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator|=(Rect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator|=(IRect const & rhs) noexcept;
	template UIRect const & Rect_T<uint32_t>::operator|=(UIRect const & rhs) noexcept;
}
