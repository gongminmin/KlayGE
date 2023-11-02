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

#include <KFL/Operators.hpp>
#include <KFL/Vector.hpp>

namespace KlayGE
{
	// 4D 矩阵
	///////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class Matrix4_T final
	{
	public:
		typedef T					value_type;

		typedef value_type*			pointer;
		typedef value_type const *	const_pointer;

		typedef value_type&			reference;
		typedef value_type const &	const_reference;

		typedef value_type*			iterator;
		typedef value_type const *	const_iterator;

		static constexpr size_t row_num = 4;
		static constexpr size_t col_num = 4;
		static constexpr size_t elem_num = row_num * col_num;

	public:
		constexpr Matrix4_T() noexcept
		{
		}
		explicit Matrix4_T(T const * rhs) noexcept;
		constexpr Matrix4_T(Matrix4_T const& rhs) noexcept : m_(rhs.m_)
		{
		}
		constexpr Matrix4_T(Matrix4_T&& rhs) noexcept : m_(std::move(rhs.m_))
		{
		}
		Matrix4_T(T f11, T f12, T f13, T f14,
			T f21, T f22, T f23, T f24,
			T f31, T f32, T f33, T f34,
			T f41, T f42, T f43, T f44) noexcept;

		static constexpr size_t size() noexcept
		{
			return elem_num;
		}

		static Matrix4_T const & Zero() noexcept;
		static Matrix4_T const & Identity() noexcept;

		constexpr reference operator()(size_t row, size_t col) noexcept
		{
			return m_[row][col];
		}
		constexpr const_reference operator()(size_t row, size_t col) const noexcept
		{
			return m_[row][col];
		}
		constexpr iterator begin() noexcept
		{
			return &m_[0][0];
		}
		constexpr const_iterator begin() const noexcept
		{
			return &m_[0][0];
		}
		constexpr iterator end() noexcept
		{
			return this->begin() + elem_num;
		}
		constexpr const_iterator end() const noexcept
		{
			return this->begin() + elem_num;
		}
		constexpr reference operator[](size_t index) noexcept
		{
			return this->data()[index];
		}
		constexpr const_reference operator[](size_t index) const noexcept
		{
			return this->data()[index];
		}

		constexpr pointer data() noexcept
		{
			return &m_[0][0];
		}
		constexpr const_pointer data() const noexcept
		{
			return &m_[0][0];
		}

		void Row(size_t index, Vector_T<T, col_num> const & rhs) noexcept;
		Vector_T<T, 4> const & Row(size_t index) const noexcept;
		void Col(size_t index, Vector_T<T, row_num> const & rhs) noexcept;
		Vector_T<T, 4> const Col(size_t index) const noexcept;

		// 赋值操作符
		Matrix4_T& operator+=(Matrix4_T const & rhs) noexcept;
		Matrix4_T& operator-=(Matrix4_T const & rhs) noexcept;
		Matrix4_T& operator*=(Matrix4_T const & rhs) noexcept;
		Matrix4_T& operator*=(T const& rhs) noexcept;
		Matrix4_T& operator/=(T const& rhs) noexcept;

		Matrix4_T& operator=(Matrix4_T const & rhs) noexcept;
		Matrix4_T& operator=(Matrix4_T&& rhs) noexcept;

		// 一元操作符
		constexpr Matrix4_T const& operator+() const noexcept
		{
			return *this;
		}
		Matrix4_T const operator-() const noexcept;

		bool operator==(Matrix4_T const & rhs) const noexcept;

		KLAYGE_DEFAULT_ADD_OPERATOR1(Matrix4_T<T>);
		KLAYGE_DEFAULT_SUB_OPERATOR1(Matrix4_T<T>);
		KLAYGE_DEFAULT_MUL_OPERATOR1(Matrix4_T<T>);
		KLAYGE_DEFAULT_MUL_OPERATOR2(Matrix4_T<T>, T);
		KLAYGE_DEFAULT_MUL_OPERATOR3(T, Matrix4_T<T>);
		KLAYGE_DEFAULT_DIV_OPERATOR2(Matrix4_T<T>, T);

		KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(Matrix4_T<T>);

	private:
		Vector_T<Vector_T<T, col_num>, row_num> m_;
	};
}

#endif			// _KFL_MATRIX_HPP
