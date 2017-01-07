/**
 * @file SIMDMath.hpp
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

#ifndef _KFL_SIMDMATH_HPP
#define _KFL_SIMDMATH_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#if defined(KLAYGE_SSE_SUPPORT)
	#define SIMD_MATH_SSE
	#include <xmmintrin.h>
#else
	#define SIMD_MATH_GENERAL
#endif

namespace KlayGE
{
	class SIMDVectorF4;
	class SIMDMatrixF4;

	namespace SIMDMathLib
	{
		// General Vector
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 Add(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 Substract(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 Multiply(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 Divide(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 Negative(SIMDVectorF4 const & rhs);

		SIMDVectorF4 BaryCentric(SIMDVectorF4 const & v1, SIMDVectorF4 const & v2, SIMDVectorF4 const & v3,
			float f, float g);
		SIMDVectorF4 CatmullRom(SIMDVectorF4 const & v0, SIMDVectorF4 const & v1,
			SIMDVectorF4 const & v2, SIMDVectorF4 const & v3, float s);
		SIMDVectorF4 CubicBezier(SIMDVectorF4 const & v0, SIMDVectorF4 const & v1,
			SIMDVectorF4 const & v2, SIMDVectorF4 const & v3, float s);
		SIMDVectorF4 CubicBSpline(SIMDVectorF4 const & v0, SIMDVectorF4 const & v1,
			SIMDVectorF4 const & v2, SIMDVectorF4 const & v3, float s);
		SIMDVectorF4 Hermite(SIMDVectorF4 const & v1, SIMDVectorF4 const & t1,
			SIMDVectorF4 const & v2, SIMDVectorF4 const & t2, float s);
		SIMDVectorF4 Lerp(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs, float s);

		SIMDVectorF4 Abs(SIMDVectorF4 const & x);
		SIMDVectorF4 Sgn(SIMDVectorF4 const & x);
		SIMDVectorF4 Sqr(SIMDVectorF4 const & x);
		SIMDVectorF4 Cube(SIMDVectorF4 const & x);

		SIMDVectorF4 LoadVector1(float v);
		SIMDVectorF4 LoadVector2(float2 const & v);
		SIMDVectorF4 LoadVector3(float3 const & v);
		SIMDVectorF4 LoadVector4(float4 const & v);
		SIMDVectorF4 LoadVector2(float const * v);
		SIMDVectorF4 LoadVector3(float const * v);
		SIMDVectorF4 LoadVector4(float const * v);
		void StoreVector1(float& fs, SIMDVectorF4 const & v);
		void StoreVector2(float2& fs, SIMDVectorF4 const & v);
		void StoreVector3(float3& fs, SIMDVectorF4 const & v);
		void StoreVector4(float4& fs, SIMDVectorF4 const & v);
		SIMDVectorF4 SetVector(float x, float y, float z, float w);
		SIMDVectorF4 SetVector(float v);
		float GetX(SIMDVectorF4 const & rhs);
		float GetY(SIMDVectorF4 const & rhs);
		float GetZ(SIMDVectorF4 const & rhs);
		float GetW(SIMDVectorF4 const & rhs);
		float GetByIndex(SIMDVectorF4 const & rhs, size_t index);
		SIMDVectorF4 SetX(SIMDVectorF4 const & rhs, float v);
		SIMDVectorF4 SetY(SIMDVectorF4 const & rhs, float v);
		SIMDVectorF4 SetZ(SIMDVectorF4 const & rhs, float v);
		SIMDVectorF4 SetW(SIMDVectorF4 const & rhs, float v);
		SIMDVectorF4 SetByIndex(SIMDVectorF4 const & rhs, float v, size_t index);

		SIMDVectorF4 Maximize(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 Minimize(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);

		SIMDVectorF4 Reflect(SIMDVectorF4 const & incident, SIMDVectorF4 const & normal);
		SIMDVectorF4 Refract(SIMDVectorF4 const & incident, SIMDVectorF4 const & normal, float refraction_index);

		// 2D Vector
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 CrossVector2(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 DotVector2(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 LengthSqVector2(SIMDVectorF4 const & rhs);
		SIMDVectorF4 LengthVector2(SIMDVectorF4 const & rhs);
		SIMDVectorF4 NormalizeVector2(SIMDVectorF4 const & rhs);
		SIMDVectorF4 TransformCoordVector2(SIMDVectorF4 const & v, SIMDMatrixF4 const & mat);
		SIMDVectorF4 TransformNormalVector2(SIMDVectorF4 const & v, SIMDMatrixF4 const & mat);

		// 3D Vector
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 Angle(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 CrossVector3(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 DotVector3(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 LengthSqVector3(SIMDVectorF4 const & rhs);
		SIMDVectorF4 LengthVector3(SIMDVectorF4 const & rhs);
		SIMDVectorF4 NormalizeVector3(SIMDVectorF4 const & rhs);
		SIMDVectorF4 TransformCoordVector3(SIMDVectorF4 const & v, SIMDMatrixF4 const & mat);
		SIMDVectorF4 TransformNormalVector3(SIMDVectorF4 const & v, SIMDMatrixF4 const & mat);
		SIMDVectorF4 TransformQuat(SIMDVectorF4 const & v, SIMDVectorF4 const & quat);
		SIMDVectorF4 Project(SIMDVectorF4 const & vec,
			SIMDMatrixF4 const & world, SIMDMatrixF4 const & view, SIMDMatrixF4 const & proj,
			int const viewport[4], float near_plane, float far_plane);
		SIMDVectorF4 Unproject(SIMDVectorF4 const & win_vec, float clip_w,
			SIMDMatrixF4 const & world, SIMDMatrixF4 const & view, SIMDMatrixF4 const & proj,
			int const viewport[4], float near_plane, float far_plane);

		// 4D Vector
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 CrossVector4(SIMDVectorF4 const & v1, SIMDVectorF4 const & v2, SIMDVectorF4 const & v3);
		SIMDVectorF4 DotVector4(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 LengthSqVector4(SIMDVectorF4 const & rhs);
		SIMDVectorF4 LengthVector4(SIMDVectorF4 const & rhs);
		SIMDVectorF4 NormalizeVector4(SIMDVectorF4 const & rhs);
		SIMDVectorF4 TransformVector4(SIMDVectorF4 const & v, SIMDMatrixF4 const & mat);

		// 4D Matrix
		///////////////////////////////////////////////////////////////////////////////
		SIMDMatrixF4 Add(SIMDMatrixF4 const & lhs, SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 Substract(SIMDMatrixF4 const & lhs, SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 Multiply(SIMDMatrixF4 const & lhs, SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 Multiply(SIMDMatrixF4 const & lhs, float rhs);
		SIMDVectorF4 Determinant(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 Negative(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 Inverse(SIMDMatrixF4 const & rhs);

		SIMDMatrixF4 LookAtLH(SIMDVectorF4 const & eye, SIMDVectorF4 const & at);
		SIMDMatrixF4 LookAtLH(SIMDVectorF4 const & eye, SIMDVectorF4 const & at,
			SIMDVectorF4 const & up);
		SIMDMatrixF4 LookAtRH(SIMDVectorF4 const & eye, SIMDVectorF4 const & at);
		SIMDMatrixF4 LookAtRH(SIMDVectorF4 const & eye, SIMDVectorF4 const & at,
			SIMDVectorF4 const & up);
		SIMDMatrixF4 OrthoLH(float w, float h, float near_plane, float far_plane);
		SIMDMatrixF4 OrthoOffCenterLH(float left, float right, float bottom, float top,
			float near_plane, float far_plane);
		SIMDMatrixF4 OrthoRH(float width, float height, float near_plane, float far_plane);
		SIMDMatrixF4 OrthoOffCenterRH(float left, float right, float bottom, float top,
			float near_plane, float far_plane);
		SIMDMatrixF4 PerspectiveLH(float width, float height, float near_plane, float far_plane);
		SIMDMatrixF4 PerspectiveFovLH(float fov, float aspect, float near_plane, float far_plane);
		SIMDMatrixF4 PerspectiveOffCenterLH(float left, float right, float bottom, float top,
			float near_plane, float far_plane);
		SIMDMatrixF4 PerspectiveRH(float width, float height, float near_plane, float far_plane);
		SIMDMatrixF4 PerspectiveFovRH(float fov, float aspect, float near_plane, float far_plane);
		SIMDMatrixF4 PerspectiveOffCenterRH(float left, float right, float bottom, float top,
			float near_plane, float far_plane);

		SIMDMatrixF4 Reflect(SIMDVectorF4 const & p);

		SIMDMatrixF4 RotationX(float x);
		SIMDMatrixF4 RotationY(float y);
		SIMDMatrixF4 RotationZ(float z);
		SIMDMatrixF4 Rotation(float angle, float x, float y, float z);
		SIMDMatrixF4 RotationMatrixYawPitchRoll(float yaw, float pitch, float roll);
		SIMDMatrixF4 RotationMatrixYawPitchRoll(SIMDVectorF4 const & ang);

		SIMDMatrixF4 Scaling(float sx, float sy, float sz);
		SIMDMatrixF4 Scaling(SIMDVectorF4 const & s);

		SIMDMatrixF4 Shadow(SIMDVectorF4 const & l, SIMDVectorF4 const & p);

		SIMDMatrixF4 QuatToMatrix(SIMDVectorF4 const & quat);

		SIMDMatrixF4 Translation(float x, float y, float z);
		SIMDMatrixF4 Translation(SIMDVectorF4 const & pos);

		SIMDMatrixF4 Transpose(SIMDMatrixF4 const & rhs);

		SIMDMatrixF4 LHToRH(SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 RHToLH(SIMDMatrixF4 const & rhs);

		void Decompose(SIMDVectorF4& scale, SIMDVectorF4& rot, SIMDVectorF4& trans, SIMDMatrixF4 const & rhs);
		SIMDMatrixF4 Transformation(SIMDVectorF4 const * scaling_center, SIMDVectorF4 const * scaling_rotation, SIMDVectorF4 const * scale,
			SIMDVectorF4 const * rotation_center, SIMDVectorF4 const * rotation, SIMDVectorF4 const * trans);

		// Quaternion
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 Conjugate(SIMDVectorF4 const & rhs);

		SIMDVectorF4 AxisToAxis(SIMDVectorF4 const & from, SIMDVectorF4 const & to);
		SIMDVectorF4 UnitAxisToUnitAxis(SIMDVectorF4 const & from, SIMDVectorF4 const & to);

		SIMDVectorF4 BaryCentricQuat(SIMDVectorF4 const & q1, SIMDVectorF4 const & q2, SIMDVectorF4 const & q3,
			float f, float g);

		SIMDVectorF4 Exp(SIMDVectorF4 const & rhs);
		SIMDVectorF4 Ln(SIMDVectorF4 const & rhs);

		SIMDVectorF4 Inverse(SIMDVectorF4 const & rhs);

		SIMDVectorF4 MultiplyQuat(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);

		SIMDVectorF4 RotationAxis(SIMDVectorF4 const & v, float angle);
		SIMDVectorF4 RotationQuatYawPitchRoll(float yaw, float pitch, float roll);
		SIMDVectorF4 RotationQuatYawPitchRoll(SIMDVectorF4 const & ang);
		void ToYawPitchRoll(float& yaw, float& pitch, float& roll, SIMDVectorF4 const & quat);
		void ToAxisAngle(SIMDVectorF4& vec, float& ang, SIMDVectorF4 const & quat);

		SIMDVectorF4 ToQuaternion(SIMDMatrixF4 const & mat);
		SIMDVectorF4 ToQuaternion(SIMDVectorF4 const & tangent, SIMDVectorF4 const & binormal, SIMDVectorF4 const & normal, int bits);

		SIMDVectorF4 Slerp(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs, float s);

		void SquadSetup(SIMDVectorF4& a, SIMDVectorF4& b, SIMDVectorF4& c,
			SIMDVectorF4 const & q0, SIMDVectorF4 const & q1, SIMDVectorF4 const & q2,
			SIMDVectorF4 const & q3);
		SIMDVectorF4 Squad(SIMDVectorF4 const & q1, SIMDVectorF4 const & a, SIMDVectorF4 const & b,
			SIMDVectorF4 const & c, float t);

		// Plane
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 DotPlane(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 DotCoord(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
		SIMDVectorF4 DotNormal(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);

		SIMDVectorF4 FromPointNormal(SIMDVectorF4 const & point, SIMDVectorF4 const & normal);
		SIMDVectorF4 FromPoints(SIMDVectorF4 const & v0, SIMDVectorF4 const & v1, SIMDVectorF4 const & v2);
		SIMDVectorF4 MultiplyPlane(SIMDVectorF4 const & p, SIMDMatrixF4 const & mat);

		SIMDVectorF4 NormalizePlane(SIMDVectorF4 const & rhs);

		float IntersectRay(SIMDVectorF4 const & p, SIMDVectorF4 const & orig, SIMDVectorF4 const & dir);

		// From Game Programming Gems 5, Section 2.6.
		void ObliqueClipping(SIMDMatrixF4& proj, SIMDVectorF4 const & clip_plane);


		// Color
		///////////////////////////////////////////////////////////////////////////////
		SIMDVectorF4 NegativeColor(SIMDVectorF4 const & rhs);
		SIMDVectorF4 ModulateColor(SIMDVectorF4 const & lhs, SIMDVectorF4 const & rhs);
	}
}

#include <KFL/SIMDVector.hpp>
#include <KFL/SIMDMatrix.hpp>

#endif		// _KFL_SIMDMATH_HPP
