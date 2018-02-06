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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4146) // Unary minus operator on uint32_t
#endif
		return Size_T<T>(-this->cx(), -this->cy());
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

	}

	template <typename T>
	bool Size_T<T>::operator==(Size_T<T> const & rhs) const noexcept
	{
		return size_ == rhs.size_;
	}


	template class Size_T<float>;
	template class Size_T<int32_t>;
	template class Size_T<uint32_t>;

	template Size const & Size_T<float>::operator+=(Size const & rhs) noexcept;
	template Size const & Size_T<float>::operator+=(ISize const & rhs) noexcept;
	template Size const & Size_T<float>::operator+=(UISize const & rhs) noexcept;
	template Size const & Size_T<float>::operator-=(Size const & rhs) noexcept;
	template Size const & Size_T<float>::operator-=(ISize const & rhs) noexcept;
	template Size const & Size_T<float>::operator-=(UISize const & rhs) noexcept;

	template ISize const & Size_T<int32_t>::operator+=(Size const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator+=(ISize const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator+=(UISize const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator-=(Size const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator-=(ISize const & rhs) noexcept;
	template ISize const & Size_T<int32_t>::operator-=(UISize const & rhs) noexcept;

	template UISize const & Size_T<uint32_t>::operator+=(Size const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator+=(ISize const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator+=(UISize const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator-=(Size const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator-=(ISize const & rhs) noexcept;
	template UISize const & Size_T<uint32_t>::operator-=(UISize const & rhs) noexcept;
}
