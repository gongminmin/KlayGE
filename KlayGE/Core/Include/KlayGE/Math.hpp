// Math.hpp
// KlayGE 数学函数库 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/MathTypes.hpp>

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <limits>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 常量定义
	/////////////////////////////////////////////////////////////////////////////////
	const float PI		= 3.141592f;			// PI
	const float PI2		= 6.283185f;			// PI * 2
	const float PIdiv2	= 1.570796f;			// PI / 2

	const float DEG90	= 1.570796f;			// 90 度
	const float DEG270	= -1.570796f;			// 270 度
	const float DEG45	= 0.7853981f;			// 45 度
	const float DEG5	= 0.0872664f;			// 5 度
	const float DEG10	= 0.1745329f;			// 10 度
	const float DEG20	= 0.3490658f;			// 20 度 
	const float DEG30	= 0.5235987f;			// 30 度
	const float DEG60	= 1.047197f;			// 60 度
	const float DEG120	= 2.094395f;			// 120 度

	const float DEG40	= 0.6981317f;			// 40 度
	const float DEG80	= 1.396263f;			// 80 度
	const float DEG140	= 2.443460f;			// 140 度
	const float DEG160	= 2.792526f;			// 160 度

	const float SQRT2	= 1.414213f;			// 根2
	const float SQRT_2	= 0.7071068f;			// 1 / SQRT2
	const float SQRT3	= 1.732050f;			// 根3

	const float DEG2RAD	= 0.01745329f;			// 角度化弧度因数
	const float RAD2DEG	= 57.29577f;			// 弧度化角度因数

	class MathLib
	{
	public:
		static MathLib* Create(CPUOptimal cpu);

		virtual ~MathLib()
			{ }

	public:
		// 求绝对值
		template <typename T>
		static T Abs(const T& x)
			{ return x < T(0) ? -x : x; }

		// 取符号
		template <typename T>
		static T Sgn(const T& x)
			{ return x < T(0) ? T(-1) : (x > T(0) ? T(1) : T(0)); }
		
		// 平方
		template <typename T>
		static T Sqr(const T& x)
			{ return x * x; }
		// 立方
		template <typename T>
		static T Cube(const T& x)
			{ return Sqr(x) * x; }

		// 角度化弧度
		template <typename T>
		static T Deg2Rad(const T& x)
			{ return static_cast<T>(x * DEG2RAD); }
		// 弧度化角度
		template <typename T>
		static T Rad2Deg(const T& x)
			{ return static_cast<T>(x * RAD2DEG); }

		// 四舍五入
		template <typename T>
		static T Round(const T& x)
		{
			return (x > 0) ? static_cast<T>(static_cast<int>(0.5f + x)) :
					-static_cast<T>(static_cast<int>(0.5f - x));
		}
		// 取整
		template <typename T>
		static T Trunc(const T& x)
			{ return static_cast<T>(static_cast<int>(x)); }

		// 取三个中小的
		template <typename T>
		static const T& Min3(const T& a, const T& b, const T& c)
			{ return std::min(std::min(a, b), c); }
		// 取三个中大的
		template <typename T>
		static const T& Max3(const T& a, const T& b, const T& c)
			{ return std::max(std::max(a, b), c); }

		// 余数
		template <typename T>
		static T Mod(const T& x, const T& y)
			{ return x % y; }
		// 浮点版本
		template <>
		static float Mod<float>(const float& x, const float& y)
			{ return std::fmodf(x, y); }
		template <>
		static double Mod<double>(const double& x, const double& y)
			{ return std::fmod(x, y); }

		// 求和
		template <typename InputIterator, typename T>
		static T Sum(InputIterator first, InputIterator last)
		{
			T sum(0);
			for (InputIterator i = first; i != last; ++ i)
			{
				sum += (*i);
			}
			return sum;
		}

		// 平均数
		template <typename InputIterator, typename T>
		static T Avg(InputIterator first, InputIterator last)
		{
			return MathLib::Sum(first, last) / (last - first);
		}

		// 限制 val 在 low 和 high 之间
		template <typename T>
		static const T& Limit(const T& val, const T& low, const T& high)
		{
			if (val < low)
			{
				return low;
			}
			else
			{
				if (val > high)
				{
					return high;
				}
				else
				{
					return val;
				}
			}
		}

		// 环绕处理
		template <typename T>
		static T Surround(const T& val, const T& low, const T& high)
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


		// 任意随机数
		static int Random()
			{ return std::rand(); }

		// 小于x的随机数
		template <typename T>
		static T Random(const T& x)
			{ return MathLib::Mod<T>(static_cast<T>(MathLib::Random()), x); }

		// 在min和max之间的随机数
		template <typename T>
		static T Random(const T& minv, const T& maxv)
			{ return minv + MathLib::Random(maxv - minv); }

		// 奇数则返回true
		template <typename T>
		static bool IsOdd(const T& x)
			{ return MathLib::Mod(x, 2) != 0; }
		// 偶数则返回true
		template <typename T>
		static bool IsEven(const T& x)
			{ return !MathLib::IsOdd(x); }

		// 判断 val 是否在 low 和 high 之间
		template <typename T>
		static bool InBound(const T& val, const T& low, const T& high)
			{ return ((val >= low) && (val <= high)); }
		
		// 判断两个数是否相等
		template <typename T>
		static bool Eq(const T& lhs, const T& rhs)
			{ return (lhs == rhs); }
		// 浮点版本
		template <>
		static bool Eq(const float& lhs, const float& rhs)
		{
			return (MathLib::Abs<float>(lhs - rhs)
				<= std::numeric_limits<float>::epsilon());
		}
		template <>
		static bool Eq(const double& lhs, const double& rhs)
		{
			return (MathLib::Abs<double>(lhs - rhs)
				<= std::numeric_limits<double>::epsilon());
		}

	public:
		// 基本数学运算
		///////////////////////////////////////////////////////////////////////////////
		virtual float Abs(float x) const = 0;
		virtual float Sqrt(float x) const = 0;
		virtual float RecipSqrt(float x) const = 0;

		virtual float Pow(float x, float y) const = 0;
		virtual float Exp(float x) const = 0;

		virtual float Log(float x) const = 0;
		virtual float Log10(float x) const = 0;

		virtual float Sin(float x) const = 0;
		virtual float Cos(float x) const = 0;
		virtual void SinCos(float x, float& s, float& c) const = 0;
		virtual float Tan(float x) const = 0;

		virtual float ASin(float x) const = 0;
		virtual float ACos(float x) const = 0;
		virtual float ATan(float x) const = 0;

		virtual float Sinh(float x) const = 0;
		virtual float Cosh(float x) const = 0;
		virtual float Tanh(float x) const = 0;


	public:

		// 几种类型的Dot
		template <typename T>
		static typename T::value_type Dot(const T& lhs, const T& rhs)
		{
			T::value_type ret(0);
			for (int i = 0; i < T::elem_num; ++ i)
			{
				ret += lhs[i] * rhs[i];
			}

			return ret;
		}

		// 几种类型的LengthSq
		template <typename T>
		static typename T::value_type LengthSq(const T& rhs)
			{ return MathLib::Dot(rhs, rhs); }

		// 几种类型的Length
		template <typename T>
		float Length(const T& rhs) const
			{ return this->Sqrt(MathLib::LengthSq(rhs)); }

		// 几种类型的Lerp
		template <typename T>
		static T& Lerp(T& out, const T& lhs, const T& rhs, float s)
		{
			out = lhs + (rhs - lhs) * s;
			return out;
		}

		template <typename T, int N>
		static Vector_T<T, N>& Maximize(Vector_T<T, N>& out,
			const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs)
		{
			std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(), std::max<T>);
			return out;
		}

		template <typename T, int N>
		static Vector_T<T, N>& Minimize(Vector_T<T, N>& out,
			const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs)
		{
			std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(), std::min<T>);
			return out;
		}


		// 2D 向量
		///////////////////////////////////////////////////////////////////////////////
		static float CCW(const Vector2& lhs, const Vector2& rhs)
			{ return lhs.x() * rhs.y() - lhs.y() * rhs.x(); }

		virtual Vector2& BaryCentric(Vector2& out,
			const Vector2& v1, const Vector2& v2, const Vector2& v3,
			float f, float g) const = 0;
		virtual Vector2& Normalize(Vector2& out, const Vector2& rhs) const = 0;
		virtual Vector4& Transform(Vector4& out, const Vector2& v, const Matrix4& mat) const = 0;
		virtual Vector2& TransformCoord(Vector2& out, const Vector2& v, const Matrix4& mat) const = 0;
		virtual Vector2& TransformNormal(Vector2& out, const Vector2& v, const Matrix4& mat) const = 0;


		// 3D 向量
		///////////////////////////////////////////////////////////////////////////////
		virtual float Angle(const Vector3& lhs, const Vector3& rhs) const = 0;
		virtual Vector3& BaryCentric(Vector3& out,
			const Vector3& v1, const Vector3& v2, const Vector3& v3,
			float f, float g) const = 0;
		virtual Vector3& Normalize(Vector3& out, const Vector3& rhs) const = 0;
		virtual Vector3& Cross(Vector3& out, const Vector3& lhs, const Vector3& rhs) const = 0;
		virtual Vector4& Transform(Vector4& out, const Vector3& v, const Matrix4& mat) const = 0;
		virtual Vector3& TransformCoord(Vector3& out, const Vector3& v, const Matrix4& mat) const = 0;
		virtual Vector3& TransformNormal(Vector3& out, const Vector3& v, const Matrix4& mat) const = 0;
		virtual Vector3& TransQuat(Vector3& out, const Vector3& v, const Quaternion& quat) const = 0;
		virtual Vector3& Project(Vector3& out, const Vector3& vec,
			const Matrix4& world, const Matrix4& view, const Matrix4& proj,
			const int viewport[4], float near, float far) const = 0;
		virtual Vector3& UnProject(Vector3& out, const Vector3& winVec, float clipW,
			const Matrix4& world, const Matrix4& view, const Matrix4& proj,
			const int viewport[4], float near, float far) const = 0;


		// 4D 向量
		///////////////////////////////////////////////////////////////////////////////
		virtual Vector4& BaryCentric(Vector4& out,
			const Vector4& v1, const Vector4& v2, const Vector4& v3,
			float f, float g) const = 0;
		virtual Vector4& Cross(Vector4& out, const Vector4& v1, const Vector4& v2, const Vector4& v3) const = 0;
		virtual Vector4& Normalize(Vector4& out, const Vector4& rhs) const = 0;
		virtual Vector4& Transform(Vector4& out, const Vector4& v, const Matrix4& mat) const = 0;


		// 4D 矩阵
		///////////////////////////////////////////////////////////////////////////////
		virtual Matrix4& Multiply(Matrix4& out, const Matrix4& lhs, const Matrix4& rhs) const = 0;
		virtual float Determinant(const Matrix4& m) const = 0;
		virtual float Inverse(Matrix4& out, const Matrix4& m) const = 0;
		virtual Matrix4& LookAtLH(Matrix4& out, const Vector3& vEye, const Vector3& vAt, const Vector3& vUp) const = 0;
		virtual Matrix4& LookAtRH(Matrix4& out, const Vector3& vEye, const Vector3& vAt, const Vector3& vUp) const = 0;
		virtual Matrix4& OrthoLH(Matrix4& out, float w, float h, float fNear, float fFar) const = 0;
		virtual Matrix4& OrthoOffCenterLH(Matrix4& out, float l, float r, float b, float t, float fNear, float fFar) const = 0;
		virtual Matrix4& PerspectiveLH(Matrix4& out, float w, float h, float fNear, float fFar) const = 0;
		virtual Matrix4& PerspectiveFovLH(Matrix4& out, float fFOV, float fAspect, float fNear, float fFar) const = 0;
		virtual Matrix4& PerspectiveOffCenterLH(Matrix4& out, float l, float r, float b, float t,
			float fNear, float fFar) const = 0;
		virtual Matrix4& Reflect(Matrix4& out, const Plane& p) const = 0;
		virtual Matrix4& RotationX(Matrix4& out, float x) const = 0;
		virtual Matrix4& RotationY(Matrix4& out, float y) const = 0;
		virtual Matrix4& RotationZ(Matrix4& out, float z) const = 0;
		virtual Matrix4& Rotation(Matrix4& out, float angle, float x, float y, float z) const = 0;
		virtual Matrix4& Scaling(Matrix4& out, float x, float y, float z) const = 0;
		virtual Matrix4& Shadow(Matrix4& out, const Vector4& v, const Plane& p) const = 0;
		virtual Matrix4& ToMatrix(Matrix4& out, const Quaternion& quat) const = 0;
		virtual Matrix4& Translation(Matrix4& out, float x, float y, float z) const = 0;
		virtual Matrix4& Transpose(Matrix4& out, const Matrix4& m) const = 0;
		virtual Matrix4& AffineTransformation(Matrix4& out, float f,
			const Vector3& vRotationCenter = Vector3::Zero(),
			const Quaternion& qRotation = Quaternion(0, 0, 0, 1),
			const Vector3& vTranslation = Vector3::Zero()) const = 0;

		virtual Matrix4& LHToRH(Matrix4& out, const Matrix4& rhs) const = 0;

		virtual Matrix4& Scaling(Matrix4& out, const Vector3& vPos) const;
		virtual Matrix4& Translation(Matrix4& out, const Vector3& vPos) const;
		virtual Matrix4& LookAtLH(Matrix4& out, const Vector3& vEye, const Vector3& vAt) const;
		virtual Matrix4& LookAtRH(Matrix4& out, const Vector3& vEye, const Vector3& vAt) const;
		virtual Matrix4& OrthoRH(Matrix4& out, float w, float h, float fNear, float fFar) const;
		virtual Matrix4& OrthoOffCenterRH(Matrix4& out, float l, float r, float b, float t, 
			float fNear, float fFar) const;
		virtual Matrix4& PerspectiveRH(Matrix4& out, float w, float h, float fNear, float fFar) const;
		virtual Matrix4& PerspectiveFovRH(Matrix4& out, float fFOV, float fAspect, float fNear, float fFar) const;
		virtual Matrix4& PerspectiveOffCenterRH(Matrix4& out, float l, float r, float b, float t, 
			float fNear, float fFar) const;

		virtual Matrix4& RHToLH(Matrix4& out, const Matrix4& rhs) const;


		// 四元数
		///////////////////////////////////////////////////////////////////////////////
		static Quaternion& Conjugate(Quaternion& out, const Quaternion& rhs)
		{
			out = Quaternion(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
			return out;
		}

		virtual Quaternion& AxisToAxis(Quaternion& out, const Vector3& vFrom, const Vector3& vTo) const = 0;
		virtual Quaternion& BaryCentric(Quaternion& out, const Quaternion& q1, const Quaternion& q2,
			const Quaternion& q3, float f, float g) const = 0;
		virtual Quaternion& Exp(Quaternion& out, const Quaternion& rhs) const = 0;
		virtual Quaternion& Inverse(Quaternion& out, const Quaternion& q) const = 0;
		virtual Quaternion& Normalize(Quaternion& out, const Quaternion& q) const = 0;
		virtual Quaternion& Ln(Quaternion& out, const Quaternion& rhs) const = 0;
		virtual Quaternion& Multiply(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs) const = 0;
		virtual Quaternion& RotationYawPitchRoll(Quaternion& out, float fYaw, float fPitch, float fRoll) const = 0;
		virtual void ToAxisAngle(Vector3& vec, float& ang, const Quaternion& quat) const = 0;
		virtual Quaternion& ToQuaternion(Quaternion& out, const Matrix4& m) const = 0;
		virtual Quaternion& RotationAxis(Quaternion& out, const Vector3& v, float s) const = 0;
		virtual Quaternion& Slerp(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs, float s) const = 0;
		virtual Quaternion& UnitAxisToUnitAxis2(Quaternion& out, const Vector3& vFrom, const Vector3& vTo) const = 0;

		virtual Quaternion& RotationYawPitchRoll(Quaternion& out, const Vector3& vAng) const;


		// 平面
		///////////////////////////////////////////////////////////////////////////////
		static float Dot(const Plane& lhs, const Vector4& rhs)
			{ return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d() * rhs.w(); }

		static float DotCoord(const Plane& lhs, const Vector3& rhs)
			{ return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d(); }

		static float DotNormal(const Plane& lhs, const Vector3& rhs)
			{ return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z(); }

		virtual Plane& Normalize(Plane& out, const Plane& p) const = 0;
		virtual Plane& FromPointNormal(Plane& out, const Vector3& vPoint, const Vector3& vNor) const = 0;
		virtual Plane& FromPoints(Plane& out, const Vector3& v1, const Vector3& v2, const Vector3& v3) const = 0;
		virtual Plane& Transform(Plane& out, const Plane& p, const Matrix4& mat) const = 0;
		virtual bool IntersectLine(Vector3& out, const Plane& p, const Vector3& vStart, const Vector3& vEnd) const = 0;


		// 颜色
		///////////////////////////////////////////////////////////////////////////////
		static Color& Negative(Color& out, const Color& rhs)
		{
			out = Color(1 - rhs.r(), 1 - rhs.g(), 1 - rhs.b(), rhs.a());
			return out;
		}

		static Color& Modulate(Color& out, const Color& lhs, const Color& rhs)
		{
			out = Color(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
			return out;
		}

		// 范围
		///////////////////////////////////////////////////////////////////////////////
		virtual bool VecInBox(const Box& box, const Vector3& v) const = 0;
		virtual bool BoundProbe(const Box& box, const Vector3& vPos, const Vector3& vDir) const = 0;
	};
}

#endif		// _MATH_HPP
