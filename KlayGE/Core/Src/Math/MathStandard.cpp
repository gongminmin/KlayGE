// MathStandard.cpp
// KlayGE 数学函数库 实现文件
// Ver 1.3.8.3
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 1.3.8.3
// 初次建立 (2003.1.11)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/MathTypes.hpp>

#include <cmath>

#include "MathImpl.hpp"

namespace KlayGE
{
	namespace KlayGEImpl
	{
		float MathStandardLib::Abs(float x) const
		{
			return std::fabsf(x);
		}

		float MathStandardLib::Sqrt(float x) const
		{
			return std::sqrtf(x);
		}

		float MathStandardLib::RecipSqrt(float x) const
		{
			return 1.0f / this->Sqrt(x);
		}

		float MathStandardLib::Pow(float x, float y) const
		{
			return std::powf(x, y);
		}

		float MathStandardLib::Exp(float x) const
		{
			return std::expf(x);
		}

		float MathStandardLib::Log(float x) const
		{
			return std::logf(x);
		}

		float MathStandardLib::Log10(float x) const
		{
			return std::log10f(x);
		}

		float MathStandardLib::Sin(float x) const
		{
			return std::sinf(x);
		}

		float MathStandardLib::Cos(float x) const
		{
			return std::cosf(x);
		}

		void MathStandardLib::SinCos(float x, float& s, float& c) const
		{
			s = this->Sin(x);
			c = this->Cos(x);
		}

		float MathStandardLib::Tan(float x) const
		{
			return std::tanf(x);
		}

		float MathStandardLib::ASin(float x) const
		{
			return std::asinf(x);
		}

		float MathStandardLib::ACos(float x) const
		{
			return std::acosf(x);
		}

		float MathStandardLib::ATan(float x) const
		{
			return std::atanf(x);
		}

		float MathStandardLib::Sinh(float x) const
		{
			return std::sinhf(x);
		}

		float MathStandardLib::Cosh(float x) const
		{
			return std::coshf(x);
		}

		float MathStandardLib::Tanh(float x) const
		{
			return std::tanhf(x);
		}


		// 2D 向量
		///////////////////////////////////////////////////////////////////////////////

		Vector2& MathStandardLib::BaryCentric(Vector2& out, const Vector2& v1,
			const Vector2& v2, const Vector2& v3,
			float f, float g) const
		{
			out = v1 + f * (v2 - v1) + g * (v3 - v1);
			return out;
		}

		Vector2& MathStandardLib::Normalize(Vector2& out, const Vector2& rhs) const
		{
			out = rhs / this->Length(rhs);
			return out;
		}

