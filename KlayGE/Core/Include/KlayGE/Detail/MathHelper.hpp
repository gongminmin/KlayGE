// MathHelper.hpp
// KlayGE 数学库实现细节 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.718)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _MATHHELPER_HPP
#define _MATHHELPER_HPP

#include <KlayGE/PreDeclare.hpp>

#include <boost/static_assert.hpp>

namespace KlayGE
{
	namespace detail
	{
		template <typename T, int N>
		struct dot_helper
		{
			static T Do(T const * lhs, T const * rhs)
			{
				return lhs[0] * rhs[0] + dot_helper<T, N - 1>::Do(lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct dot_helper<T, 1>
		{
			static T Do(T const * lhs, T const * rhs)
			{
				return lhs[0] * rhs[0];
			}
		};

		template <typename T, int N>
		struct max_minimize_helper
		{
			static void DoMax(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
				max_minimize_helper<T, N - 1>::DoMax(out + 1, lhs + 1, rhs + 1);
			}

			static void DoMin(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
				max_minimize_helper<T, N - 1>::DoMin(out + 1, lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct max_minimize_helper<T, 1>
		{
			static void DoMax(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
			}

			static void DoMin(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
			}
		};

		template <typename T, int N>
		struct transform_helper
		{
			static Vector_T<T, 4> Do(Vector_T<T, N> const & v, Matrix4_T<T> const & mat);
		};
		template <typename T>
		struct transform_helper<T, 4>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 4> const & v, Matrix4_T<T> const & mat)
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
			static Vector_T<T, 4> Do(Vector_T<T, 3> const & v, Matrix4_T<T> const & mat)
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
			static Vector_T<T, 4> Do(Vector_T<T, 2> const & v, Matrix4_T<T> const & mat)
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
			static Vector_T<T, N> Do(Vector_T<T, N> const & v, Matrix4_T<T> const & mat);
		};
		template <typename T>
		struct transform_normal_helper<T, 3>
		{
			static Vector_T<T, 3> Do(Vector_T<T, 3> const & v, Matrix4_T<T> const & mat)
			{
				Vector_T<T, 4> temp(v.x(), v.y(), v.z(), T(0));
				temp = transform_helper<T, 4>::Do(temp, mat);
				return Vector_T<T, 3>(temp.x(), temp.y(), temp.z());
			}
		};
		template <typename T>
		struct transform_normal_helper<T, 2>
		{
			static Vector_T<T, 2> Do(Vector_T<T, 2> const & v, Matrix4_T<T> const & mat)
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
			static void DoCopy(T out[N], U const rhs[N])
			{
				out[0] = static_cast<T>(rhs[0]);
				vector_helper<T, N - 1>::DoCopy(out + 1, rhs + 1);
			}

			static void DoAssign(T out[N], T const & rhs)
			{
				out[0] = rhs;
				vector_helper<T, N - 1>::DoAssign(out + 1, rhs);
			}

			static void DoAdd(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] + rhs[0];
				vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs + 1);
			}

			static void DoAdd(T out[N], T const lhs[N], T const & rhs)
			{
				out[0] = lhs[0] + rhs;
				vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs);
			}

			static void DoSub(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] - rhs[0];
				vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs + 1);
			}

			static void DoSub(T out[N], T const lhs[N], T const & rhs)
			{
				out[0] = lhs[0] - rhs;
				vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs);
			}

			static void DoMul(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] * rhs[0];
				vector_helper<T, N - 1>::DoMul(out + 1, lhs + 1, rhs + 1);
			}

			static void DoScale(T out[N], T const lhs[N], T const & rhs)
			{
				out[0] = lhs[0] * rhs;
				vector_helper<T, N - 1>::DoScale(out + 1, lhs + 1, rhs);
			}

			static void DoDiv(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] / rhs[0];
				vector_helper<T, N - 1>::DoMul(out + 1, lhs + 1, rhs + 1);
			}

			static void DoNegate(T out[N], T const rhs[N])
			{
				out[0] = -rhs[0];
				vector_helper<T, N - 1>::DoNegate(out + 1, rhs + 1);
			}

			static bool DoEqual(T const lhs[N], T const rhs[N])
			{
				return vector_helper<T, 1>::DoEqual(lhs, rhs) && vector_helper<T, N - 1>::DoEqual(lhs + 1, rhs + 1);
			}

			static void DoSwap(T lhs[N], T rhs[N])
			{
				std::swap(lhs[0], rhs[0]);
				return vector_helper<T, N - 1>::DoSwap(lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct vector_helper<T, 1>
		{
			template <typename U>
			static void DoCopy(T out[1], U const rhs[1])
			{
				out[0] = static_cast<T>(rhs[0]);
			}

			static void DoAssign(T out[1], T const & rhs)
			{
				out[0] = rhs;
			}

			static void DoAdd(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] + rhs[0];
			}

			static void DoAdd(T out[1], T const lhs[1], T const rhs)
			{
				out[0] = lhs[0] + rhs;
			}

			static void DoSub(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] - rhs[0];
			}

			static void DoSub(T out[1], T const lhs[1], T const & rhs)
			{
				out[0] = lhs[0] - rhs;
			}

			static void DoMul(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] * rhs[0];
			}

			static void DoScale(T out[1], T const lhs[1], T const & rhs)
			{
				out[0] = lhs[0] * rhs;
			}

			static void DoDiv(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] / rhs[0];
			}

			static void DoNegate(T out[1], T const rhs[1])
			{
				out[0] = -rhs[0];
			}

			static bool DoEqual(T const lhs[1], T const rhs[1])
			{
				return lhs[0] == rhs[0];
			}

			static void DoSwap(T lhs[1], T rhs[1])
			{
				std::swap(lhs[0], rhs[0]);
			}
		};
	}
}

#endif		// _MATHHELPER_HPP
