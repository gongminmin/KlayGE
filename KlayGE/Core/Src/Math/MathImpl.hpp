// MathImpl.hpp
// KlayGE 数学函数库 内部头文件
// Ver 1.3.8.2
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.3.8.2
// 初次建立 (2003.1.2)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _MATHIMPL_HPP
#define _MATHIMPL_HPP

namespace KlayGE
{
	namespace KlayGEImpl
	{
		class MathStandardLib : public MathLib
		{
		public:
			~MathStandardLib()
				{ }

		public:
			// 基本数学运算
			///////////////////////////////////////////////////////////////////////////////
			virtual float Abs(float x) const;
			virtual float Sqrt(float x) const;
			virtual float RecipSqrt(float x) const;

			virtual float Pow(float x, float y) const;
			virtual float Exp(float x) const;

			virtual float Log(float x) const;
			virtual float Log10(float x) const;

			virtual float Sin(float x) const;
			virtual float Cos(float x) const;
			virtual void SinCos(float x, float& s, float& c) const;
			virtual float Tan(float x) const;

			virtual float ASin(float x) const;
			virtual float ACos(float x) const;
			virtual float ATan(float x) const;

			virtual float Sinh(float x) const;
			virtual float Cosh(float x) const;
			virtual float Tanh(float x) const;

		public:
			// 2D 向量
			///////////////////////////////////////////////////////////////////////////////
			virtual Vector2& BaryCentric(Vector2& out, const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g) const;
			virtual Vector2& Normalize(Vector2& out, const Vector2& rhs) const;
			virtual Vector4& Transform(Vector4& out, const Vector2& v, const Matrix4& mat) const;
			virtual Vector2& TransformCoord(Vector2& out, const Vector2& v, const Matrix4& mat) const;
			virtual Vector2& TransformNormal(Vector2& out, const Vector2& v, const Matrix4& mat) const;


			// 3D 向量
			///////////////////////////////////////////////////////////////////////////////
			virtual float Angle(const Vector3& lhs, const Vector3& rhs) const;
			virtual Vector3& BaryCentric(Vector3& out, const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g) const;
			virtual Vector3& Normalize(Vector3& out, const Vector3& rhs) const;
			virtual Vector3& Cross(Vector3& out, const Vector3& lhs, const Vector3& rhs) const;
			virtual Vector4& Transform(Vector4& out, const Vector3& v, const Matrix4& mat) const;
			virtual Vector3& TransformCoord(Vector3& out, const Vector3& v, const Matrix4& mat) const;
			virtual Vector3& TransformNormal(Vector3& out, const Vector3& v, const Matrix4& mat) const;
			virtual Vector3& TransQuat(Vector3& out, const Vector3& v, const Quaternion& quat) const;
			virtual Vector3& Project(Vector3& out, const Vector3& vec,
				const Matrix4& world, const Matrix4& view, const Matrix4& proj,
				const int viewport[4], float near, float far) const;
			virtual Vector3& UnProject(Vector3& out, const Vector3& winVec, float clipW,
				const Matrix4& world, const Matrix4& view, const Matrix4& proj,
				const int viewport[4], float near, float far) const;


			// 4D 向量
			///////////////////////////////////////////////////////////////////////////////
			virtual Vector4& BaryCentric(Vector4& out, const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g) const;
			virtual Vector4& Cross(Vector4& out, const Vector4& v1, const Vector4& v2, const Vector4& v3) const;
			virtual Vector4& Normalize(Vector4& out, const Vector4& rhs) const;
			virtual Vector4& Transform(Vector4& out, const Vector4& v, const Matrix4& mat) const;


			// 4D 矩阵
			///////////////////////////////////////////////////////////////////////////////
			virtual Matrix4& Multiply(Matrix4& out, const Matrix4& lhs, const Matrix4& rhs) const;
			virtual float Determinant(const Matrix4& m) const;
			virtual float Inverse(Matrix4& out, const Matrix4& m) const;
			virtual Matrix4& LookAtLH(Matrix4& out, const Vector3& vEye, const Vector3& vAt, const Vector3& vUp) const;
			virtual Matrix4& LookAtRH(Matrix4& out, const Vector3& vEye, const Vector3& vAt, const Vector3& vUp) const;
			virtual Matrix4& OrthoLH(Matrix4& out, float w, float h, float fNear, float fFar) const;
			virtual Matrix4& OrthoOffCenterLH(Matrix4& out, float l, float r, float b, float t, float fNear, float fFar) const;
			virtual Matrix4& PerspectiveLH(Matrix4& out, float w, float h, float fNear, float fFar) const;
			virtual Matrix4& PerspectiveFovLH(Matrix4& out, float fFOV, float fAspect, float fNear, float fFar) const;
			virtual Matrix4& PerspectiveOffCenterLH(Matrix4& out, float l, float r, float b, float t,
				float fNear, float fFar) const;
			virtual Matrix4& Reflect(Matrix4& out, const Plane& p) const;
			virtual Matrix4& RotationX(Matrix4& out, float x) const;
			virtual Matrix4& RotationY(Matrix4& out, float y) const;
			virtual Matrix4& RotationZ(Matrix4& out, float z) const;
			virtual Matrix4& Rotation(Matrix4& out, float angle, float x, float y, float z) const;
			virtual Matrix4& Scaling(Matrix4& out, float x, float y, float z) const;
			virtual Matrix4& Shadow(Matrix4& out, const Vector4& v, const Plane& p) const;
			virtual Matrix4& ToMatrix(Matrix4& out, const Quaternion& quat) const;
			virtual Matrix4& Translation(Matrix4& out, float x, float y, float z) const;
			virtual Matrix4& Transpose(Matrix4& out, const Matrix4& m) const;
			virtual Matrix4& AffineTransformation(Matrix4& out, float f, const Vector3& vRotationCenter = MakeVector(0.0f, 0.0f, 0.0f),
				const Quaternion& qRotation = Quaternion(0, 0, 0, 1), const Vector3& vTranslation = MakeVector(0.0f, 0.0f, 0.0f)) const;

