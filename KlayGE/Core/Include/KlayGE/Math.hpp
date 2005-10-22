// Math.hpp
// KlayGE 数学函数库 头文件
// Ver 2.6.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.6.0
// 改进了ComputeBoundingSphere (2005.5.23)
//
// 2.5.0
// 改为通过返回值返回结果 (2005.4.12)
//
// 2.1.1
// 修改了自定义类型 (2004.4.22)
// 增加了网格函数 (2004.5.18)
//
// 2.0.4
// 修改了Random的接口 (2004.3.29)
//
// 2.0.0
// 初次建立 (2003.9.18)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _MATH_HPP
#define _MATH_HPP

#include <KlayGE/PreDeclare.hpp>

#include <limits>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iterator>

#include <boost/static_assert.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 常量定义
	/////////////////////////////////////////////////////////////////////////////////
	float const PI		= 3.141592f;			// PI
	float const PI2		= 6.283185f;			// PI * 2
	float const PIdiv2	= 1.570796f;			// PI / 2
		  
	float const DEG90	= 1.570796f;			// 90 度
	float const DEG270	= -1.570796f;			// 270 度
	float const DEG45	= 0.7853981f;			// 45 度
	float const DEG5	= 0.0872664f;			// 5 度
	float const DEG10	= 0.1745329f;			// 10 度
	float const DEG20	= 0.3490658f;			// 20 度 
	float const DEG30	= 0.5235987f;			// 30 度
	float const DEG60	= 1.047197f;			// 60 度
	float const DEG120	= 2.094395f;			// 120 度
		  
	float const DEG40	= 0.6981317f;			// 40 度
	float const DEG80	= 1.396263f;			// 80 度
	float const DEG140	= 2.443460f;			// 140 度
	float const DEG160	= 2.792526f;			// 160 度
		  
	float const SQRT2	= 1.414213f;			// 根2
	float const SQRT_2	= 0.7071068f;			// 1 / SQRT2
	float const SQRT3	= 1.732050f;			// 根3
		  
	float const DEG2RAD	= 0.01745329f;			// 角度化弧度因数
	float const RAD2DEG	= 57.29577f;			// 弧度化角度因数

	namespace MathLib
	{
		// 求绝对值
		template <typename T>
		inline T
		Abs(T const & x)
		{
			return x < T(0) ? -x : x;
		}

		// 取符号
		template <typename T>
		inline T
		Sgn(T const & x)
		{
			return x < T(0) ? T(-1) : (x > T(0) ? T(1) : T(0));
		}
		
		// 平方
		template <typename T>
		inline T
		Sqr(T const & x)
		{
			return x * x;
		}
		// 立方
		template <typename T>
		inline T
		Cube(T const & x)
		{
			return Sqr(x) * x;
		}

		// 角度化弧度
		template <typename T>
		inline T
		Deg2Rad(T const & x)
		{
			return static_cast<T>(x * DEG2RAD);
		}
		// 弧度化角度
		template <typename T>
		inline T
		Rad2Deg(T const & x)
		{
			return static_cast<T>(x * RAD2DEG);
		}

		// 四舍五入
		template <typename T>
		inline T
		Round(T const & x)
		{
			return (x > 0) ? static_cast<T>(static_cast<int>(T(0.5) + x)) :
					-static_cast<T>(static_cast<int>(T(0.5) - x));
		}
		// 取整
		template <typename T>
		inline T
		Trunc(T const & x)
		{
			return static_cast<T>(static_cast<int>(x));
		}

		// 取三个中小的
		template <typename T>
		inline T const &
		Min3(T const & a, T const & b, T const & c)
		{
			return std::min(std::min(a, b), c);
		}
		// 取三个中大的
		template <typename T>
		inline T const &
		Max3(T const & a, T const & b, T const & c)
		{
			return std::max(std::max(a, b), c);
		}

		// 余数
		template <typename T>
		inline T
		Mod(T const & x, T const & y)
		{
			return x % y;
		}
		// 浮点版本
		template<>
		inline float
		Mod<float>(float const & x, float const & y)
		{
			return std::fmod(x, y);
		}
		template <>
		inline double
		Mod<double>(double const & x, double const & y)
		{
			return std::fmod(x, y);
		}

		// 限制 val 在 low 和 high 之间
		template <typename T>
		inline T const &
		Clamp(T const & val, T const & low, T const & high)
		{
			return std::max(low, std::min(high, val));
		}

		// 环绕处理
		template <typename T>
		inline T
		Wrap(T const & val, T const & low, T const & high)
		{
			T ret(val);
			T rang(high - low);

			while (ret >= high)
			{
				ret -= rang;
			}
			while (ret < low)
			{
				ret += rang;
			}

			return ret;
		}

		// 镜像处理
		template <typename T>
		inline T
		Mirror(T const & val, T const & low, T const & high)
		{
			T ret(val);
			T rang(high - low);

			while ((ret > high) || (ret < low))
			{
				if (ret > high)
				{
					ret = 2 * high - val;
				}
				else
				{
					if (ret < low)
					{
						ret = 2 * low - val;
					}
				}
			}

			return ret;
		}

		// 奇数则返回true
		template <typename T>
		inline bool
		IsOdd(T const & x)
		{
			return Mod(x, 2) != 0;
		}
		// 偶数则返回true
		template <typename T>
		inline bool
		IsEven(T const & x)
		{
			return !IsOdd(x);
		}

		// 判断 val 是否在 low 和 high 之间
		template <typename T>
		inline bool
		InBound(T const & val, T const & low, T const & high)
		{
			return ((val >= low) && (val <= high));
		}
		
		// 判断两个数是否相等
		template <typename T>
		inline bool
		Eq(T const & lhs, T const & rhs)
		{
			return (lhs == rhs);
		}
		// 浮点版本
		template <>
		inline bool
		Eq<float>(float const & lhs, float const & rhs)
		{
			return (Abs<float>(lhs - rhs)
				<= std::numeric_limits<float>::epsilon());
		}
		template <>
		inline bool
		Eq<double>(double const & lhs, double const & rhs)
		{
			return (Abs<double>(lhs - rhs)
				<= std::numeric_limits<double>::epsilon());
		}


		// 基本数学运算
		///////////////////////////////////////////////////////////////////////////////
		inline float
		Abs(float x)
		{
			return std::abs(x);
		}
		inline float
		Sqrt(float x)
		{
			return std::sqrt(x);
		}
		inline float
		RecipSqrt(float x)
		{
			return 1.0f / Sqrt(x);
		}

		inline float
		Pow(float x, float y)
		{
			return std::pow(x, y);
		}
		inline float
		Exp(float x)
		{
			return std::exp(x);
		}

		inline float
		Log(float x)
		{
			return std::log(x);
		}
		inline float
		Log10(float x)
		{
			return std::log10(x);
		}

		inline float
		Sin(float x)
		{
			return std::sin(x);
		}
		inline float
		Cos(float x)
		{
			return Sin(x + PI / 2);
		}
		inline void
		SinCos(float x, float& s, float& c)
		{
			s = Sin(x);
			c = Cos(x);
		}
		inline float
		Tan(float x)
		{
			return std::tan(x);
		}

		inline float
		ASin(float x)
		{
			return std::asin(x);
		}
		inline float
		ACos(float x)
		{
			return std::acos(x);
		}
		inline float
		ATan(float x)
		{
			return std::atan(x);
		}

		inline float
		Sinh(float x)
		{
			return std::sinh(x);
		}
		inline float
		Cosh(float x)
		{
			return std::cosh(x);
		}
		inline float
		Tanh(float x)
		{
			return std::tanh(x);
		}


		template <typename T, int N>
		struct DotHelper
		{
			static T Do(T const * lhs, T const * rhs)
			{
				return lhs[0] * rhs[0] + DotHelper<T, N - 1>::Do(lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct DotHelper<T, 1>
		{
			static T Do(T const * lhs, T const * rhs)
			{
				return lhs[0] * rhs[0];
			}
		};

		// 几种类型的Dot
		template <typename T>
		inline typename T::value_type
		Dot(T const & lhs, T const & rhs)
		{
			return DotHelper<T::value_type, T::elem_num>::Do(&lhs[0], &rhs[0]);
		}

		// 几种类型的LengthSq
		template <typename T>
		inline typename T::value_type
		LengthSq(T const & rhs)
		{
			return Dot(rhs, rhs);
		}

		// 几种类型的Length
		template <typename T>
		inline typename T::value_type
		Length(T const & rhs)
		{
			return Sqrt(LengthSq(rhs));
		}

		// 几种类型的Lerp
		template <typename T>
		inline T
		Lerp(T const & lhs, T const & rhs, float s)
		{
			return lhs + (rhs - lhs) * s;
		}

		template <typename T, int N>
		struct MaxMinimizeHelper
		{
			static void DoMax(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
				MaxMinimizeHelper<T, N - 1>::DoMax(out + 1, lhs + 1, rhs + 1);
			}

			static void DoMin(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
				MaxMinimizeHelper<T, N - 1>::DoMin(out + 1, lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct MaxMinimizeHelper<T, 1>
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
		inline Vector_T<T, N>
		Maximize(Vector_T<T, N> const & lhs, Vector_T<T, N> const & rhs)
		{
			Vector_T<T, N> ret;
			MaxMinimizeHelper<T, N>::DoMax(&ret[0], &lhs[0], &rhs[0]);
			return ret;
		}

		template <typename T, int N>
		inline Vector_T<T, N>
		Minimize(Vector_T<T, N> const & lhs, Vector_T<T, N> const & rhs)
		{
			Vector_T<T, N> ret;
			MaxMinimizeHelper<T, N>::DoMin(&ret[0], &lhs[0], &rhs[0]);
			return ret;
		}

		template <typename T, int N>
		struct TransformHelper
		{
			static Vector_T<T, 4> Do(Vector_T<T, N> const & v, Matrix4 const & mat);
		};
		template <typename T>
		struct TransformHelper<T, 4>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 4> const & v, Matrix4 const & mat)
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + v.w() * mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + v.w() * mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + v.w() * mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + v.w() * mat(3, 3));
			}
		};
		template <typename T>
		struct TransformHelper<T, 3>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 3> const & v, Matrix4 const & mat)
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + mat(3, 3));
			}
		};
		template <typename T>
		struct TransformHelper<T, 2>
		{
			static Vector_T<T, 4> Do(Vector_T<T, 2> const & v, Matrix4 const & mat)
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + mat(3, 3));
			}
		};

		template <typename T, int N>
		inline Vector_T<T, 4>
		Transform(Vector_T<T, N> const & v, Matrix4 const & mat)
		{
			return TransformHelper<T, N>::Do(v, mat);
		}

		template <typename T, int N>
		inline Vector_T<T, N>
		TransformCoord(Vector_T<T, N> const & v, Matrix4 const & mat)
		{
			BOOST_STATIC_ASSERT(N < 4);

			Vector_T<T, 4> temp(TransformHelper<T, N>::Do(v, mat));
			Vector_T<T, N> ret = Vector_T<T, N>(&temp[0]);
			if (Eq(temp.w(), T(0)))
			{
				ret = Vector_T<T, N>::Zero();
			}
			else
			{
				ret /= temp.w();
			}
			return ret;
		}


		template <typename T, int N>
		struct TransformNormalHelper
		{
			static Vector_T<T, N> Do(Vector_T<T, N> const & v, Matrix4 const & mat);
		};
		template <typename T>
		struct TransformNormalHelper<T, 3>
		{
			static Vector_T<T, 3> Do(Vector_T<T, 3> const & v, Matrix4 const & mat)
			{
				Vector_T<T, 4> temp(v.x(), v.y(), v.z(), T(0));
				temp = TransformHelper<T, 4>::Do(temp, mat);
				return Vector_T<T, 3>(temp.x(), temp.y(), temp.z());
			}
		};
		template <typename T>
		struct TransformNormalHelper<T, 2>
		{
			static Vector_T<T, 2> Do(Vector_T<T, 2> const & v, Matrix4 const & mat)
			{
				Vector_T<T, 3> temp(v.x(), v.y(), T(0));
				temp = TransformNormalHelper<T, 3>::Do(temp, mat);
				return Vector_T<T, 2>(temp.x(), temp.y());
			}
		};

		template <typename T, int N>
		inline Vector_T<T, N>
		TransformNormal(Vector_T<T, N> const & v, Matrix4 const & mat)
		{
			BOOST_STATIC_ASSERT(N < 4);

			return TransformNormalHelper<T, N>::Do(v, mat);
		}

		template <typename T, int N>
		inline Vector_T<T, N>
		BaryCentric(Vector_T<T, N> const & v1, Vector_T<T, N> const & v2, Vector_T<T, N> const & v3,
			T const & f, T const & g)
		{
			return (1 - f - g) * v1 + f * v2 + g * v3;
		}

		template <typename T>
		inline T
		Normalize(T const & rhs)
		{
			return rhs * RecipSqrt(LengthSq(rhs));
		}


		// 2D 向量
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline T
		Cross(Vector_T<T, 2> const & lhs, Vector_T<T, 2> const & rhs)
		{
			return lhs.x() * rhs.y() - lhs.y() * rhs.x();
		}


		// 3D 向量
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline T
		Angle(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return ACos(Dot(lhs, rhs) / (Length(lhs) * Length(rhs)));
		}

		template <typename T>
		inline Vector_T<T, 3>
		Cross(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return Vector_T<T, 3>(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
				lhs.z() * rhs.x() - lhs.x() * rhs.z(),
				lhs.x() * rhs.y() - lhs.y() * rhs.x());
		}

		template <typename T>
		inline Vector_T<T, 3>
		TransQuat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat)
		{
			// result = av + bq + c(q.v CROSS v)
			// where
			//  a = q.w()^2 - (q.v DOT q.v)
			//  b = 2 * (q.v DOT v)
			//  c = 2q.w()
			T const a(quat.w() * quat.w() - Dot(quat.v(), quat.v()));
			T const b(2 * Dot(quat.v(), v));
			T const c(quat.w() + quat.w());

			// Must store this, because result may alias v
			Vector_T<T, 3> cross(Cross(quat.v(), v));		// q.v CROSS v

			return Vector_T<T, 3>(a * v.x() + b * quat.x() + c * cross.x(),
				a * v.y() + b * quat.y() + c * cross.y(),
				a * v.z() + b * quat.z() + c * cross.z());
		}

		template <typename T>
		inline Vector_T<T, 3>
		Project(Vector_T<T, 3> const & vec,
			Matrix4_T<T> const & world, Matrix4_T<T> const & view, Matrix4_T<T> const & proj,
			int const viewport[4], T const & nearPlane, T const & farPlane)
		{
			Vector_T<T, 4> temp(Transform(objVec, world));
			temp = Transform(temp, view);
			temp = Transform(temp, proj);
			temp /= temp.w();

			Vector_T<T, 3> ret;
			ret.x() = (temp.x() + 1) * viewport[2] / 2 + viewport[0];
			ret.y() = (-temp.y() + 1) * viewport[3] / 2 + viewport[1];
			ret.z() = (temp.z() + 1) * (farPlane - nearPlane) / 2 + nearPlane;
			return ret;
		}

		template <typename T>
		inline Vector_T<T, 3>
		UnProject(Vector_T<T, 3> const & winVec, const T& clipW,
			Matrix4_T<T> const & world, Matrix4_T<T> const & view, Matrix4_T<T> const & proj,
			int const viewport[4], T const & nearPlane, T const & farPlane)
		{
			Vector_T<T, 4> temp;
			temp.x() = 2 * (winVec.x() - viewport[0]) / viewport[2] - 1;
			temp.y() = -(2 * (winVec.y() - viewport[1]) / viewport[3] - 1);
			temp.z() = 2 * (winVec.z() - nearPlane) / (farPlane - nearPlane) - 1;
			temp.w() = clipW;

			Matrix4_T<T> mat(Inverse(world * view * proj));
			temp = Transform(temp, mat);

			return Vector_T<T, 3>(temp.x(), temp.y(), temp.z()) / temp.w();
		}


		// 4D 向量
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline Vector_T<T, 4>
		Cross(Vector_T<T, 4> const & v1, Vector_T<T, 4> const & v2, Vector_T<T, 4> const & v3)
		{
			T const A = (v2.x() * v3.y()) - (v2.y() * v3.x());
			T const B = (v2.x() * v3.z()) - (v2.z() * v3.x());
			T const C = (v2.x() * v3.w()) - (v2.w() * v3.x());
			T const D = (v2.y() * v3.z()) - (v2.z() * v3.y());
			T const E = (v2.y() * v3.w()) - (v2.w() * v3.y());
			T const F = (v2.z() * v3.w()) - (v2.w() * v3.z());

			return Vector_T<T, 4>((v1.y() * F) - (v1.z() * E) + (v1.w() * D),
				-(v1.x() * F) + (v1.z() * C) - (v1.w() * B),
				(v1.x() * E) - (v1.y() * C) + (v1.w() * A),
				-(v1.x() * D) + (v1.y() * B) - (v1.z() * A));
		}


		// 4D 矩阵
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline Matrix4_T<T>
		Multiply(Matrix4_T<T> const & lhs, Matrix4_T<T> const & rhs)
		{
			return Matrix4_T<T>(
				lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0) + lhs(0, 3) * rhs(3, 0),
				lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1) + lhs(0, 3) * rhs(3, 1),
				lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2) + lhs(0, 3) * rhs(3, 2),
				lhs(0, 0) * rhs(0, 3) + lhs(0, 1) * rhs(1, 3) + lhs(0, 2) * rhs(2, 3) + lhs(0, 3) * rhs(3, 3),

				lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0) + lhs(1, 3) * rhs(3, 0),
				lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1) + lhs(1, 3) * rhs(3, 1),
				lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2) + lhs(1, 3) * rhs(3, 2),
				lhs(1, 0) * rhs(0, 3) + lhs(1, 1) * rhs(1, 3) + lhs(1, 2) * rhs(2, 3) + lhs(1, 3) * rhs(3, 3),

				lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0) + lhs(2, 3) * rhs(3, 0),
				lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1) + lhs(2, 3) * rhs(3, 1),
				lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2) + lhs(2, 3) * rhs(3, 2),
				lhs(2, 0) * rhs(0, 3) + lhs(2, 1) * rhs(1, 3) + lhs(2, 2) * rhs(2, 3) + lhs(2, 3) * rhs(3, 3),

				lhs(3, 0) * rhs(0, 0) + lhs(3, 1) * rhs(1, 0) + lhs(3, 2) * rhs(2, 0) + lhs(3, 3) * rhs(3, 0),
				lhs(3, 0) * rhs(0, 1) + lhs(3, 1) * rhs(1, 1) + lhs(3, 2) * rhs(2, 1) + lhs(3, 3) * rhs(3, 1),
				lhs(3, 0) * rhs(0, 2) + lhs(3, 1) * rhs(1, 2) + lhs(3, 2) * rhs(2, 2) + lhs(3, 3) * rhs(3, 2),
				lhs(3, 0) * rhs(0, 3) + lhs(3, 1) * rhs(1, 3) + lhs(3, 2) * rhs(2, 3) + lhs(3, 3) * rhs(3, 3));
		}

		template <typename T>
		inline T
		Determinant(Matrix4_T<T> const & rhs)
		{
			T const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
			T const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
			T const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
			T const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
			T const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
			T const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

			return rhs(0, 0) * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342)
				- rhs(0, 1) * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341)
				+ rhs(0, 2) * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241)
				- rhs(0, 3) * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241);
		}

		template <typename T>
		inline Matrix4_T<T>
		Inverse(Matrix4_T<T> const & rhs)
		{
			T const _2132_2231(rhs(1, 0) * rhs(2, 1) - rhs(1, 1) * rhs(2, 0));
			T const _2133_2331(rhs(1, 0) * rhs(2, 2) - rhs(1, 2) * rhs(2, 0));
			T const _2134_2431(rhs(1, 0) * rhs(2, 3) - rhs(1, 3) * rhs(2, 0));
			T const _2142_2241(rhs(1, 0) * rhs(3, 1) - rhs(1, 1) * rhs(3, 0));
			T const _2143_2341(rhs(1, 0) * rhs(3, 2) - rhs(1, 2) * rhs(3, 0));
			T const _2144_2441(rhs(1, 0) * rhs(3, 3) - rhs(1, 3) * rhs(3, 0));
			T const _2233_2332(rhs(1, 1) * rhs(2, 2) - rhs(1, 2) * rhs(2, 1));
			T const _2234_2432(rhs(1, 1) * rhs(2, 3) - rhs(1, 3) * rhs(2, 1));
			T const _2243_2342(rhs(1, 1) * rhs(3, 2) - rhs(1, 2) * rhs(3, 1));
			T const _2244_2442(rhs(1, 1) * rhs(3, 3) - rhs(1, 3) * rhs(3, 1));
			T const _2334_2433(rhs(1, 2) * rhs(2, 3) - rhs(1, 3) * rhs(2, 2));
			T const _2344_2443(rhs(1, 2) * rhs(3, 3) - rhs(1, 3) * rhs(3, 2));
			T const _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
			T const _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
			T const _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
			T const _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
			T const _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
			T const _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

			// 行列式的值
			T const det(Determinant(rhs));
			if (!Eq<T>(det, 0))
			{
				T invDet(T(1) / det);

				return Matrix4(
					+invDet * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342),
					-invDet * (rhs(0, 1) * _3344_3443 - rhs(0, 2) * _3244_3442 + rhs(0, 3) * _3243_3342),
					+invDet * (rhs(0, 1) * _2344_2443 - rhs(0, 2) * _2244_2442 + rhs(0, 3) * _2243_2342),
					-invDet * (rhs(0, 1) * _2334_2433 - rhs(0, 2) * _2234_2432 + rhs(0, 3) * _2233_2332),

					-invDet * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341),
					+invDet * (rhs(0, 0) * _3344_3443 - rhs(0, 2) * _3144_3441 + rhs(0, 3) * _3143_3341),
					-invDet * (rhs(0, 0) * _2344_2443 - rhs(0, 2) * _2144_2441 + rhs(0, 3) * _2143_2341),
					+invDet * (rhs(0, 0) * _2334_2433 - rhs(0, 2) * _2134_2431 + rhs(0, 3) * _2133_2331),

					+invDet * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241),
					-invDet * (rhs(0, 0) * _3244_3442 - rhs(0, 1) * _3144_3441 + rhs(0, 3) * _3142_3241),
					+invDet * (rhs(0, 0) * _2244_2442 - rhs(0, 1) * _2144_2441 + rhs(0, 3) * _2142_2241),
					-invDet * (rhs(0, 0) * _2234_2432 - rhs(0, 1) * _2134_2431 + rhs(0, 3) * _2132_2231),

					-invDet * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241),
					+invDet * (rhs(0, 0) * _3243_3342 - rhs(0, 1) * _3143_3341 + rhs(0, 2) * _3142_3241),
					-invDet * (rhs(0, 0) * _2243_2342 - rhs(0, 1) * _2143_2341 + rhs(0, 2) * _2142_2241),
					+invDet * (rhs(0, 0) * _2233_2332 - rhs(0, 1) * _2133_2331 + rhs(0, 2) * _2132_2231));
			}
			else
			{
				return rhs;
			}
		}

		template <typename T>
		inline Matrix4_T<T>
		LookAtLH(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt,
			Vector_T<T, 3> const & vUp = Vector_T<T, 3>(0, 1, 0))
		{
			Vector_T<T, 3> zAxis(Normalize(vAt - vEye));
			Vector_T<T, 3> xAxis(Normalize(Cross(vUp, zAxis)));
			Vector_T<T, 3> yAxis(Cross(zAxis, xAxis));

			return Matrix4_T<T>(
				xAxis.x(),			yAxis.x(),			zAxis.x(),			0,
				xAxis.y(),			yAxis.y(),			zAxis.y(),			0,
				xAxis.z(),			yAxis.z(),			zAxis.z(),			0,
				-Dot(xAxis, vEye),	-Dot(yAxis, vEye),	-Dot(zAxis, vEye),	1);
		}

		template <typename T>
		inline Matrix4_T<T>
		LookAtRH(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt,
			Vector_T<T, 3> const & vUp = Vector_T<T, 3>(0, 1, 0))
		{
			Vector_T<T, 3> zAxis(Normalize(vEye - vAt));
			Vector_T<T, 3> xAxis(Normalize(Cross(vUp, zAxis)));
			Vector_T<T, 3> yAxis(Cross(zAxis, xAxis));

			return Matrix4_T<T>(
				xAxis.x(),			yAxis.x(),			zAxis.x(),			0,
				xAxis.y(),			yAxis.y(),			zAxis.y(),			0,
				xAxis.z(),			yAxis.z(),			zAxis.z(),			0,
				-Dot(xAxis, vEye),	-Dot(yAxis, vEye),	-Dot(zAxis, vEye),	1);
		}

		template <typename T>
		inline Matrix4_T<T>
		OrthoLH(T const & w, T const & h, T const & nearPlane, T const & farPlane)
		{
			T const w_2(w / 2);
			T const h_2(h / 2);
			return OrthoOffCenterLH(-w_2, w_2, -h_2, h_2, nearPlane, farPlane);
		}
		template <typename T>
		inline Matrix4_T<T>
		OrthoOffCenterLH(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane)
		{
			T const q(T(1) / (farPlane - nearPlane));
			T const invWidth(T(1) / (right - left));
			T const invHeight(T(1) / (top - bottom));

			return Matrix4_T<T>(
				invWidth + invWidth,		0,								0,					0,
				0,							invHeight + invHeight,			0,					0,
				0,							0,								q,					0,
				-(left + right) * invWidth,	-(top + bottom) * invHeight,	-nearPlane * q,		1);
		}

		template <typename T>
		inline Matrix4_T<T>
		PerspectiveLH(T const & width, T const & height, T const & nearPlane, T const & farPlane)
		{
			T const q(farPlane / (farPlane - nearPlane));
			T const near2(nearPlane + nearPlane);

			return Matrix4_T<T>(
				near2 / width,	0,				0,				0,
				0,				near2 / height,	0,				0,
				0,				0,				q,				1,
				0,				0,				-nearPlane * q, 0);
		}
		template <typename T>
		inline Matrix4_T<T>
		PerspectiveFovLH(T const & fov, T const & aspect, T const & nearPlane, T const & farPlane)
		{
			T const h(T(1) / Tan(fov / 2));
			T const w(h / aspect);
			T const q(farPlane / (farPlane - nearPlane));

			return Matrix4_T<T>(
				w,		0,		0,				0,
				0,		h,		0,				0,
				0,		0,		q,				1,
				0,		0,		-nearPlane * q, 0);
		}
		template <typename T>
		inline Matrix4_T<T>
		PerspectiveOffCenterLH(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane)
		{
			T const q(farPlane / (farPlane - nearPlane));
			T const near2(nearPlane + nearPlane);
			T const invWidth(T(1) / (right - left));
			T const invHeight(T(1) / (top - bottom));

			return Matrix4_T<T>(
				near2 * invWidth,			0,								0,				0,
				0,							near2 * invHeight,				0,				0,
				-(left + right)* invWidth,	-(top + bottom) * invHeight,	q,				1,
				0,							0,								-nearPlane * q, 0);
		}

		template <typename T>
		inline Matrix4_T<T>
		Reflect(Plane_T<T> const & p)
		{
			Plane_T<T> P(Normalize(p));
			T const aa2(-2 * P.a() * P.a()), ab2(-2 * P.a() * P.b()), ac2(-2 * P.a() * P.c()), ad2(-2 * P.a() * P.d());
			T const bb2(-2 * P.b() * P.b()), bc2(-2 * P.b() * P.c()), bd2(-2 * P.a() * P.c());
			T const cc2(-2 * P.c() * P.c()), cd2(-2 * P.c() * P.d());

			return Matrix4_T<T>(
				aa2 + 1,	ab2,		ac2,		0,
				ab2,		bb2 + 1,	bc2,		0,
				ac2,		bc2,		cc2 + 1,	0,
				ad2,		bd2,		cd2,		1);
		}

		template <typename T>
		inline Matrix4_T<T>
		RotationX(T const & x)
		{
			float sx, cx;
			SinCos(x, sx, cx);

			return Matrix4_T<T>(
				1,	0,		0,		0,
				0,	cx,		sx,		0,
				0,	-sx,	cx,		0,
				0,	0,		0,		1);
		}
		template <typename T>
		inline Matrix4_T<T>
		RotationY(T const & y)
		{
			float sy, cy;
			SinCos(y, sy, cy);

			return Matrix4_T<T>(
				cy,		0,		-sy,	0,
				0,		1,		0,		0,
				sy,		0,		cy,		0,
				0,		0,		0,		1);
		}
		template <typename T>
		inline Matrix4_T<T>
		RotationZ(T const & z)
		{
			float sz, cz;
			SinCos(z, sz, cz);

			return Matrix4_T<T>(
				cz,		sz,		0,		0,
				-sz,	cz,		0,		0,
				0,		0,		1,		0,
				0,		0,		0,		1);
		}
		template <typename T>
		inline Matrix4_T<T>
		Rotation(T const & angle, T const & x, T const & y, T const & z)
		{
			Quaternion_T<T> quat(RotationAxis(Vector_T<T, 3>(x, y, z), angle));
			return ToMatrix(quat);
		}
		template <typename T>
		inline Matrix4_T<T>
		RotationMatrixYawPitchRoll(T const & yaw, T const & pitch, T const & roll)
		{
			Matrix4 rotX(RotationX(pitch));
			Matrix4 rotY(RotationY(yaw));
			Matrix4 rotZ(RotationZ(roll));
			return rotZ * rotX * rotY;
		}

		template <typename T>
		inline Matrix4_T<T>
		Scaling(T const & sx, T const & sy, T const & sz)
		{
			return Matrix4(
				sx,	0,	0,	0,
				0,	sy,	0,	0,
				0,	0,	sz,	0,
				0,	0,	0,	1);
		}

		template <typename T>
		inline Matrix4_T<T>
		Shadow(Vector_T<T, 4> const & v, Plane_T<T> const & p)
		{
			Vector_T<T, 4> const v(-L);
			Plane_T<T> P(Normalize(p));
			T const d(-Dot(P, v));

			return Matrix4_T<T>(
				P.a() * v.x() + d,	P.a() * v.y(),		P.a() * v.z(),		P.a() * v.w(),
				P.b() * v.x(),		P.b() * v.y() + d,	P.b() * v.z(),		P.b() * v.w(),
				P.c() * v.x(),		P.c() * v.y(),		P.c() * v.z() + d,	P.c() * v.w(),
				P.d() * v.x(),		P.d() * v.y(),		P.d() * v.z(),		P.d() * v.w() + d);
		}

		template <typename T>
		inline Matrix4_T<T>
		ToMatrix(Quaternion_T<T> const & quat)
		{
			// calculate coefficients
			T const x2(quat.x() + quat.x());
			T const y2(quat.y() + quat.y());
			T const z2(quat.z() + quat.z());
			  
			T const xx2(quat.x() * x2), xy2(quat.x() * y2), xz2(quat.x() * z2);
			T const yy2(quat.y() * y2), yz2(quat.y() * z2), zz2(quat.z() * z2);
			T const wx2(quat.w() * x2), wy2(quat.w() * y2), wz2(quat.w() * z2);

			return Matrix4_T<T>(
				1 - yy2 - zz2,	xy2 + wz2,		xz2 - wy2,		0,
				xy2 - wz2,		1 - xx2 - zz2,	yz2 + wx2,		0,
				xz2 + wy2,		yz2 - wx2,		1 - xx2 - yy2,	0,
				0,				0,				0,				1);
		}

		template <typename T>
		inline Matrix4_T<T>
		Translation(T const & x, T const & y, T const & z)
		{
			return Matrix4_T<T>(
				1,	0,	0,	0,
				0,	1,	0,	0,
				0,	0,	1,	0,
				x,	y,	z,	1);
		}

		template <typename T>
		inline Matrix4_T<T>
		Transpose(Matrix4_T<T> const & rhs)
		{
			return Matrix4_T<T>(
				rhs(0, 0), rhs(1, 0), rhs(2, 0), rhs(3, 0),
				rhs(0, 1), rhs(1, 1), rhs(2, 1), rhs(3, 1),
				rhs(0, 2), rhs(1, 2), rhs(2, 2), rhs(3, 2),
				rhs(0, 3), rhs(1, 3), rhs(2, 3), rhs(3, 3));
		}

		template <typename T>
		inline Matrix4_T<T>
		LHToRH(Matrix4_T<T> const & rhs)
		{
			Matrix4_T<T> ret = rhs;
			ret(2, 0) = -ret(2, 0);
			ret(2, 1) = -ret(2, 1);
			ret(2, 2) = -ret(2, 2);
			ret(2, 3) = -ret(2, 3);
			return ret;
		}

		template <typename T>
		inline Matrix4_T<T>
		Scaling(Vector_T<T, 3> const & vPos)
		{
			return Scaling(vPos.x(), vPos.y(), vPos.z());
		}
		template <typename T>
		inline Matrix4_T<T>
		Translation(Vector_T<T, 3> const & vPos)
		{
			return Translation(vPos.x(), vPos.y(), vPos.z());
		}
		template <typename T>
		inline Matrix4_T<T>
		OrthoRH(T const & width, T const & height, T const & nearPlane, T const & farPlane)
		{
			return LHToRH(OrthoLH(w, h, nearPlane, farPlane));
		}
		template <typename T>
		inline Matrix4_T<T>
		OrthoOffCenterRH(T const & left, T const & right, T const & bottom, T const & top, 
			T const & nearPlane, T const & farPlane)
		{
			return LHToRH(OrthoOffCenterLH(left, right, bottom, top, nearPlane, farPlane));
		}
		template <typename T>
		inline Matrix4_T<T>
		PerspectiveRH(T const & width, T const & height,
			T const & nearPlane, T const & farPlane)
		{
			return LHToRH(PerspectiveLH(w, h, nearPlane, farPlane));
		}
		template <typename T>
		inline Matrix4_T<T>
		PerspectiveFovRH(T const & fov, T const & aspect,
			T const & nearPlane, T const & farPlane)
		{
			return LHToRH(PerspectiveFovLH(fov, aspect, nearPlane, farPlane));
		}
		template <typename T>
		inline Matrix4_T<T>
		PerspectiveOffCenterRH(T const & left, T const & right, T const & bottom, T const & top, 
			T const & nearPlane, T const & farPlane)
		{
			return LHToRH(PerspectiveOffCenterLH(left, right, bottom, top, nearPlane, farPlane));
		}

		template <typename T>
		inline Matrix4_T<T>
		RHToLH(Matrix4_T<T> const & rhs)
		{
			return LHToRH(rhs);
		}

		template <typename T>
		inline Matrix4_T<T>
		RotationMatrixYawPitchRoll(Vector_T<T, 3> const & ang)
		{
			return RotationMatrixYawPitchRoll(ang.x(), ang.y(), ang.z());
		}


		// 四元数
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline Quaternion_T<T>
		Conjugate(Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
		}

		template <typename T>
		inline Quaternion_T<T>
		AxisToAxis(Vector_T<T, 3> const & from, Vector_T<T, 3> const & to)
		{
			Vector_T<T, 3> a(Normalize(from));
			Vector_T<T, 3> b(Normalize(to));

			return UnitAxisToUnitAxis(a, b);
		}
		template <typename T>
		inline Quaternion_T<T>
		UnitAxisToUnitAxis(Vector_T<T, 3> const & from, Vector_T<T, 3> const & to)
		{
			T dot = Abs(Dot(from, to));
			if (Eq(dot, T(1)))
			{
				return Quaternion_T<T>::Identity();
			}
			else
			{
				if (Eq(dot, T(-1)))
				{
					return Quaternion_T<T>(1, 0, 0, 0);
				}
				else
				{
					Vector_T<T, 3> axis = Cross(from, to);

					T const cos_theta = Dot(from, to);
					T const sin_theta = Sqrt(1 - cos_theta * cos_theta);
					T const sin_half_theta = Sqrt((1 - cos_theta) / 2);
					T const cos_half_theta = sin_theta / (2 * sin_half_theta);

					return Quaternion_T<T>(axis * sin_half_theta, cos_half_theta);
				}
			}
		}

		template <typename T>
		inline Quaternion_T<T>
		BaryCentric(Quaternion_T<T> const & q1, Quaternion_T<T> const & q2,
			Quaternion_T<T> const & q3, T const & f, T const & g)
		{
			T const temp(f + g);
			Quaternion_T<T> qT1(Slerp(q1, q2, temp));
			Quaternion_T<T> qT2(Slerp(q1, q3, temp));

			return Slerp(qT1, qT2, g / temp);
		}

		template <typename T>
		inline Quaternion_T<T>
		Exp(Quaternion_T<T> const & rhs)
		{
			T const theta(Length(rhs.v()));
			return Quaternion_T<T>(Normalize(rhs.v()) * Sin(theta), Cos(theta));
		}
		template <typename T>
		inline Quaternion_T<T>
		Ln(Quaternion_T<T> const & rhs)
		{
			T const theta_2(ACos(rhs.w()));
			return Quaternion_T<T>(Normalize(rhs.v() * (theta_2 + theta_2)), 0);
		}

		template <typename T>
		inline Quaternion_T<T>
		Inverse(Quaternion_T<T> const & rhs)
		{
			T const inv(T(1) / Length(rhs));
			return Quaternion(-rhs.x() * inv, -rhs.y() * inv, -rhs.z() * inv, rhs.w() * inv);
		}

		template <typename T>
		inline Quaternion_T<T>
		Multiply(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(
				lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
				lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
				lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
				lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
		}

		template <typename T>
		inline Quaternion_T<T>
		RotationQuatYawPitchRoll(T const & yaw, T const & pitch, T const & roll)
		{
			T const angX(pitch / 2), angY(yaw / 2), angZ(roll / 2);
			T sx, sy, sz;
			T cx, cy, cz;
			SinCos(angX, sx, cx);
			SinCos(angY, sy, cy);
			SinCos(angZ, sz, cz);

			return Quaternion_T<T>(
				sx * cy * cz + cx * sy * sz,
				cx * sy * cz - sx * cy * sz,
				cx * cy * sz - sx * sy * cz,
				sx * sy * sz + cx * cy * cz);
		}

		template <typename T>
		inline void
		ToAxisAngle(Vector_T<T, 3>& vec, T& ang, Quaternion_T<T> const & quat)
		{
			T const tw(ACos(quat.w()));
			T const scale(T(1) / Sin(tw));

			ang = tw + tw;
			vec.x() = quat.x() * scale;
			vec.y() = quat.y() * scale;
			vec.z() = quat.z() * scale;
		}

		template <typename T>
		inline Quaternion_T<T>
		ToQuaternion(Matrix4_T<T> const & mat)
		{
			Quaternion_T<T> quat;
			T s;
			T const tr(mat(0, 0) + mat(1, 1) + mat(2, 2));

			// check the diagonal
			if (tr > 0)
			{
				s = Sqrt(tr + 1);
				quat.w() = s * T(0.5);
				s = T(0.5) / s;
				quat.x() = (mat(1, 2) - mat(2, 1)) * s;
				quat.y() = (mat(2, 0) - mat(0, 2)) * s;
				quat.z() = (mat(0, 1) - mat(1, 0)) * s;
			}
			else
			{
				if ((mat(1, 1) > mat(0, 0)) && (mat(2, 2) <= mat(1, 1)))
				{
					s = Sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);

					quat.y() = s * T(0.5);

					if (!Eq<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(2, 0) - mat(0, 2)) * s;
					quat.z() = (mat(2, 1) + mat(1, 2)) * s;
					quat.x() = (mat(0, 1) + mat(1, 0)) * s;
				}
				else
				{
					if ((mat(1, 1) <= mat(0, 0) && mat(2, 2) > mat(0, 0)) || (mat(2, 2) > mat(1, 1)))
					{
						s = Sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

						quat.z() = s * T(0.5);

						if (!Eq<T>(s, 0))
						{
							s = T(0.5) / s;
						}

						quat.w() = (mat(0, 1) - mat(1, 0)) * s;
						quat.x() = (mat(0, 2) + mat(2, 0)) * s;
						quat.y() = (mat(1, 2) + mat(2, 1)) * s;
					}
					else
					{
						s = Sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

						quat.x() = s * T(0.5);

						if (!Eq<T>(s, 0))
						{
							s = T(0.5) / s;
						}

						quat.w() = (mat(1, 2) - mat(2, 1)) * s;
						quat.y() = (mat(1, 0) + mat(0, 1)) * s;
						quat.z() = (mat(2, 0) + mat(0, 2)) * s;
					}
				}
			}

			return Normalize(quat);
		}

		template <typename T>
		inline Quaternion_T<T>
		RotationAxis(Vector_T<T, 3> const & v, T const & angle)
		{
			T const ang(angle * T(0.5));
			T sa, ca;
			SinCos(ang, sa, ca);

			return Quaternion_T<T>(sa * Normalize(v), ca);
		}

		template <typename T>
		inline Quaternion_T<T>
		Slerp(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs, T const & slerp)
		{
			T scale0, scale1;
			Quaternion_T<T> q2;

			// DOT the quats to get the cosine of the angle between them
			T const cosom(Dot(lhs, rhs));

			// Two special cases:
			// Quats are exactly opposite, within DELTA?
			if (cosom > std::numeric_limits<T>::epsilon() - T(1))
			{
				// make sure they are different enough to avoid a divide by 0
				if (cosom < T(1) - std::numeric_limits<T>::epsilon())
				{
					// SLERP away
					T const omega(ACos(cosom));
					T const isinom(T(1) / Sin(omega));
					scale0 = Sin((T(1) - slerp) * omega) * isinom;
					scale1 = Sin(slerp * omega) * isinom;
				}
				else
				{
					// LERP is good enough at this distance
					scale0 = T(1)- slerp;
					scale1 = slerp;
				}

				q2 = rhs * scale1;
			}
			else
			{
				// SLERP towards a perpendicular quat
				// Set slerp parameters
				scale0 = Sin((T(1) - slerp) * PIdiv2);
				scale1 = Sin(slerp * PIdiv2);

				q2.x() = -rhs.y() * scale1;
				q2.y() = +rhs.x() * scale1;
				q2.z() = -rhs.w() * scale1;
				q2.w() = +rhs.z() * scale1;
			}

			// Compute the result
			return scale0 * lhs + q2;
		}

		template <typename T>
		inline Quaternion_T<T>
		RotationQuatYawPitchRoll(Vector_T<T, 3> const & ang)
		{
			return RotationQuatYawPitchRoll(ang.x(), ang.y(), ang.z());
		}


		// 平面
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline T
		Dot(Plane_T<T> const & lhs, Vector_T<T, 4> const & rhs)
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d() * rhs.w();
		}
		template <typename T>
		inline T
		DotCoord(Plane_T<T> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d();
		}
		template <typename T>
		inline T
		DotNormal(Plane_T<T> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z();
		}

		template <typename T>
		inline Plane_T<T>
		Normalize(Plane_T<T> const & rhs)
		{
			T const inv(T(1) / Length(rhs));
			return Plane_T<T>(rhs.a() * inv, rhs.b() * inv, rhs.c() * inv, rhs.d() * inv);
		}
		template <typename T>
		inline Plane_T<T>
		FromPointNormal(Vector_T<T, 3> const & point, Vector_T<T, 3> const & normal)
		{
			return Plane(normal.x(), normal.y(), normal.z(), -Dot(point, normal));
		}
		template <typename T>
		inline Plane_T<T>
		FromPoints(Vector_T<T, 3> const & v0, Vector_T<T, 3> const & v1, Vector_T<T, 3> const & v2)
		{
			Vector_T<T, 3> vec(Cross(v1 - v0, v2 - v0));
			return FromPointNormal(v0, Normalize(vec));
		}
		template <typename T>
		inline Plane_T<T>
		Transform(Plane_T<T> const & p, Matrix4_T<T> const & mat)
		{
			return Plane_T<T>(
				p.a() * mat(0, 0) + p.b() * mat(1, 0) + p.c() * mat(2, 0) + p.d() * mat(3, 0),
				p.a() * mat(0, 1) + p.b() * mat(1, 1) + p.c() * mat(2, 1) + p.d() * mat(3, 1),
				p.a() * mat(0, 2) + p.b() * mat(1, 2) + p.c() * mat(2, 2) + p.d() * mat(3, 2),
				p.a() * mat(0, 3) + p.b() * mat(1, 3) + p.c() * mat(2, 3) + p.d() * mat(3, 3));
		}
		
		// 求直线和平面的交点，直线orig + t * dir
		template <typename T>
		inline T
		IntersectLine(Plane_T<T> const & p,
			Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir)
		{
			T deno(Dot(dir, p.Normal()));
			if (Eq(deno, T(0)))
			{
				deno = T(0.0001);
			}

			return -DotCoord(p, orig) / deno;
		}


		// 颜色
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline Color_T<T>
		Negative(Color_T<T> const & rhs)
		{
			return Color_T<T>(1 - rhs.r(), 1 - rhs.g(), 1 - rhs.b(), rhs.a());
		}
		template <typename T>
		inline Color_T<T>
		Modulate(Color_T<T> const & lhs, Color_T<T> const & rhs)
		{
			return Color_T<T>(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
		}


		// 范围
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		inline bool
		VecInSphere(Sphere_T<T> const & sphere, Vector_T<T, 3> const & v)
		{
			if (Length(v - sphere.Center()) < sphere.Radius())
			{
				return true;
			}
			return false;
		}

		template <typename T>
		inline bool
		BoundProbe(Sphere_T<T> const & sphere, Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir)
		{
			T const a = LengthSq(dir);
			T const b = 2 * Dot(dir, orig - sphere.Center());
			T const c = LengthSq(orig - sphere.Center()) - sphere.Radius() * sphere.Radius();

			if (b * b - 4 * a * c < 0)
			{
				return false;
			}
			return true;
		}

		template <typename T>
		inline bool
		VecInBox(Box_T<T> const & box, Vector_T<T, 3> const & v)
		{
			return (InBound(v.x(), box.Min().x(), box.Max().x()))
				&& (InBound(v.y(), box.Min().y(), box.Max().y()))
				&& (InBound(v.z(), box.Min().z(), box.Max().z()));
		}

		template <typename T>
		inline bool
		BoundProbe(Box_T<T> const & box, Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir)
		{
			Vector_T<T, 3> const leftBottomNear(box.LeftBottomNear());
			Vector_T<T, 3> const leftTopNear(box.LeftTopNear());
			Vector_T<T, 3> const rightTopNear(box.RightTopNear());
			Vector_T<T, 3> const leftTopFar(box.LeftTopFar());

			Plane_T<T> pNear(FromPoints(leftBottomNear, leftTopNear, rightTopNear));
			Plane_T<T> pTop(FromPoints(leftTopNear, leftTopFar, rightTopNear));
			Plane_T<T> pLeft(FromPoints(leftTopFar, leftTopNear, leftBottomNear));

			T t = IntersectLine(pNear, orig, dir);
			if (t >= 0)
			{
				Vector_T<T, 3> vec = orig + t * dir;
				if (!(InBound(vec.x(), leftBottomNear.x(), rightTopNear.x())
					&& InBound(vec.y(), leftBottomNear.y(), leftTopNear.y())))
				{
					return false;
				}
			}

			t = IntersectLine(pTop, orig, dir);
			if (t >= 0)
			{
				Vector_T<T, 3> vec = orig + t * dir;
				if (!(InBound(vec.x(), leftTopNear.x(), rightTopNear.x())
					&& InBound(vec.z(), leftTopNear.z(), leftTopFar.z())))
				{
					return false;
				}
			}

			t = IntersectLine(pLeft, orig, dir);
			if (t >= 0)
			{
				Vector_T<T, 3> vec = orig + t * dir;
				if (!(InBound(vec.y(), leftBottomNear.y(), leftTopNear.y())
					&& InBound(vec.z(), leftBottomNear.z(), leftTopFar.z())))
				{
					return false;
				}
			}

			return true;
		}

		// from Graphics Gems I p301
		template <typename value_type, typename Iterator>
		inline Box_T<value_type>
		ComputeBoundingBox(Iterator first, Iterator last)
		{
			Vector_T<value_type, 3> minVec = *first;
			Vector_T<value_type, 3> maxVec = *first;
			Iterator iter = first;
			++ iter;
			for (; iter != last; ++ iter)
			{
				Vector_T<value_type, 3> const & v = *iter;
				minVec = Minimize(minVec, v);
				maxVec = Maximize(maxVec, v);
			}
			return Box_T<value_type>(minVec, maxVec);
		}

		template <typename value_type, typename Iterator>
		inline Sphere_T<value_type>
		ComputeBoundingSphere(Iterator first, Iterator last)
		{
			value_type const min_float = std::numeric_limits<value_type>::min();
			value_type const max_float = std::numeric_limits<value_type>::max();
			Vector_T<value_type, 3> x_min(max_float, max_float, max_float);
			Vector_T<value_type, 3> y_min(max_float, max_float, max_float);
			Vector_T<value_type, 3> z_min(max_float, max_float, max_float);
			Vector_T<value_type, 3> x_max(min_float, min_float, min_float);
			Vector_T<value_type, 3> y_max(min_float, min_float, min_float);
			Vector_T<value_type, 3> z_max(min_float, min_float, min_float);
			for (Iterator iter = first; iter != last; ++ iter)
			{
				if (x_min.x() > iter->x())
				{	
					x_min = *iter;
				}	
				if (y_min.y() > iter->y())
				{	
					y_min = *iter;
				}	
				if (z_min.z() > iter->z())
				{	
					z_min = *iter;
				}

				if (x_max.x() < iter->x())
				{	
					x_max = *iter;
				}	
				if (y_max.y() < iter->y())
				{	
					y_max = *iter;
				}	
				if (z_max.z() < iter->z())
				{	
					z_max = *iter;
				}
			}

			value_type x_span = LengthSq(x_max - x_min);
			value_type y_span = LengthSq(y_max - y_min);
			value_type z_span = LengthSq(z_max - z_min);

			Vector_T<value_type, 3> dia1 = x_min;
			Vector_T<value_type, 3> dia2 = x_max;
			T max_span = x_span;
			if (y_span > max_span)
			{
				max_span = y_span;
				dia1 = y_min;
				dia2 = y_max;
			}
			if (z_span > max_span)
			{
				max_span = z_span;
				dia1 = z_min;
				dia2 = z_max;
			}

			Vector_T<value_type, 3> center((dia1 + dia2) / 2);
			value_type r = Length(dia2 - center);

			for (Iterator iter = first; iter != last; ++ iter)
			{
				value_type d = Length(*iter - center);

				if (d > r)
				{
					r = (d + r) / 2;
					center = (r * center + (d - r) * (*iter)) / d;
				}
			}

			return Sphere_T<value_type>(center, r);
		}


		// 网格
		///////////////////////////////////////////////////////////////////////////////

		// 计算TBN基
		template <typename T, typename TangentIterator, typename BinormIterator,
			typename IndexIterator, typename PositionIterator, typename TexCoordIterator>
		inline void
		ComputeTangent(TangentIterator targentBegin, BinormIterator binormBegin,
								IndexIterator indicesBegin, IndexIterator indicesEnd,
								PositionIterator xyzsBegin, PositionIterator xyzsEnd,
								TexCoordIterator texsBegin)
		{
			typedef typename std::iterator_traits<PositionIterator>::value_type position_type;
			typedef typename std::iterator_traits<TexCoordIterator>::value_type texcoord_type;

			int const num = static_cast<int>(std::distance(xyzsBegin, xyzsEnd));

			for (int i = 0; i < num; ++ i)
			{
				*(targentBegin + i) = Vector3::Zero();
				*(binormBegin + i) = Vector3::Zero();
			}

			for (IndexIterator iter = indicesBegin; iter != indicesEnd; iter += 3)
			{
				uint16_t const v0Index = *(iter + 0);
				uint16_t const v1Index = *(iter + 1);
				uint16_t const v2Index = *(iter + 2);

				position_type const & v0XYZ(*(xyzsBegin + v0Index));
				position_type const & v1XYZ(*(xyzsBegin + v1Index));
				position_type const & v2XYZ(*(xyzsBegin + v2Index));

				Vector_T<T, 3> v1v0 = v1XYZ - v0XYZ;
				Vector_T<T, 3> v2v0 = v2XYZ - v0XYZ;

				texcoord_type const & v0Tex(*(texsBegin + v0Index));
				texcoord_type const & v1Tex(*(texsBegin + v1Index));
				texcoord_type const & v2Tex(*(texsBegin + v2Index));

				T s1 = v1Tex.x() - v0Tex.x();
				T t1 = v1Tex.y() - v0Tex.y();

				T s2 = v2Tex.x() - v0Tex.x();
				T t2 = v2Tex.y() - v0Tex.y();

				T denominator = s1 * t2 - s2 * t1;
				Vector_T<T, 3> tangent, binormal;
				if (denominator < std::numeric_limits<T>::epsilon())
				{
					tangent = Vector_T<T, 3>(1, 0, 0);
					binormal = Vector_T<T, 3>(0, 1, 0);
				}
				else
				{
					tangent = (t2 * v1v0 - t1 * v2v0) / denominator;
					binormal = (s1 * v2v0 - s2 * v1v0) / denominator;
				}

				*(targentBegin + v0Index) += tangent;
				*(binormBegin + v0Index) += binormal;

				*(targentBegin + v1Index) += tangent;
				*(binormBegin + v1Index) += binormal;

				*(targentBegin + v2Index) += tangent;
				*(binormBegin + v2Index) += binormal;
			}

			for (int i = 0; i < num; ++ i)
			{
				*(targentBegin + i) = Normalize(*(targentBegin + i));
				*(binormBegin + i) = Normalize(*(binormBegin + i));
			}
		}

		template <typename T, typename NormalIterator, typename IndexIterator, typename PositionIterator>
		inline void
		ComputeNormal(NormalIterator normalBegin,
								IndexIterator indicesBegin, IndexIterator indicesEnd,
								PositionIterator xyzsBegin, PositionIterator xyzsEnd)
		{
			typedef typename std::iterator_traits<PositionIterator>::value_type position_type;

			NormalIterator normalEnd = normalBegin;
			std::advance(normalEnd, std::distance(xyzsBegin, xyzsEnd));
			std::fill(normalBegin, normalEnd, Vector_T<T, 3>::Zero());

			for (IndexIterator iter = indicesBegin; iter != indicesEnd; iter += 3)
			{
				uint16_t const v0Index = *(iter + 0);
				uint16_t const v1Index = *(iter + 1);
				uint16_t const v2Index = *(iter + 2);

				position_type const & v0(*(xyzsBegin + v0Index));
				position_type const & v1(*(xyzsBegin + v1Index));
				position_type const & v2(*(xyzsBegin + v2Index));

				Vector_T<T, 3> v03(v0.x(), v0.y(), v0.z());
				Vector_T<T, 3> v13(v1.x(), v1.y(), v1.z());
				Vector_T<T, 3> v23(v2.x(), v2.y(), v2.z());

				Vector_T<T, 3> vec(Cross(v13 - v03, v23 - v03));

				*(normalBegin + v0Index) += vec;
				*(normalBegin + v1Index) += vec;
				*(normalBegin + v2Index) += vec;
			}

			for (NormalIterator iter = normalBegin; iter != normalEnd; ++ iter)
			{
				*iter = Normalize(*iter);
			}
		}

		template <typename T>
		inline void
		Intersect(Vector_T<T, 3> const & v0, Vector_T<T, 3> const & v1, Vector_T<T, 3> const & v2,
						Vector_T<T, 3> const & ray_orig, Vector_T<T, 3> const & ray_dir,
						T& t, T& u, T& v)
		{
			// Find vectors for two edges sharing vert0
			Vector_T<T, 3> edge1 = v1 - v0;
			Vector_T<T, 3> edge2 = v2 - v0;

			// Begin calculating determinant - also used to calculate U parameter
			Vector_T<T, 3> pvec(Cross(ray_dir, edge2));

			// If determinant is near zero, ray lies in plane of triangle
			T det = Dot(edge1, pvec);

			Vector_T<T, 3> tvec;
			if (det > 0)
			{
				tvec = ray_orig - v0;
			}
			else
			{
				tvec = v0 - ray_orig;
				det = -det;
			}

			// Calculate U parameter
			u = Dot(tvec, pvec);

			// Prepare to test V parameter
			Vector_T<T, 3> qvec(Cross(tvec, edge1));

			// Calculate V parameter
			v = Dot(ray_dir, qvec);

			// Calculate t, scale parameters, ray intersects triangle
			t = Dot(edge2, qvec);

			T const inv_det = T(1) / det;
			v *= inv_det;
			u *= inv_det;
			t *= inv_det;
		}

		template <typename T>
		inline bool
		BaryCentricInTriangle(T const & u, T const & v)
		{
			// test bounds
			if ((u < 0) || (u > 1))
			{
				return false;
			}
			if ((v < 0) || (u + v > 1))
			{
				return false;
			}
			return true;
		}
	};
}

#include <KlayGE/Vector.hpp>
#include <KlayGE/Rect.hpp>
#include <KlayGE/Size.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Quaternion.hpp>
#include <KlayGE/Plane.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/Bound.hpp>
#include <KlayGE/Sphere.hpp>
#include <KlayGE/Box.hpp>

#endif		// _MATH_HPP
