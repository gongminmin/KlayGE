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

	namespace MathLib
	{
		// 求绝对值
		template <typename T>
		inline T
		Abs(const T& x)
			{ return x < T(0) ? -x : x; }

		// 取符号
		template <typename T>
		inline T
		Sgn(const T& x)
			{ return x < T(0) ? T(-1) : (x > T(0) ? T(1) : T(0)); }
		
		// 平方
		template <typename T>
		inline T
		Sqr(const T& x)
			{ return x * x; }
		// 立方
		template <typename T>
		inline T
		Cube(const T& x)
			{ return Sqr(x) * x; }

		// 角度化弧度
		template <typename T>
		inline T
		Deg2Rad(const T& x)
			{ return static_cast<T>(x * DEG2RAD); }
		// 弧度化角度
		template <typename T>
		inline T
		Rad2Deg(const T& x)
			{ return static_cast<T>(x * RAD2DEG); }

		// 四舍五入
		template <typename T>
		inline T
		Round(const T& x)
		{
			return (x > 0) ? static_cast<T>(static_cast<int>(0.5f + x)) :
					-static_cast<T>(static_cast<int>(0.5f - x));
		}
		// 取整
		template <typename T>
		inline T
		Trunc(const T& x)
			{ return static_cast<T>(static_cast<int>(x)); }

		// 取三个中小的
		template <typename T>
		inline const T&
		Min3(const T& a, const T& b, const T& c)
			{ return std::min(std::min(a, b), c); }
		// 取三个中大的
		template <typename T>
		inline const T&
		Max3(const T& a, const T& b, const T& c)
			{ return std::max(std::max(a, b), c); }

		// 余数
		template <typename T>
		inline T
		Mod(const T& x, const T& y)
			{ return x % y; }
		// 浮点版本
		template <>
		inline float
		Mod<float>(const float& x, const float& y)
			{ return std::fmodf(x, y); }
		template <>
		inline double
		Mod<double>(const double& x, const double& y)
			{ return std::fmod(x, y); }

		// 求和
		template <typename InputIterator, typename T>
		inline T
		Sum(InputIterator first, InputIterator last)
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
		inline T
		Avg(InputIterator first, InputIterator last)
		{
			return MathLib::Sum(first, last) / (last - first);
		}

		// 限制 val 在 low 和 high 之间
		template <typename T>
		inline const T&
		Limit(const T& val, const T& low, const T& high)
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
		inline T
		Surround(const T& val, const T& low, const T& high)
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

		// 奇数则返回true
		template <typename T>
		inline bool
		IsOdd(const T& x)
			{ return MathLib::Mod(x, 2) != 0; }
		// 偶数则返回true
		template <typename T>
		inline bool
		IsEven(const T& x)
			{ return !MathLib::IsOdd(x); }

		// 判断 val 是否在 low 和 high 之间
		template <typename T>
		inline bool
		InBound(const T& val, const T& low, const T& high)
			{ return ((val >= low) && (val <= high)); }
		
		// 判断两个数是否相等
		template <typename T>
		inline bool
		Eq(const T& lhs, const T& rhs)
			{ return (lhs == rhs); }
		// 浮点版本
		template <>
		inline bool
		Eq(const float& lhs, const float& rhs)
		{
			return (MathLib::Abs<float>(lhs - rhs)
				<= std::numeric_limits<float>::epsilon());
		}
		template <>
		inline bool
		Eq(const double& lhs, const double& rhs)
		{
			return (MathLib::Abs<double>(lhs - rhs)
				<= std::numeric_limits<double>::epsilon());
		}


		// 基本数学运算
		///////////////////////////////////////////////////////////////////////////////
		float Abs(float x);
		float Sqrt(float x);
		float RecipSqrt(float x);

		float Pow(float x, float y);
		float Exp(float x);

		float Log(float x);
		float Log10(float x);

		float Sin(float x);
		float Cos(float x);
		void SinCos(float x, float& s, float& c);
		float Tan(float x);

		float ASin(float x);
		float ACos(float x);
		float ATan(float x);

		float Sinh(float x);
		float Cosh(float x);
		float Tanh(float x);


		// 几种类型的Dot
		template <typename T>
		inline typename T::value_type
		Dot(const T& lhs, const T& rhs)
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
		inline typename T::value_type
		LengthSq(const T& rhs)
			{ return MathLib::Dot(rhs, rhs); }

		// 几种类型的Length
		template <typename T>
		inline typename T::value_type
		Length(const T& rhs)
			{ return MathLib::Sqrt(MathLib::LengthSq(rhs)); }

		// 几种类型的Lerp
		template <typename T>
		inline T&
		Lerp(T& out, const T& lhs, const T& rhs, float s)
		{
			out = lhs + (rhs - lhs) * s;
			return out;
		}

		template <typename T, int N>
		inline Vector_T<T, N>&
		Maximize(Vector_T<T, N>& out,
			const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs)
		{
			std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(), std::max<T>);
			return out;
		}

		template <typename T, int N>
		inline Vector_T<T, N>&
		Minimize(Vector_T<T, N>& out,
			const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs)
		{
			std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(), std::min<T>);
			return out;
		}


		// 2D 向量
		///////////////////////////////////////////////////////////////////////////////
		float CCW(const Vector2& lhs, const Vector2& rhs);

		Vector2& BaryCentric(Vector2& out,
			const Vector2& v1, const Vector2& v2, const Vector2& v3,
			float f, float g);
		Vector2& Normalize(Vector2& out, const Vector2& rhs);
		Vector4& Transform(Vector4& out, const Vector2& v, const Matrix4& mat);
		Vector2& TransformCoord(Vector2& out, const Vector2& v, const Matrix4& mat);
		Vector2& TransformNormal(Vector2& out, const Vector2& v, const Matrix4& mat);


		// 3D 向量
		///////////////////////////////////////////////////////////////////////////////
		float Angle(const Vector3& lhs, const Vector3& rhs);
		Vector3& BaryCentric(Vector3& out,
			const Vector3& v1, const Vector3& v2, const Vector3& v3,
			float f, float g);
		Vector3& Normalize(Vector3& out, const Vector3& rhs);
		Vector3& Cross(Vector3& out, const Vector3& lhs, const Vector3& rhs);
		Vector4& Transform(Vector4& out, const Vector3& v, const Matrix4& mat);
		Vector3& TransformCoord(Vector3& out, const Vector3& v, const Matrix4& mat);
		Vector3& TransformNormal(Vector3& out, const Vector3& v, const Matrix4& mat);
		Vector3& TransQuat(Vector3& out, const Vector3& v, const Quaternion& quat);
		Vector3& Project(Vector3& out, const Vector3& vec,
			const Matrix4& world, const Matrix4& view, const Matrix4& proj,
			const int viewport[4], float near, float far);
		Vector3& UnProject(Vector3& out, const Vector3& winVec, float clipW,
			const Matrix4& world, const Matrix4& view, const Matrix4& proj,
			const int viewport[4], float near, float far);


		// 4D 向量
		///////////////////////////////////////////////////////////////////////////////
		Vector4& BaryCentric(Vector4& out,
			const Vector4& v1, const Vector4& v2, const Vector4& v3,
			float f, float g);
		Vector4& Cross(Vector4& out, const Vector4& v1, const Vector4& v2, const Vector4& v3);
		Vector4& Normalize(Vector4& out, const Vector4& rhs);
		Vector4& Transform(Vector4& out, const Vector4& v, const Matrix4& mat);


		// 4D 矩阵
		///////////////////////////////////////////////////////////////////////////////
		Matrix4& Multiply(Matrix4& out, const Matrix4& lhs, const Matrix4& rhs);
		float Determinant(const Matrix4& m);
		float Inverse(Matrix4& out, const Matrix4& m);
		Matrix4& LookAtLH(Matrix4& out, const Vector3& vEye, const Vector3& vAt, const Vector3& vUp);
		Matrix4& LookAtRH(Matrix4& out, const Vector3& vEye, const Vector3& vAt, const Vector3& vUp);
		Matrix4& OrthoLH(Matrix4& out, float w, float h, float fNear, float fFar);
		Matrix4& OrthoOffCenterLH(Matrix4& out, float l, float r, float b, float t, float fNear, float fFar);
		Matrix4& PerspectiveLH(Matrix4& out, float w, float h, float fNear, float fFar);
		Matrix4& PerspectiveFovLH(Matrix4& out, float fFOV, float fAspect, float fNear, float fFar);
		Matrix4& PerspectiveOffCenterLH(Matrix4& out, float l, float r, float b, float t,
			float fNear, float fFar);
		Matrix4& Reflect(Matrix4& out, const Plane& p);
		Matrix4& RotationX(Matrix4& out, float x);
		Matrix4& RotationY(Matrix4& out, float y);
		Matrix4& RotationZ(Matrix4& out, float z);
		Matrix4& Rotation(Matrix4& out, float angle, float x, float y, float z);
		Matrix4& Scaling(Matrix4& out, float x, float y, float z);
		Matrix4& Shadow(Matrix4& out, const Vector4& v, const Plane& p);
		Matrix4& ToMatrix(Matrix4& out, const Quaternion& quat);
		Matrix4& Translation(Matrix4& out, float x, float y, float z);
		Matrix4& Transpose(Matrix4& out, const Matrix4& m);
		Matrix4& AffineTransformation(Matrix4& out, float f,
			const Vector3& vRotationCenter = Vector3::Zero(),
			const Quaternion& qRotation = Quaternion(0, 0, 0, 1),
			const Vector3& vTranslation = Vector3::Zero());

		Matrix4& LHToRH(Matrix4& out, const Matrix4& rhs);

		Matrix4& Scaling(Matrix4& out, const Vector3& vPos);
		Matrix4& Translation(Matrix4& out, const Vector3& vPos);
		Matrix4& LookAtLH(Matrix4& out, const Vector3& vEye, const Vector3& vAt);
		Matrix4& LookAtRH(Matrix4& out, const Vector3& vEye, const Vector3& vAt);
		Matrix4& OrthoRH(Matrix4& out, float w, float h, float fNear, float fFar);
		Matrix4& OrthoOffCenterRH(Matrix4& out, float l, float r, float b, float t, 
			float fNear, float fFar);
		Matrix4& PerspectiveRH(Matrix4& out, float w, float h, float fNear, float fFar);
		Matrix4& PerspectiveFovRH(Matrix4& out, float fFOV, float fAspect, float fNear, float fFar);
		Matrix4& PerspectiveOffCenterRH(Matrix4& out, float l, float r, float b, float t, 
			float fNear, float fFar);

		Matrix4& RHToLH(Matrix4& out, const Matrix4& rhs);


		// 四元数
		///////////////////////////////////////////////////////////////////////////////
		Quaternion& Conjugate(Quaternion& out, const Quaternion& rhs);

		Quaternion& AxisToAxis(Quaternion& out, const Vector3& vFrom, const Vector3& vTo);
		Quaternion& BaryCentric(Quaternion& out, const Quaternion& q1, const Quaternion& q2,
			const Quaternion& q3, float f, float g);
		Quaternion& Exp(Quaternion& out, const Quaternion& rhs);
		Quaternion& Inverse(Quaternion& out, const Quaternion& q);
		Quaternion& Normalize(Quaternion& out, const Quaternion& q);
		Quaternion& Ln(Quaternion& out, const Quaternion& rhs);
		Quaternion& Multiply(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs);
		Quaternion& RotationYawPitchRoll(Quaternion& out, float fYaw, float fPitch, float fRoll);
		void ToAxisAngle(Vector3& vec, float& ang, const Quaternion& quat);
		Quaternion& ToQuaternion(Quaternion& out, const Matrix4& m);
		Quaternion& RotationAxis(Quaternion& out, const Vector3& v, float s);
		Quaternion& Slerp(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs, float s);
		Quaternion& UnitAxisToUnitAxis2(Quaternion& out, const Vector3& vFrom, const Vector3& vTo);

		Quaternion& RotationYawPitchRoll(Quaternion& out, const Vector3& vAng);


		// 平面
		///////////////////////////////////////////////////////////////////////////////
		float Dot(const Plane& lhs, const Vector4& rhs);
		float DotCoord(const Plane& lhs, const Vector3& rhs);
		float DotNormal(const Plane& lhs, const Vector3& rhs);

		Plane& Normalize(Plane& out, const Plane& p);
		Plane& FromPointNormal(Plane& out, const Vector3& vPoint, const Vector3& vNor);
		Plane& FromPoints(Plane& out, const Vector3& v1, const Vector3& v2, const Vector3& v3);
		Plane& Transform(Plane& out, const Plane& p, const Matrix4& mat);
		bool IntersectLine(Vector3& out, const Plane& p, const Vector3& vStart, const Vector3& vEnd);


		// 颜色
		///////////////////////////////////////////////////////////////////////////////
		Color& Negative(Color& out, const Color& rhs);
		Color& Modulate(Color& out, const Color& lhs, const Color& rhs);

		// 范围
		///////////////////////////////////////////////////////////////////////////////
		bool VecInBox(const Box& box, const Vector3& v);
		bool BoundProbe(const Box& box, const Vector3& vPos, const Vector3& vDir);
	};


	class Random
	{
	public:
		static Random& Instance();

		// 任意随机数
		int Next() const;

		// 小于x的随机数
		template <typename T>
		T Next(const T& x)
			{ return MathLib::Mod<T>(static_cast<T>(Random::Instance().Next()), x); }

		// 在min和max之间的随机数
		template <typename T>
		T Next(const T& minv, const T& maxv)
			{ return minv + Random::Instance().Next(maxv - minv); }

	private:
		Random();
	};
}

#endif		// _MATH_HPP
