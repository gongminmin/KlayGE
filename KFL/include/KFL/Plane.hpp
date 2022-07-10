/**
 * @file Plane.hpp
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

#ifndef _KFL_PLANE_HPP
#define _KFL_PLANE_HPP

#pragma once

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

namespace KlayGE
{
	// ����һ��ƽ�� ax + by + cz + d = 0
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Plane_T final : boost::equality_comparable<Plane_T<T>>
	{
	public:
		static constexpr size_t elem_num = 4;

		typedef T value_type;

		typedef typename Vector_T<T, elem_num>::pointer pointer;
		typedef typename Vector_T<T, elem_num>::const_pointer const_pointer;

		typedef typename Vector_T<T, elem_num>::reference reference;
		typedef typename Vector_T<T, elem_num>::const_reference const_reference;

		typedef typename Vector_T<T, elem_num>::iterator iterator;
		typedef typename Vector_T<T, elem_num>::const_iterator const_iterator;

	public:
		constexpr Plane_T() noexcept
		{
		}
		explicit constexpr Plane_T(T const* rhs) noexcept : plane_(rhs)
		{
		}
		constexpr Plane_T(Plane_T const& rhs) noexcept : plane_(rhs.plane_)
		{
		}
		constexpr Plane_T(Plane_T&& rhs) noexcept : plane_(std::move(rhs.plane_))
		{
		}
		explicit constexpr Plane_T(Vector_T<T, elem_num> rhs) noexcept : plane_(std::move(rhs))
		{
		}
		constexpr Plane_T(T a, T b, T c, T d) noexcept : plane_(std::move(a), std::move(b), std::move(c), std::move(d))
		{
		}

		static constexpr size_t size() noexcept
		{
			return elem_num;
		}

		// ȡ����
		constexpr iterator begin() noexcept
		{
			return plane_.begin();
		}
		constexpr const_iterator begin() const noexcept
		{
			return plane_.begin();
		}
		constexpr iterator end() noexcept
		{
			return plane_.end();
		}
		constexpr const_iterator end() const noexcept
		{
			return plane_.end();
		}
		constexpr reference operator[](size_t index) noexcept
		{
			return plane_[index];
		}
		constexpr const_reference operator[](size_t index) const noexcept
		{
			return plane_[index];
		}

		constexpr reference a() noexcept
		{
			return plane_[0];
		}
		constexpr const_reference a() const noexcept
		{
			return plane_[0];
		}
		constexpr reference b() noexcept
		{
			return plane_[1];
		}
		constexpr const_reference b() const noexcept
		{
			return plane_[1];
		}
		constexpr reference c() noexcept
		{
			return plane_[2];
		}
		constexpr const_reference c() const noexcept
		{
			return plane_[2];
		}
		constexpr reference d() noexcept
		{
			return plane_[3];
		}
		constexpr const_reference d() const noexcept
		{
			return plane_[3];
		}

		// ��ֵ������
		Plane_T& operator=(Plane_T const & rhs) noexcept;
		Plane_T& operator=(Plane_T&& rhs) noexcept;
		Plane_T& operator=(Vector_T<T, elem_num> const & rhs) noexcept;
		Plane_T& operator=(Vector_T<T, elem_num>&& rhs) noexcept;

		// һԪ������
		constexpr Plane_T const& operator+() const noexcept
		{
			return *this;
		}
		Plane_T const operator-() const noexcept;

		// ȡ��������
		constexpr Vector_T<T, 3> const& Normal() const noexcept
		{
			return plane_.template AsVector<3>();
		}
		void Normal(Vector_T<T, 3> const & rhs) noexcept;

		constexpr Vector_T<T, 4> const& AsVector4() const noexcept
		{
			return plane_;
		}

		bool operator==(Plane_T<T> const & rhs) const noexcept;

	private:
		Vector_T<T, elem_num> plane_;
	};
}

#endif			// _KFL_PLANE_HPP
