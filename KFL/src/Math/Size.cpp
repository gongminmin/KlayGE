/**
 * @file Size.cpp
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

#include <KFL/Size.hpp>

namespace KlayGE
{
	template Size_T<float>::Size_T(float const * rhs) noexcept;
	template Size_T<float>::Size_T(Size&& rhs) noexcept;
	template Size_T<float>::Size_T(float cx, float cy) noexcept;
	template Size const & Size_T<float>::operator+=(Size const & rhs) noexcept;
	template Size const & Size_T<float>::operator+=(ISize const & rhs) noexcept;
	template Size const & Size_T<float>::operator+=(UISize const & rhs) noexcept;
	template Size const & Size_T<float>::operator-=(Size const & rhs) noexcept;
	template Size const & Size_T<float>::operator-=(ISize const & rhs) noexcept;
	template Size const & Size_T<float>::operator-=(UISize const & rhs) noexcept;
	template Size& Size_T<float>::operator=(Size&& rhs) noexcept;
	template Size const Size_T<float>::operator+() const noexcept;
	template Size const Size_T<float>::operator-() const noexcept;
	template bool Size_T<float>::operator==(Size const & rhs) const noexcept;

	template Size_T<int32_t>::Size_T(int32_t const * rhs) noexcept;
	template Size_T<int32_t>::Size_T(ISize&& rhs) noexcept;
	template Size_T<int32_t>::Size_T(int32_t cx, int32_t cy) noexcept;
	template ISize const & Size_T<int32_t>::operator+=(Size const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator+=(ISize const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator+=(UISize const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator-=(Size const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator-=(ISize const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator-=(UISize const & rhs) noexcept;
	template ISize& Size_T<int32_t>::operator=(ISize&& rhs) noexcept;
	template ISize const Size_T<int32_t>::operator+() const noexcept;
	template ISize const Size_T<int32_t>::operator-() const noexcept;
	template bool Size_T<int32_t>::operator==(ISize const & rhs) const noexcept;

	template Size_T<uint32_t>::Size_T(uint32_t const * rhs) noexcept;
	template Size_T<uint32_t>::Size_T(UISize&& rhs) noexcept;
	template Size_T<uint32_t>::Size_T(uint32_t cx, uint32_t cy) noexcept;
	template UISize const & Size_T<uint32_t>::operator+=(Size const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator+=(ISize const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator+=(UISize const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator-=(Size const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator-=(ISize const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator-=(UISize const & rhs) noexcept;
	template UISize& Size_T<uint32_t>::operator=(UISize&& rhs) noexcept;
	template UISize const Size_T<uint32_t>::operator+() const noexcept;
	template bool Size_T<uint32_t>::operator==(UISize const & rhs) const noexcept;


	template <typename T>
	Size_T<T>::Size_T(T const * rhs) noexcept
		: size_(rhs)
	{
	}

	template <typename T>
	Size_T<T>::Size_T(T cx, T cy) noexcept
	{
		this->cx() = cx;
		this->cy() = cy;
	}

	template <typename T>
	Size_T<T>::Size_T(Size_T<T>&& rhs) noexcept
		: size_(std::move(rhs.size_))
	{
	}

	template <typename T>
	template <typename U>
	Size_T<T> const & Size_T<T>::operator+=(Size_T<U> const & rhs) noexcept
	{
		size_ += Size_T<T>(rhs).size_;
		return *this;
	}

	template <typename T>
	template <typename U>
	Size_T<T> const & Size_T<T>::operator-=(Size_T<U> const & rhs) noexcept
	{
		size_ -= Size_T<T>(rhs).size_;
		return *this;
	}

	template <typename T>
	Size_T<T>& Size_T<T>::operator=(Size_T<T>&& rhs) noexcept
	{
		size_ = std::move(rhs.size_);
		return *this;
	}

	template <typename T>
	Size_T<T> const Size_T<T>::operator+() const noexcept
	{
		return *this;
	}

	template <typename T>
	Size_T<T> const Size_T<T>::operator-() const noexcept
	{
		return Size_T<T>(-this->cx(), -this->cy());
	}

	template <typename T>
	bool Size_T<T>::operator==(Size_T<T> const & rhs) const noexcept
	{
		return size_ == rhs.size_;
	}
}
