/**
 * @file Matrix.hpp
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

#ifndef _KFL_MATRIX_HPP
#define _KFL_MATRIX_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <boost/operators.hpp>

#include <KFL/Vector.hpp>

namespace KlayGE
{
	// 4D 矩阵
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Matrix4_T : boost::addable<Matrix4_T<T>,
						boost::subtractable<Matrix4_T<T>,
						boost::dividable2<Matrix4_T<T>, T,
						boost::multipliable2<Matrix4_T<T>, T,
						boost::multipliable<Matrix4_T<T>,
						boost::equality_comparable<Matrix4_T<T>>>>>>>
	{
	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		enum { row_num = 4, col_num = 4 };
		enum { elem_num = row_num * col_num };

	public:
		Matrix4_T() KLAYGE_NOEXCEPT
		{
		}
		explicit Matrix4_T(T const * rhs) KLAYGE_NOEXCEPT;
		Matrix4_T(Matrix4_T const & rhs) KLAYGE_NOEXCEPT;
		Matrix4_T(Matrix4_T&& rhs) KLAYGE_NOEXCEPT;
		Matrix4_T(T f11, T f12, T f13, T f14,
			T f21, T f22, T f23, T f24,
			T f31, T f32, T f33, T f34,
			T f41, T f42, T f43, T f44) KLAYGE_NOEXCEPT;

		static size_t size() KLAYGE_NOEXCEPT
		{
			return elem_num;
		}

		static Matrix4_T const & Zero() KLAYGE_NOEXCEPT;
		static Matrix4_T const & Identity() KLAYGE_NOEXCEPT;

		reference operator()(size_t row, size_t col) KLAYGE_NOEXCEPT
		{
			return m_[row][col];
		}
		const_reference operator()(size_t row, size_t col) const KLAYGE_NOEXCEPT
		{
			return m_[row][col];
		}
		iterator begin() KLAYGE_NOEXCEPT
		{
			return &m_[0][0];
		}
		const_iterator begin() const KLAYGE_NOEXCEPT
		{
			return &m_[0][0];
		}
		iterator end() KLAYGE_NOEXCEPT
		{
			return this->begin() + elem_num;
		}
		const_iterator end() const KLAYGE_NOEXCEPT
		{
			return this->begin() + elem_num;
		}
		reference operator[](size_t index) KLAYGE_NOEXCEPT
		{
			return *(this->begin() + index);
		}
		const_reference operator[](size_t index) const KLAYGE_NOEXCEPT
		{
			return *(this->begin() + index);
		}

		void Row(size_t index, Vector_T<T, col_num> const & rhs) KLAYGE_NOEXCEPT;
		Vector_T<T, 4> const & Row(size_t index) const KLAYGE_NOEXCEPT;
		void Col(size_t index, Vector_T<T, row_num> const & rhs) KLAYGE_NOEXCEPT;
		Vector_T<T, 4> const Col(size_t index) const KLAYGE_NOEXCEPT;

		// 赋值操作符
		Matrix4_T& operator+=(Matrix4_T const & rhs) KLAYGE_NOEXCEPT;
		Matrix4_T& operator-=(Matrix4_T const & rhs) KLAYGE_NOEXCEPT;
		Matrix4_T& operator*=(Matrix4_T const & rhs) KLAYGE_NOEXCEPT;
		Matrix4_T& operator*=(T rhs) KLAYGE_NOEXCEPT;
		Matrix4_T& operator/=(T rhs) KLAYGE_NOEXCEPT;

		Matrix4_T& operator=(Matrix4_T const & rhs) KLAYGE_NOEXCEPT;
		Matrix4_T& operator=(Matrix4_T&& rhs) KLAYGE_NOEXCEPT;

		// 一元操作符
		Matrix4_T const operator+() const KLAYGE_NOEXCEPT;
		Matrix4_T const operator-() const KLAYGE_NOEXCEPT;

		bool operator==(Matrix4_T const & rhs) const KLAYGE_NOEXCEPT;

	private:
		Vector_T<Vector_T<T, col_num>, row_num> m_;
	};
}

#endif			// _KFL_MATRIX_HPP