		Vector4& MathStandardLib::Transform(Vector4& out, const Vector2& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0) + mat(3, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1) + mat(3, 1),
										v.x() * mat(0, 2) + v.y() * mat(1, 2) + mat(3, 2),
										v.x() * mat(0, 3) + v.y() * mat(1, 3) + mat(3, 3));
			return out;
		}

		Vector2& MathStandardLib::TransformCoord(Vector2& out, const Vector2& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0) + mat(3, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1) + mat(3, 1));
			return out;
		}

		Vector2& MathStandardLib::TransformNormal(Vector2& out, const Vector2& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1));
			return out;
		}


		// 3D 向量
		///////////////////////////////////////////////////////////////////////////////

		float MathStandardLib::Angle(const Vector3& lhs, const Vector3& rhs) const
		{
			return this->ACos(this->Dot(lhs, rhs) / (this->Length(lhs) * this->Length(rhs)));
		}

		Vector3& MathStandardLib::BaryCentric(Vector3& out, const Vector3& v1,
			const Vector3& v2, const Vector3& v3,
			float f, float g) const
		{
			out = v1 + f * (v2 - v1) + g * (v3 - v1);
			return out;
		}

		Vector3& MathStandardLib::Normalize(Vector3& out, const Vector3& rhs) const
		{
			out = rhs / this->Length(rhs);
			return out;
		}

		Vector3& MathStandardLib::Cross(Vector3& out, const Vector3& lhs, const Vector3& rhs) const
		{
			out = MakeVector(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
										lhs.z() * rhs.x() - lhs.x() * rhs.z(),
										lhs.x() * rhs.y() - lhs.y() * rhs.x());
			return out;
		}

		Vector4& MathStandardLib::Transform(Vector4& out, const Vector3& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + mat(3, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + mat(3, 1),
										v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + mat(3, 2),
										v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + mat(3, 3));
			return out;
		}

		Vector3& MathStandardLib::TransformCoord(Vector3& out, const Vector3& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + mat(3, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + mat(3, 1),
										v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + mat(3, 2));
			return out;
		}

		Vector3& MathStandardLib::TransformNormal(Vector3& out, const Vector3& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1),
										v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2));
			return out;
		}

		Vector3& MathStandardLib::TransQuat(Vector3& out, const Vector3& v, const Quaternion& q) const
		{
			// result = av + bq + c(q.v CROSS v)
			// where
			//  a = q.w()^2 - (q.v DOT q.v)
			//  b = 2 * (q.v DOT v)
			//  c = 2q.w()
			const float a(q.w() * q.w() - this->Dot(q.v(), q.v()));
			const float b(2 * this->Dot(q.v(), v));
			const float c(q.w() + q.w());

			// Must store this, because result may alias v
			Vector3 cross;
			this->Cross(cross, q.v(), v);		// q.v CROSS v

			out = MakeVector(a * v.x() + b * q.x() + c * cross.x(),
					a * v.y() + b * q.y() + c * cross.y(),
					a * v.z() + b * q.z() + c * cross.z());
			return out;
		}

		Vector3& MathStandardLib::Project(Vector3& out, const Vector3& objVec,
			const Matrix4& world, const Matrix4& view, const Matrix4& proj,
			const int viewport[4], float near, float far) const
		{
			this->TransformCoord(out, objVec, world * view * proj);

			out.x() = (out.x() + 1) * viewport[2] / 2 + viewport[0];
			out.y() = (-out.y() + 1) * viewport[3] / 2 + viewport[1];
			out.z() = (out.z() + 1) * (far - near) / 2 + near;

			return out;
		}

		Vector3& MathStandardLib::UnProject(Vector3& out, const Vector3& winVec, float clipW,
				const Matrix4& world, const Matrix4& view, const Matrix4& proj,
				const int viewport[4], float near, float far) const
		{
			out.x() = 2 * (winVec.x() - viewport[0]) / viewport[2] - 1;
			out.y() = -(2 * (winVec.y() - viewport[1]) / viewport[3] - 1);
			out.z() = 2 * (winVec.z() - near) / (far - near) - 1;

			Matrix4 mat;
			this->Inverse(mat, world * view * proj);
			return this->TransformCoord(out, out, mat);
		}


		// 4D 向量
		///////////////////////////////////////////////////////////////////////////////

		Vector4& MathStandardLib::BaryCentric(Vector4& out, const Vector4& v1,
			const Vector4& v2, const Vector4& v3,
			float f, float g) const
		{
			out = v1 + f * (v2 - v1) + g * (v3 - v1);
			return out;
		}

		Vector4& MathStandardLib::Cross(Vector4& out, const Vector4& v1, const Vector4& v2, const Vector4& v3) const
		{
			const float A = (v2.x() * v3.y()) - (v2.y() * v3.x());
			const float B = (v2.x() * v3.z()) - (v2.z() * v3.x());
			const float C = (v2.x() * v3.w()) - (v2.w() * v3.x());
			const float D = (v2.y() * v3.z()) - (v2.z() * v3.y());
			const float E = (v2.y() * v3.w()) - (v2.w() * v3.y());
			const float F = (v2.z() * v3.w()) - (v2.w() * v3.z());

			out = MakeVector((v1.y() * F) - (v1.z() * E) + (v1.w() * D),
										-(v1.x() * F) + (v1.z() * C) - (v1.w() * B),
										(v1.x() * E) - (v1.y() * C) + (v1.w() * A),
										-(v1.x() * D) + (v1.y() * B) - (v1.z() * A));
			return out;
		}

		Vector4& MathStandardLib::Normalize(Vector4& out, const Vector4& rhs) const
		{
			out = rhs / this->Length(rhs);
			return out;
		}

		Vector4& MathStandardLib::Transform(Vector4& out, const Vector4& v, const Matrix4& mat) const
		{
			out = MakeVector(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + v.w() * mat(3, 0),
										v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + v.w() * mat(3, 1),
										v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + v.w() * mat(3, 2),
										v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + v.w() * mat(3, 3));
			return out;
		}


		// 4D 矩阵
		///////////////////////////////////////////////////////////////////////////////

		Matrix4& MathStandardLib::Translation(Matrix4& out, float x, float y, float z) const
		{
			out = Matrix4(
				1,	0,	0,	0,
				0,	1,	0,	0,
				0,	0,	1,	0,
				x,	y,	z,	1);
			return out;
		}

		Matrix4& MathStandardLib::Scaling(Matrix4& out, float sx, float sy, float sz) const
		{
			out = Matrix4(
				sx,	0,	0,	0,
				0,	sy,	0,	0,
				0,	0,	sz,	0,
				0,	0,	0,	1);
			return out;
		}

		Matrix4& MathStandardLib::RotationX(Matrix4& out, float x) const
		{
			float sx, cx;
			this->SinCos(x, sx, cx);

			out = Matrix4(
				1,	0,		0,		0,
				0,	cx,		sx,		0,
				0,	-sx,	cx,		0,
				0,	0,		0,		1);

			return out;
		}

		Matrix4& MathStandardLib::RotationY(Matrix4& out, float y) const
		{
			float sy, cy;
			this->SinCos(y, sy, cy);

			out = Matrix4(
				cy,		0,		-sy,	0,
				0,		1,		0,		0,
				sy,		0,		cy,		0,
				0,		0,		0,		1);

			return out;
		}

		Matrix4& MathStandardLib::RotationZ(Matrix4& out, float z) const
		{
			float sz, cz;
			this->SinCos(z, sz, cz);

			out = Matrix4(
				cz,		sz,		0,	0,
				-sz,	cz,		0,	0,
				0,		0,		1,	0,
				0,		0,		0,	1);

			return out;
		}

		Matrix4& MathStandardLib::Rotation(Matrix4& out, float angle, float x, float y, float z) const
		{
			Quaternion quat;
			this->RotationAxis(quat, MakeVector(x, y, z), angle);
			return this->ToMatrix(out, quat);
		}

		float MathStandardLib::Determinant(const Matrix4& rhs) const
		{
			const float _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
			const float _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
			const float _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
			const float _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
			const float _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
			const float _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

			return rhs(0, 0) * (rhs(1, 1) * _3344_3443 - rhs(1, 2) * _3244_3442 + rhs(1, 3) * _3243_3342)
				- rhs(0, 1) * (rhs(1, 0) * _3344_3443 - rhs(1, 2) * _3144_3441 + rhs(1, 3) * _3143_3341)
				+ rhs(0, 2) * (rhs(1, 0) * _3244_3442 - rhs(1, 1) * _3144_3441 + rhs(1, 3) * _3142_3241)
				- rhs(0, 3) * (rhs(1, 0) * _3243_3342 - rhs(1, 1) * _3143_3341 + rhs(1, 2) * _3142_3241);
		}

		float MathStandardLib::Inverse(Matrix4& out, const Matrix4& rhs) const
		{
			const float _2132_2231(rhs(1, 0) * rhs(2, 1) - rhs(1, 1) * rhs(2, 0));
			const float _2133_2331(rhs(1, 0) * rhs(2, 2) - rhs(1, 2) * rhs(2, 0));
			const float _2134_2431(rhs(1, 0) * rhs(2, 3) - rhs(1, 3) * rhs(2, 0));
			const float _2142_2241(rhs(1, 0) * rhs(3, 1) - rhs(1, 1) * rhs(3, 0));
			const float _2143_2341(rhs(1, 0) * rhs(3, 2) - rhs(1, 2) * rhs(3, 0));
			const float _2144_2441(rhs(1, 0) * rhs(3, 3) - rhs(1, 3) * rhs(3, 0));
			const float _2233_2332(rhs(1, 1) * rhs(2, 2) - rhs(1, 2) * rhs(2, 1));
			const float _2234_2432(rhs(1, 1) * rhs(2, 3) - rhs(1, 3) * rhs(2, 1));
			const float _2243_2342(rhs(1, 1) * rhs(3, 2) - rhs(1, 2) * rhs(3, 1));
			const float _2244_2442(rhs(1, 1) * rhs(3, 3) - rhs(1, 3) * rhs(3, 1));
			const float _2334_2433(rhs(1, 2) * rhs(2, 3) - rhs(1, 3) * rhs(2, 2));
			const float _2344_2443(rhs(1, 2) * rhs(3, 3) - rhs(1, 3) * rhs(3, 2));
			const float _3142_3241(rhs(2, 0) * rhs(3, 1) - rhs(2, 1) * rhs(3, 0));
			const float _3143_3341(rhs(2, 0) * rhs(3, 2) - rhs(2, 2) * rhs(3, 0));
			const float _3144_3441(rhs(2, 0) * rhs(3, 3) - rhs(2, 3) * rhs(3, 0));
			const float _3243_3342(rhs(2, 1) * rhs(3, 2) - rhs(2, 2) * rhs(3, 1));
			const float _3244_3442(rhs(2, 1) * rhs(3, 3) - rhs(2, 3) * rhs(3, 1));
			const float _3344_3443(rhs(2, 2) * rhs(3, 3) - rhs(2, 3) * rhs(3, 2));

			// 行列式的值
			const float det(this->Determinant(rhs));
			float invDet(1.0f);
			if (!this->Eq(det, 0.0f))
			{
				invDet = 1.0f / det;
			}

			out = Matrix4(
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

			return det;
		}

		Matrix4& MathStandardLib::Multiply(Matrix4& out, const Matrix4& lhs, const Matrix4& rhs) const
		{
			out = Matrix4(
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
			return out;
		}

		Matrix4& MathStandardLib::Shadow(Matrix4& out, const Vector4& L, const Plane& p) const
		{
			const Vector4 v(-L);
			Plane P;
			this->Normalize(P, p);
			const float d(-this->Dot(P, v));

			out = Matrix4(
				P.a() * v.x() + d,	P.a() * v.y(),		P.a() * v.z(),		P.a() * v.w(),
				P.b() * v.x(),		P.b() * v.y() + d,	P.b() * v.z(),		P.b() * v.w(),
				P.c() * v.x(),		P.c() * v.y(),		P.c() * v.z() + d,	P.c() * v.w(),
				P.d() * v.x(),		P.d() * v.y(),		P.d() * v.z(),		P.d() * v.w() + d);
			return out;
		}

		Matrix4& MathStandardLib::LookAtLH(Matrix4& out, const Vector3& vEye, 
			const Vector3& vAt, const Vector3& vUp) const
		{
			Vector3 zAxis;
			this->Normalize(zAxis, vAt - vEye);
			Vector3 xAxis;
			this->Cross(xAxis, vUp, zAxis);
			this->Normalize(xAxis, xAxis);
			Vector3 yAxis;
			this->Cross(yAxis, zAxis, xAxis);

			out = Matrix4(
				xAxis.x(),					yAxis.x(),					zAxis.x(),					0,
				xAxis.y(),					yAxis.y(),					zAxis.y(),					0,
				xAxis.z(),					yAxis.z(),					zAxis.z(),					0,
				-this->Dot(xAxis, vEye),	-this->Dot(yAxis, vEye),	-this->Dot(zAxis, vEye),	1);
			return out;
		}

		Matrix4& MathStandardLib::LookAtRH(Matrix4& out, const Vector3& vEye, 
			const Vector3& vAt, const Vector3& vUp) const
		{
			Vector3 zAxis;
			this->Normalize(zAxis, vEye - vAt);
			Vector3 xAxis;
			this->Cross(xAxis, vUp, zAxis);
			this->Normalize(xAxis, xAxis);
			Vector3 yAxis;
			this->Cross(yAxis, zAxis, xAxis);

			out = Matrix4(
				xAxis.x(),					yAxis.x(),					zAxis.x(),					0,
				xAxis.y(),					yAxis.y(),					zAxis.y(),					0,
				xAxis.z(),					yAxis.z(),					zAxis.z(),					0,
				-this->Dot(xAxis, vEye),	-this->Dot(yAxis, vEye),	-this->Dot(zAxis, vEye),	1);
			return out;
		}

		Matrix4& MathStandardLib::OrthoLH(Matrix4& out, float w, float h, float fNear, float fFar) const
		{
			const float w_2(w / 2);
			const float h_2(h / 2);

			return this->OrthoOffCenterLH(out, -w_2, w_2, -h_2, h_2, fNear, fFar);
		}

		Matrix4& MathStandardLib::OrthoOffCenterLH(Matrix4& out, float l, float r, float b, float t, 
			float fNear, float fFar) const
		{
			const float q(1.0f / (fFar - fNear));
			const float r_l(1.0f / (r - l));
			const float t_b(1.0f / (t - b));

			out = Matrix4(
				r_l + r_l,		0,				0,				0,
				0,				t_b + t_b,		0,				0,
				0,				0,				q,				0,
				-(l + r) * r_l, -(t + b) * t_b, -fNear * q,		1);
			return out;
		}

		Matrix4& MathStandardLib::PerspectiveLH(Matrix4& out, float w, float h, float fNear, float fFar) const
		{
			const float q(fFar / (fFar - fNear));
			const float fNear2(fNear + fNear);

			out = Matrix4(
				fNear2 / w,		0,				0,			0,
				0,				fNear2 / h,		0,			0,
				0,				0,				q,			1,
				0,				0,				-fNear * q, 0);
			return out;
		}

		Matrix4& MathStandardLib::PerspectiveFovLH(Matrix4& out, float fFOV, float fAspect,
			float fNear, float fFar) const
		{
			const float h(1.0f / this->Tan(fFOV * 0.5f));
			const float w(h / fAspect);
			const float q(fFar / (fFar - fNear));

			out = Matrix4(
				w,		0,		0,			0,
				0,		h,		0,			0,
				0,		0,		q,			1,
				0,		0,		-fNear * q, 0);
			return out;
		}

		Matrix4& MathStandardLib::PerspectiveOffCenterLH(Matrix4& out, float l, float r, float b, float t, 
			float fNear, float fFar) const
		{
			const float q(fFar / (fFar - fNear));
			const float fNear2(fNear + fNear);
			const float r_l(1.0f / (r - l));
			const float t_b(1.0f / (t - b));

			out = Matrix4(
				fNear2 * r_l,		0,					0,			0,
				0,					fNear2 * t_b,		0,			0,
				-(l + r)* r_l,		-(t + b) * t_b,		q,			1,
				0,					0,					-fNear * q, 0);
			return out;
		}

		Matrix4& MathStandardLib::ToMatrix(Matrix4& out, const Quaternion& quat) const
		{
			// calculate coefficients
			const float x2(quat.x() + quat.x());
			const float y2(quat.y() + quat.y());
			const float z2(quat.z() + quat.z());

			const float xx2(quat.x() * x2), xy2(quat.x() * y2), xz2(quat.x() * z2);
			const float yy2(quat.y() * y2), yz2(quat.y() * z2), zz2(quat.z() * z2);
			const float wx2(quat.w() * x2), wy2(quat.w() * y2), wz2(quat.w() * z2);

			out = Matrix4(
				1 - yy2 - zz2,	xy2 + wz2,		xz2 - wy2,		0,
				xy2 - wz2,		1 - xx2 - zz2,	yz2 + wx2,		0,
				xz2 + wy2,		yz2 - wx2,		1 - xx2 - yy2,	0,
				0,				0,				0,				1);
			return out;
		}

		Matrix4& MathStandardLib::Reflect(Matrix4& out, const Plane& p) const
		{
			Plane P;
			this->Normalize(P, p);
			const float aa2(-2 * P.a() * P.a()), ab2(-2 * P.a() * P.b()), ac2(-2 * P.a() * P.c()), ad2(-2 * P.a() * P.d());
			const float bb2(-2 * P.b() * P.b()), bc2(-2 * P.b() * P.c()), bd2(-2 * P.a() * P.c());
			const float cc2(-2 * P.c() * P.c()), cd2(-2 * P.c() * P.d());

			out = Matrix4(
				aa2 + 1,	ab2,		ac2,		0,
				ab2,		bb2 + 1,	bc2,		0,
				ac2,		bc2,		cc2 + 1,	0,
				ad2,		bd2,		cd2,		1);
			return out;
		}

		Matrix4& MathStandardLib::Transpose(Matrix4& out, const Matrix4& rhs) const
		{
			out = Matrix4(
				rhs(0, 0), rhs(1, 0), rhs(2, 0), rhs(3, 0),
				rhs(0, 1), rhs(1, 1), rhs(2, 1), rhs(3, 1),
				rhs(0, 2), rhs(1, 2), rhs(2, 2), rhs(3, 2),
				rhs(0, 3), rhs(1, 3), rhs(2, 3), rhs(3, 3));

			return out;
		}

		Matrix4& MathStandardLib::AffineTransformation(Matrix4& out, float fScaling,
			const Vector3& vRotationCenter,
			const Quaternion& qRotation,
			const Vector3& vTranslation) const
		{
			Matrix4 Ms;
			this->Scaling(Ms, fScaling, fScaling, fScaling);
			Matrix4 Mrc;
			this->Translation(Mrc, vRotationCenter.x(), vRotationCenter.y(), vRotationCenter.z());
			Matrix4 InvMrc;
			this->Inverse(InvMrc, Mrc);
			Matrix4 Mr;
			this->ToMatrix(Mr, qRotation);
			Matrix4 Mt;
			this->Translation(Mt, vTranslation.x(), vTranslation.y(), vTranslation.z());

			out = Ms * InvMrc * Mr * Mrc * Mt;
			return out;
		}

		Matrix4& MathStandardLib::LHToRH(Matrix4& out, const Matrix4& rhs) const
		{
			out = rhs;
			out(2, 0) = -out(2, 0);
			out(2, 1) = -out(2, 1);
			out(2, 2) = -out(2, 2);
			out(2, 3) = -out(2, 3);

			return out;
		}


		// 四元数
		///////////////////////////////////////////////////////////////////////////////

		// Axis to axis quaternion double angle (no normalization)
		//		Takes two points on unit sphere an angle THETA apart, returns
		//		quaternion that represents a rotation around cross product by 2*THETA.
		///////////////////////////////////////////////////////////////////////////////
		Quaternion& MathStandardLib::UnitAxisToUnitAxis2(Quaternion& out, const Vector3& vFrom,
			const Vector3& vTo) const
		{
			Vector3 vec;
			this->Cross(vec, vFrom, vTo);

			out = Quaternion(vec, this->Dot(vFrom, vTo));
			return out;
		}

		// Axis to axis quaternion 
		//		Takes two points on unit sphere an angle THETA apart, returns
		//		quaternion that represents a rotation around cross product by theta.
		///////////////////////////////////////////////////////////////////////////////
		Quaternion& MathStandardLib::AxisToAxis(Quaternion& out, const Vector3& vFrom,
			const Vector3& vTo) const
		{
			Vector3 vA;
			this->Normalize(vA, vFrom);
			Vector3 vB;
			this->Normalize(vB, vTo);
			Vector3 vHalf;
			this->Normalize(vHalf, vA + vB);

			out = this->UnitAxisToUnitAxis2(out, vA, vHalf);
			return out;
		}

		Quaternion& MathStandardLib::BaryCentric(Quaternion& out, const Quaternion& q1,
			const Quaternion& q2, const Quaternion& q3,
			float f, float g) const
		{
			const float temp(f + g);

			Quaternion qT1;
			this->Slerp(qT1, q1, q2, temp);
			Quaternion qT2;
			this->Slerp(qT2, q1, q3, temp);

			out = this->Slerp(out, qT1, qT2, g / temp);
			return out;
		}

		Quaternion& MathStandardLib::Exp(Quaternion& out, const Quaternion& rhs) const
		{
			const float fTheta(this->Length(rhs.v()));

			Vector3 vec;
			this->Normalize(vec, rhs.v());

			out = Quaternion(vec * this->Sin(fTheta), this->Cos(fTheta));
			return out;
		}

		Quaternion& MathStandardLib::Ln(Quaternion& out, const Quaternion& rhs) const
		{
			const float fTheta_2(this->ACos(rhs.w()));
			
			Vector3 vec;
			this->Normalize(vec, rhs.v() * (fTheta_2 + fTheta_2));

			out = Quaternion(vec, 0);
			return out;
		}

		Quaternion& MathStandardLib::ToQuaternion(Quaternion& out, const Matrix4& mat) const
		{
			Quaternion quat;
			float s;
			const float tr(mat(0, 0) + mat(1, 1) + mat(2, 2));

			// check the diagonal
			if (tr > 0)
			{
				s = this->Sqrt(tr + 1);
				quat.w() = s * 0.5f;
				s = 0.5f / s;
				quat.x() = (mat(1, 2) - mat(2, 1)) * s;
				quat.y() = (mat(2, 0) - mat(0, 2)) * s;
				quat.z() = (mat(0, 1) - mat(1, 0)) * s;
			}
			else
			{
				if ((mat(1, 1) > mat(0, 0)) && (mat(2, 2) <= mat(1, 1)))
				{
					s = this->Sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);

					quat.y() = s * 0.5f;

					if (!this->Eq(s, 0.0f))
					{
						s = 0.5f / s;
					}

					quat.w() = (mat(2, 0) - mat(0, 2)) * s;
					quat.z() = (mat(2, 1) + mat(1, 2)) * s;
					quat.x() = (mat(0, 1) + mat(1, 0)) * s;
				}
				else
				{
					if ((mat(1, 1) <= mat(0, 0) && mat(2, 2) > mat(0, 0)) || (mat(2, 2) > mat(1, 1)))
					{
						s = this->Sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

						quat.z() = s * 0.5f;

						if (!this->Eq(s, 0.0f))
						{
							s = 0.5f / s;
						}

						quat.w() = (mat(0, 1) - mat(1, 0)) * s;
						quat.x() = (mat(0, 2) + mat(2, 0)) * s;
						quat.y() = (mat(1, 2) + mat(2, 1)) * s;
					}
					else
					{
						s = this->Sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

						quat.x() = s * 0.5f;

						if (!this->Eq(s, 0.0f))
						{
							s = 0.5f / s;
						}

						quat.w() = (mat(1, 2) - mat(2, 1)) * s;
						quat.y() = (mat(1, 0) + mat(0, 1)) * s;
						quat.z() = (mat(2, 0) + mat(0, 2)) * s;
					}
				}
			}

			out = quat;
			return out;
		}

		void MathStandardLib::ToAxisAngle(Vector3& vec, float& ang, const Quaternion& quat) const
		{
			const float tw(this->ACos(quat.w()));
			const float scale(1.0f / this->Sin(tw));

			ang = tw + tw;
			vec.x() = quat.x() * scale, 
			vec.y() = quat.y() * scale;
			vec.z() = quat.z() * scale;
		}

		Quaternion& MathStandardLib::RotationAxis(Quaternion& out, const Vector3& v, float fAngle) const
		{
			const float fAng(fAngle * 0.5f);
			float sa, ca;
			this->SinCos(fAng, sa, ca);

			Vector3 vec;
			this->Normalize(vec, v);

			out = Quaternion(sa * vec, ca);
			return out;
		}

		Quaternion& MathStandardLib::Normalize(Quaternion& out, const Quaternion& rhs) const
		{
			out = rhs / this->Length(rhs);
			return out;
		}

		Quaternion& MathStandardLib::Multiply(Quaternion& out, const Quaternion& lhs,
			const Quaternion& rhs) const
		{
			out = Quaternion(
				lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
				lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
				lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
				lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
			return out;
		}

		Quaternion& MathStandardLib::RotationYawPitchRoll(Quaternion& out, float fYaw,
			float fPitch, float fRoll) const
		{
			const float angX(fPitch * 0.5f), angY(fYaw * 0.5f), angZ(fRoll * 0.5f);
			float sx, sy, sz;
			float cx, cy, cz;
			this->SinCos(angX, sx, cx);
			this->SinCos(angY, sy, cy);
			this->SinCos(angZ, sz, cz);

			out = Quaternion(
				sx * cy * cz + cx * sy * sz,
				cx * sy * cz - sx * cy * sz,
				cx * cy * sz - sx * sy * cz,
				sx * sy * sz + cx * cy * cz);
			return out;
		}

		Quaternion& MathStandardLib::Slerp(Quaternion& out, const Quaternion& lhs,
			const Quaternion& rhs, float fSlerp) const
		{
			float scale0, scale1;
			Quaternion q2;

			// DOT the quats to get the cosine of the angle between them
			const float cosom(this->Dot(lhs, rhs));

			// Two special cases:
			// Quats are exactly opposite, within DELTA?
			if (cosom > std::numeric_limits<float>::epsilon() - 1.0f)
			{
				// make sure they are different enough to avoid a divide by 0
				if (cosom < 1.0f - std::numeric_limits<float>::epsilon())
				{
					// SLERP away
					const float omega(this->ACos(cosom));
					const float isinom(1.0f / this->Sin(omega));
					scale0 = this->Sin((1 - fSlerp) * omega) * isinom;
					scale1 = this->Sin(fSlerp * omega) * isinom;
				}
				else
				{
					// LERP is good enough at this distance
					scale0 = 1 - fSlerp;
					scale1 = fSlerp;
				}

				q2 = rhs * scale1;
			}
			else
			{
				// SLERP towards a perpendicular quat
				// Set slerp parameters
				scale0 = this->Sin((1 - fSlerp) * KlayGE::PIdiv2);
				scale1 = this->Sin(fSlerp * KlayGE::PIdiv2);

				q2.x() = -rhs.y() * scale1;
				q2.y() = +rhs.x() * scale1;
				q2.z() = -rhs.w() * scale1;
				q2.w() = +rhs.z() * scale1;
			}

			// Compute the result
			out = scale0 * lhs + q2;
			return out;
		}

		Quaternion& MathStandardLib::Inverse(Quaternion& out, const Quaternion& rhs) const
		{
			const float inv(1.0f / this->Length(rhs));

			out = Quaternion(-rhs.x() * inv, -rhs.y() * inv, -rhs.z() * inv, rhs.w() * inv);
			return out;
		}


		// 平面
		///////////////////////////////////////////////////////////////////////////////

		// 从三个点生成平面
		Plane& MathStandardLib::FromPoints(Plane& out, const Vector3& v0, 
			const Vector3& v1, const Vector3& v2) const
		{
			Vector3 vec;
			this->Cross(vec, v1 - v0, v1 - v2);
			this->Normalize(vec, vec);
			return this->FromPointNormal(out, v0, vec);
		}

		Plane& MathStandardLib::FromPointNormal(Plane& out, const Vector3& vPoint,
			const Vector3& vNormal) const
		{
			out = Plane(vNormal.x(), vNormal.y(), vNormal.z(), -this->Dot(vPoint, vNormal));
			return out;
		}

		// 求直线和平面的交点，直线是 vStart -> vEnd
		bool MathStandardLib::IntersectLine(Vector3& out, const Plane& P, const Vector3& vStart,
			const Vector3& vEnd) const
		{
			Vector3 vP(MakeVector(0.0f, 0.0f, 0.0f));

			if (!this->Eq(P.a(), 0.0f))
			{
				vP.x() = -P.d() / P.a();
			}
			else
			{
				if (!this->Eq(P.b(), 0.0f))
				{
					vP.y() = -P.d() / P.b();
				}
				else
				{
					if (!this->Eq(P.c(), 0.0f))
					{
						vP.z() = -P.d() / P.c();
					}
				}
			}

			const Vector3 vDir(vEnd - vStart);

			const float deno(this->Dot(vDir, P.Normal()));
			if (!this->Eq(deno, 0.0f))
			{
				return false;
			}

			const float t(this->Dot(vP - vStart, P.Normal()) / deno);

			out = vStart + t * vDir;

			return true;
		}

		// plane * Matrix4
		Plane& MathStandardLib::Transform(Plane& out, const Plane& P, const Matrix4& mat) const
		{
			out = Plane(
				P.a() * mat(0, 0) + P.b() * mat(1, 0) + P.c() * mat(2, 0) + P.d() * mat(3, 0),
				P.a() * mat(0, 1) + P.b() * mat(1, 1) + P.c() * mat(2, 1) + P.d() * mat(3, 1),
				P.a() * mat(0, 2) + P.b() * mat(1, 2) + P.c() * mat(2, 2) + P.d() * mat(3, 2),
				P.a() * mat(0, 3) + P.b() * mat(1, 3) + P.c() * mat(2, 3) + P.d() * mat(3, 3));
			return out;
		}

		Plane& MathStandardLib::Normalize(Plane& out, const Plane& rhs) const
		{
			const float inv(this->RecipSqrt(rhs.a() * rhs.a() + rhs.b() * rhs.b() + rhs.c() * rhs.c()));

			out = Plane(rhs.a() * inv, rhs.b() * inv, rhs.c() * inv, rhs.d() * inv);
			return out;
		}


		// 范围
		///////////////////////////////////////////////////////////////////////////////

		bool MathStandardLib::VecInBox(const Box& box, const Vector3& v) const
		{
			return (this->InBound(v.x(), box.Min().x(), box.Max().x()))
				&& (this->InBound(v.y(), box.Min().y(), box.Max().y()))
				&& (this->InBound(v.z(), box.Min().z(), box.Max().z()));
		}

		bool MathStandardLib::BoundProbe(const Box& box, const Vector3& vPos, const Vector3& vDir) const
		{
			const Vector3 leftBottomNear(box.LeftBottomNear());
			const Vector3 leftTopNear(box.LeftTopNear());
			const Vector3 rightTopNear(box.RightTopNear());
			const Vector3 leftTopFar(box.LeftTopFar());

			Plane pNear;
			this->FromPoints(pNear, leftBottomNear, leftTopNear, rightTopNear);
			Plane pTop;
			this->FromPoints(pTop, leftTopNear, leftTopFar, rightTopNear);
			Plane pLeft;
			this->FromPoints(pLeft, leftTopFar, leftTopNear, leftBottomNear);

			const Vector3 vEnd(vPos + vDir);

			Vector3 vec;
			if (this->IntersectLine(vec, pNear, vPos, vEnd))
			{
				if ((!this->InBound(vec.x(), leftBottomNear.x(), rightTopNear.x()))
					|| (!this->InBound(vec.y(), leftBottomNear.y(), leftTopNear.y())))
				{
					return false;
				}
			}

			if (this->IntersectLine(vec, pTop, vPos, vEnd))
			{
				if ((!this->InBound(vec.x(), leftTopNear.x(), rightTopNear.x()))
					|| (!this->InBound(vec.z(), leftTopNear.z(), leftTopFar.z())))
				{
					return false;
				}
			}

			if (this->IntersectLine(vec, pLeft, vPos, vEnd))
			{
				if ((!this->InBound(vec.y(), leftBottomNear.y(), leftTopNear.y()))
					|| (!this->InBound(vec.z(), leftBottomNear.z(), leftTopFar.z())))
				{
					return false;
				}
			}

			return true;
		}
	}
}