			virtual Matrix4& LHToRH(Matrix4& out, const Matrix4& rhs) const;


			// 四元数
			///////////////////////////////////////////////////////////////////////////////
			virtual Quaternion& AxisToAxis(Quaternion& out, const Vector3& vFrom, const Vector3& vTo) const;
			virtual Quaternion& BaryCentric(Quaternion& out, const Quaternion& q1, const Quaternion& q2,
				const Quaternion& q3, float f, float g) const;
			virtual Quaternion& Exp(Quaternion& out, const Quaternion& rhs) const;
			virtual Quaternion& Inverse(Quaternion& out, const Quaternion& q) const;
			virtual Quaternion& Normalize(Quaternion& out, const Quaternion& q) const;
			virtual Quaternion& Ln(Quaternion& out, const Quaternion& rhs) const;
			virtual Quaternion& Multiply(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs) const;
			virtual Quaternion& RotationYawPitchRoll(Quaternion& out, float fYaw, float fPitch, float fRoll) const;
			virtual void ToAxisAngle(Vector3& vec, float& ang, const Quaternion& quat) const;
			virtual Quaternion& ToQuaternion(Quaternion& out, const Matrix4& m) const;
			virtual Quaternion& RotationAxis(Quaternion& out, const Vector3& v, float s) const;
			virtual Quaternion& Slerp(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs, float s) const;
			virtual Quaternion& UnitAxisToUnitAxis2(Quaternion& out, const Vector3& vFrom, const Vector3& vTo) const;


			// 平面
			///////////////////////////////////////////////////////////////////////////////
			virtual Plane& Normalize(Plane& out, const Plane& p) const;
			virtual Plane& FromPointNormal(Plane& out, const Vector3& vPoint, const Vector3& vNor) const;
			virtual Plane& FromPoints(Plane& out, const Vector3& v1, const Vector3& v2, const Vector3& v3) const;
			virtual Plane& Transform(Plane& out, const Plane& p, const Matrix4& mat) const;
			virtual bool IntersectLine(Vector3& out, const Plane& p, const Vector3& vStart, const Vector3& vEnd) const;


			// 范围
			///////////////////////////////////////////////////////////////////////////////
			virtual bool VecInBox(const Box& box, const Vector3& v) const;
			virtual bool BoundProbe(const Box& box, const Vector3& vPos, const Vector3& vDir) const;
		};

		class Math3DNowExLib : public MathStandardLib
		{
		public:
			~Math3DNowExLib()
				{ }

		public:
			// 基本数学运算
			///////////////////////////////////////////////////////////////////////////////
			virtual float Sqrt(float x) const;
			virtual float RecipSqrt(float x) const;

			virtual float Sin(float x) const;
			virtual float Cos(float x) const;
			virtual void SinCos(float x, float& s, float& c) const;

		public:
			// 2D 向量
			///////////////////////////////////////////////////////////////////////////////
			virtual Vector2& Normalize(Vector2& out, const Vector2& rhs) const;
			virtual Vector4& Transform(Vector4& out, const Vector2& v, const Matrix4& mat) const;
			virtual Vector2& TransformCoord(Vector2& out, const Vector2& v, const Matrix4& mat) const;
			virtual Vector2& TransformNormal(Vector2& out, const Vector2& v, const Matrix4& mat) const;


			// 3D 向量
			///////////////////////////////////////////////////////////////////////////////
			virtual Vector3& Normalize(Vector3& out, const Vector3& rhs) const;
			virtual Vector3& Cross(Vector3& out, const Vector3& lhs, const Vector3& rhs) const;
			virtual Vector4& Transform(Vector4& out, const Vector3& v, const Matrix4& mat) const;
			virtual Vector3& TransformCoord(Vector3& out, const Vector3& v, const Matrix4& mat) const;
			virtual Vector3& TransformNormal(Vector3& out, const Vector3& v, const Matrix4& mat) const;
			virtual Vector3& TransQuat(Vector3& out, const Vector3& v, const Quaternion& quat) const;


			// 4D 向量
			///////////////////////////////////////////////////////////////////////////////
			virtual Vector4& Cross(Vector4& out, const Vector4& v1, const Vector4& v2, const Vector4& v3) const;
			virtual Vector4& Normalize(Vector4& out, const Vector4& rhs) const;
			virtual Vector4& Transform(Vector4& out, const Vector4& v, const Matrix4& mat) const;


			// 4D 矩阵
			///////////////////////////////////////////////////////////////////////////////
			virtual Matrix4& Multiply(Matrix4& out, const Matrix4& lhs, const Matrix4& rhs) const;
			virtual Matrix4& Shadow(Matrix4& out, const Vector4& v, const Plane& p) const;
			virtual Matrix4& Transpose(Matrix4& out, const Matrix4& m) const;


			// 四元数
			///////////////////////////////////////////////////////////////////////////////
			virtual Quaternion& Normalize(Quaternion& out, const Quaternion& q) const;
			virtual Quaternion& Multiply(Quaternion& out, const Quaternion& lhs, const Quaternion& rhs) const;
			virtual Quaternion& RotationYawPitchRoll(Quaternion& out, float fYaw, float fPitch, float fRoll) const;


			// 平面
			///////////////////////////////////////////////////////////////////////////////
			virtual Plane& Normalize(Plane& out, const Plane& p) const;
			virtual Plane& Transform(Plane& out, const Plane& p, const Matrix4& mat) const;
		};
	}
}

#endif			// _MATHIMPL_HPP
