/**
 * @file Vector.hpp
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

#ifndef _KFL_VECTOR_HPP
#define _KFL_VECTOR_HPP

#pragma once

#include <array>

#include <boost/operators.hpp>

#include <KFL/Detail/MathHelper.hpp>

namespace KlayGE
{
	template <typename T, int N>
	class Vector_T : boost::addable<Vector_T<T, N>,
						boost::subtractable<Vector_T<T, N>,
						boost::multipliable<Vector_T<T, N>,
						boost::dividable<Vector_T<T, N>,
						boost::dividable2<Vector_T<T, N>, T,
						boost::multipliable2<Vector_T<T, N>, T,
						boost::addable2<Vector_T<T, N>, T,
						boost::subtractable2<Vector_T<T, N>, T,
						boost::equality_comparable<Vector_T<T, N>>>>>>>>>>
	{
		template <typename U, int M>
		friend class Vector_T;

		typedef std::array<T, N>	DetailType;

	public:
		typedef typename DetailType::value_type			value_type;

		typedef value_type*								pointer;
		typedef value_type const *						const_pointer;

		typedef typename DetailType::reference			reference;
		typedef typename DetailType::const_reference	const_reference;

		typedef typename DetailType::iterator			iterator;
		typedef typename DetailType::const_iterator		const_iterator;

		typedef typename DetailType::size_type			size_type;
		typedef typename DetailType::difference_type	difference_type;

		enum { elem_num = N };

	public:
		Vector_T()
		{
		}
		explicit Vector_T(T const * rhs)
		{
			detail::vector_helper<T, N>::DoCopy(&vec_[0], rhs);
		}
		explicit Vector_T(T const & rhs)
		{
			detail::vector_helper<T, N>::DoAssign(&vec_[0], rhs);
		}
		Vector_T(Vector_T const & rhs)
		{
			detail::vector_helper<T, N>::DoCopy(&vec_[0], &rhs[0]);
		}
		Vector_T(Vector_T&& rhs)
			: vec_(std::move(rhs.vec_))
		{
		}
		template <typename U, int M>
		Vector_T(Vector_T<U, M> const & rhs)
		{
			static_assert(M >= N, "Could not convert to a smaller vector.");

			detail::vector_helper<T, N>::DoCopy(&vec_[0], &rhs[0]);
		}

		Vector_T(T const & x, T const & y)
		{
			static_assert(2 == elem_num, "Must be 2D vector.");

			this->x() = x;
			this->y() = y;
		}
		Vector_T(T&& x, T&& y)
		{
			static_assert(2 == elem_num, "Must be 2D vector.");

			vec_[0] = std::move(x);
			vec_[1] = std::move(y);
		}
		Vector_T(T const & x, T const & y, T const & z)
		{
			static_assert(3 == elem_num, "Must be 3D vector.");

			this->x() = x;
			this->y() = y;
			this->z() = z;
		}
		Vector_T(T&& x, T&& y, T&& z)
		{
			static_assert(3 == elem_num, "Must be 3D vector.");

			vec_[0] = std::move(x);
			vec_[1] = std::move(y);
			vec_[2] = std::move(z);
		}
		Vector_T(T const & x, T const & y, T const & z, T const & w)
		{
			static_assert(4 == elem_num, "Must be 4D vector.");

			this->x() = x;
			this->y() = y;
			this->z() = z;
			this->w() = w;
		}
		Vector_T(T&& x, T&& y, T&& z, T&& w)
		{
			static_assert(4 == elem_num, "Must be 4D vector.");

			vec_[0] = std::move(x);
			vec_[1] = std::move(y);
			vec_[2] = std::move(z);
			vec_[3] = std::move(w);
		}

		static size_t size()
		{
			return elem_num;
		}

		static Vector_T const & Zero()
		{
			static Vector_T<T, N> const zero(T(0));
			return zero;
		}

		// ȡ����
		iterator begin()
		{
			return vec_.begin();
		}
		const_iterator begin() const
		{
			return vec_.begin();
		}
		iterator end()
		{
			return vec_.end();
		}
		const_iterator end() const
		{
			return vec_.end();
		}
		reference operator[](size_t index)
		{
			return vec_[index];
		}
		const_reference operator[](size_t index) const
		{
			return vec_[index];
		}

		reference x()
		{
			static_assert(elem_num >= 1, "Must be 1D vector.");
			return vec_[0];
		}
		const_reference x() const
		{
			static_assert(elem_num >= 1, "Must be 1D vector.");
			return vec_[0];
		}

		reference y()
		{
			static_assert(elem_num >= 2, "Must be 2D vector.");
			return vec_[1];
		}
		const_reference y() const
		{
			static_assert(elem_num >= 2, "Must be 2D vector.");
			return vec_[1];
		}

		reference z()
		{
			static_assert(elem_num >= 3, "Must be 3D vector.");
			return vec_[2];
		}
		const_reference z() const
		{
			static_assert(elem_num >= 3, "Must be 3D vector.");
			return vec_[2];
		}

		reference w()
		{
			static_assert(elem_num >= 4, "Must be 4D vector.");
			return vec_[3];
		}
		const_reference w() const
		{
			static_assert(elem_num >= 4, "Must be 4D vector.");
			return vec_[3];
		}

		// ��ֵ������
		template <typename U>
		Vector_T const & operator+=(Vector_T<U, N> const & rhs)
		{
			detail::vector_helper<T, N>::DoAdd(&vec_[0], &vec_[0], &rhs.vec_[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator+=(U const & rhs)
		{
			detail::vector_helper<T, N>::DoAdd(&vec_[0], &vec_[0], rhs);
			return *this;
		}
		template <typename U>
		Vector_T const & operator-=(Vector_T<U, N> const & rhs)
		{
			detail::vector_helper<T, N>::DoSub(&vec_[0], &vec_[0], &rhs.vec_[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator-=(U const & rhs)
		{
			detail::vector_helper<T, N>::DoSub(&vec_[0], &vec_[0], rhs);
			return *this;
		}
		template <typename U>
		Vector_T const & operator*=(Vector_T<U, N> const & rhs)
		{
			detail::vector_helper<T, N>::DoMul(&vec_[0], &vec_[0], &rhs[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator*=(U const & rhs)
		{
			detail::vector_helper<T, N>::DoScale(&vec_[0], &vec_[0], rhs);
			return *this;
		}
		template <typename U>
		Vector_T const & operator/=(Vector_T<U, N> const & rhs)
		{
			detail::vector_helper<T, N>::DoDiv(&vec_[0], &vec_[0], &rhs[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator/=(U const & rhs)
		{
			return this->operator*=(1.0f / rhs);
		}

		Vector_T& operator=(Vector_T const & rhs)
		{
			if (this != &rhs)
			{
				vec_ = rhs.vec_;
			}
			return *this;
		}
		Vector_T& operator=(Vector_T&& rhs)
		{
			vec_ = std::move(rhs.vec_);
			return *this;
		}
		template <typename U, int M>
		Vector_T& operator=(Vector_T<U, M> const & rhs)
		{
			static_assert(M >= N, "Could not assign to a smaller vector.");

			detail::vector_helper<T, N>::DoCopy(&vec_[0], &rhs.vec_[0]);
			return *this;
		}

		// һԪ������
		Vector_T const operator+() const
			{ return *this; }
		Vector_T const operator-() const
		{
			Vector_T temp(*this);
			detail::vector_helper<T, N>::DoNegate(&temp.vec_[0], &vec_[0]);
			return temp;
		}

		void swap(Vector_T& rhs)
		{
			detail::vector_helper<T, N>::DoSwap(&vec_[0], &rhs.vec_[0]);
		}

		bool operator==(Vector_T const & rhs) const
		{
			return detail::vector_helper<T, N>::DoEqual(&vec_[0], &rhs[0]);
		}

	private:
		DetailType vec_;
	};

	template <typename T, int N>
	inline void swap(Vector_T<T, N>& lhs, Vector_T<T, N>& rhs)
	{
		lhs.swap(rhs);
	}
}

#endif			// _KFL_VECTOR_HPP
