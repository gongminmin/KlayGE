/**
 * @file MathHelper.hpp
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

#ifndef _KFL_MATHHELPER_HPP
#define _KFL_MATHHELPER_HPP

#pragma once

namespace KlayGE
{
	template <typename T, int N>
	class Vector_T;
	template <typename T>
	class Matrix4_T;

	namespace detail
	{
		template <typename T, int N>
		struct dot_helper
		{
			static T Do(T const * lhs, T const * rhs) noexcept
			{
				return lhs[0] * rhs[0] + dot_helper<T, N - 1>::Do(lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct dot_helper<T, 1>
		{
			static T Do(T const * lhs, T const * rhs) noexcept
			{
				return lhs[0] * rhs[0];
			}
		};

		template <typename T, int N>
		struct max_minimize_helper
		{
			static void DoMax(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
				max_minimize_helper<T, N - 1>::DoMax(out + 1, lhs + 1, rhs + 1);
			}

			static void DoMin(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
				max_minimize_helper<T, N - 1>::DoMin(out + 1, lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct max_minimize_helper<T, 1>
		{
			static void DoMax(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
			}

			static void DoMin(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
			}
		};

		template <typename T, int N>
		struct transform_helper
		{
			static Vector_T<T, 4> Do(Vector_T<T, N> const & v, Matrix4_T<T> const & mat) noexcept;
		};
		template <typename T>
		struct transform_helper<T, 4>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 4> const & v, Matrix4_T<T> const & mat) noexcept
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + v.w() * mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + v.w() * mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + v.w() * mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + v.w() * mat(3, 3));
			}
		};
		template <typename T>
		struct transform_helper<T, 3>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 3> const & v, Matrix4_T<T> const & mat) noexcept
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + mat(3, 3));
			}
		};
		template <typename T>
		struct transform_helper<T, 2>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 2> const & v, Matrix4_T<T> const & mat) noexcept
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + mat(3, 3));
			}
		};

		template <typename T, int N>
		struct transform_normal_helper
		{
			static Vector_T<T, N> Do(Vector_T<T, N> const & v, Matrix4_T<T> const & mat) noexcept;
		};
		template <typename T>
		struct transform_normal_helper<T, 3>
		{
			static Vector_T<T, 3> Do(Vector_T<T, 3> const & v, Matrix4_T<T> const & mat) noexcept
			{
				Vector_T<T, 4> temp(v.x(), v.y(), v.z(), T(0));
				temp = transform_helper<T, 4>::Do(temp, mat);
				return Vector_T<T, 3>(temp.x(), temp.y(), temp.z());
			}
		};
		template <typename T>
		struct transform_normal_helper<T, 2>
		{
			static Vector_T<T, 2> Do(Vector_T<T, 2> const & v, Matrix4_T<T> const & mat) noexcept
			{
				Vector_T<T, 3> temp(v.x(), v.y(), T(0));
				temp = transform_normal_helper<T, 3>::Do(temp, mat);
				return Vector_T<T, 2>(temp.x(), temp.y());
			}
		};

		template <typename T, int N>
		struct vector_helper
		{
			template <typename U>
			static void DoCopy(T out[N], U const rhs[N]) noexcept
			{
				out[0] = static_cast<T>(rhs[0]);
				vector_helper<T, N - 1>::DoCopy(out + 1, rhs + 1);
			}

			static void DoAssign(T out[N], T const & rhs) noexcept
			{
				out[0] = rhs;
				vector_helper<T, N - 1>::DoAssign(out + 1, rhs);
			}

			static void DoAdd(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = lhs[0] + rhs[0];
				vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs + 1);
			}

			static void DoAdd(T out[N], T const lhs[N], T const & rhs) noexcept
			{
				out[0] = lhs[0] + rhs;
				vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs);
			}

			static void DoSub(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = lhs[0] - rhs[0];
				vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs + 1);
			}

			static void DoSub(T out[N], T const lhs[N], T const & rhs) noexcept
			{
				out[0] = lhs[0] - rhs;
				vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs);
			}

			static void DoMul(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = lhs[0] * rhs[0];
				vector_helper<T, N - 1>::DoMul(out + 1, lhs + 1, rhs + 1);
			}

			static void DoScale(T out[N], T const lhs[N], T const & rhs) noexcept
			{
				out[0] = lhs[0] * rhs;
				vector_helper<T, N - 1>::DoScale(out + 1, lhs + 1, rhs);
			}

			static void DoDiv(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = lhs[0] / rhs[0];
				vector_helper<T, N - 1>::DoDiv(out + 1, lhs + 1, rhs + 1);
			}

			static void DoNegate(T out[N], T const rhs[N]) noexcept
			{
				out[0] = -rhs[0];
				vector_helper<T, N - 1>::DoNegate(out + 1, rhs + 1);
			}

			static bool DoEqual(T const lhs[N], T const rhs[N]) noexcept
			{
				return vector_helper<T, 1>::DoEqual(lhs, rhs) && vector_helper<T, N - 1>::DoEqual(lhs + 1, rhs + 1);
			}

			static void DoSwap(T lhs[N], T rhs[N]) noexcept
			{
				std::swap(lhs[0], rhs[0]);
				return vector_helper<T, N - 1>::DoSwap(lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct vector_helper<T, 1>
		{
			template <typename U>
			static void DoCopy(T out[1], U const rhs[1]) noexcept
			{
				out[0] = static_cast<T>(rhs[0]);
			}

			static void DoAssign(T out[1], T const & rhs) noexcept
			{
				out[0] = rhs;
			}

			static void DoAdd(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = lhs[0] + rhs[0];
			}

			static void DoAdd(T out[1], T const lhs[1], T const rhs) noexcept
			{
				out[0] = lhs[0] + rhs;
			}

			static void DoSub(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = lhs[0] - rhs[0];
			}

			static void DoSub(T out[1], T const lhs[1], T const & rhs) noexcept
			{
				out[0] = lhs[0] - rhs;
			}

			static void DoMul(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = lhs[0] * rhs[0];
			}

			static void DoScale(T out[1], T const lhs[1], T const & rhs) noexcept
			{
				out[0] = lhs[0] * rhs;
			}

			static void DoDiv(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = lhs[0] / rhs[0];
			}

			static void DoNegate(T out[1], T const rhs[1]) noexcept
			{
				out[0] = -rhs[0];
			}

			static bool DoEqual(T const lhs[1], T const rhs[1]) noexcept
			{
				return lhs[0] == rhs[0];
			}

			static void DoSwap(T lhs[1], T rhs[1]) noexcept
			{
				std::swap(lhs[0], rhs[0]);
			}
		};
	}
}

#endif			// _KFL_MATHHELPER_HPP
