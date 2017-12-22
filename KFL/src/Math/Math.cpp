/**
 * @file Math.cpp
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

#include <KFL/KFL.hpp>
#include <KFL/Detail/MathHelper.hpp>

#include <KFL/Math.hpp>

namespace KlayGE
{
	namespace MathLib
	{
		template int1 abs(int1 const & x) noexcept;
		template int2 abs(int2 const & x) noexcept;
		template int3 abs(int3 const & x) noexcept;
		template int4 abs(int4 const & x) noexcept;
		template float1 abs(float1 const & x) noexcept;
		template float2 abs(float2 const & x) noexcept;
		template float3 abs(float3 const & x) noexcept;
		template float4 abs(float4 const & x) noexcept;

		template <typename T, int N>
		Vector_T<T, N> abs(Vector_T<T, N> const & x) noexcept
		{
			Vector_T<T, N> ret;
			for (int i = 0; i < N; ++ i)
			{
				ret[i] = MathLib::abs(x[i]);
			}
			return ret;
		}

		template int1 sgn(int1 const & x) noexcept;
		template int2 sgn(int2 const & x) noexcept;
		template int3 sgn(int3 const & x) noexcept;
		template int4 sgn(int4 const & x) noexcept;
		template float1 sgn(float1 const & x) noexcept;
		template float2 sgn(float2 const & x) noexcept;
		template float3 sgn(float3 const & x) noexcept;
		template float4 sgn(float4 const & x) noexcept;
		
		template <typename T, int N>
		Vector_T<T, N> sgn(Vector_T<T, N> const & x) noexcept
		{
			Vector_T<T, N> ret;
			for (int i = 0; i < N; ++ i)
			{
				ret[i] = MathLib::sgn(x[i]);
			}
			return ret;
		}

		template int1 sqr(int1 const & x) noexcept;
		template int2 sqr(int2 const & x) noexcept;
		template int3 sqr(int3 const & x) noexcept;
		template int4 sqr(int4 const & x) noexcept;
		template float1 sqr(float1 const & x) noexcept;
		template float2 sqr(float2 const & x) noexcept;
		template float3 sqr(float3 const & x) noexcept;
		template float4 sqr(float4 const & x) noexcept;

		template <typename T, int N>
		Vector_T<T, N> sqr(Vector_T<T, N> const & x) noexcept
		{
			Vector_T<T, N> ret;
			for (int i = 0; i < N; ++ i)
			{
				ret[i] = MathLib::sqr(x[i]);
			}
			return ret;
		}

		template int1 cube(int1 const & x) noexcept;
		template int2 cube(int2 const & x) noexcept;
		template int3 cube(int3 const & x) noexcept;
		template int4 cube(int4 const & x) noexcept;
		template float1 cube(float1 const & x) noexcept;
		template float2 cube(float2 const & x) noexcept;
		template float3 cube(float3 const & x) noexcept;
		template float4 cube(float4 const & x) noexcept;

		template <typename T, int N>
		Vector_T<T, N> cube(Vector_T<T, N> const & x) noexcept
		{
			Vector_T<T, N> ret;
			for (int i = 0; i < N; ++ i)
			{
				ret[i] = MathLib::cube(x[i]);
			}
			return ret;
		}

		float abs(float x) noexcept
		{
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = x;
			fni.i &= 0x7FFFFFFF;
			return fni.f;
		}
		
		float sqrt(float x) noexcept
		{
			return std::sqrt(x);
		}

		// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
		float recip_sqrt(float number) noexcept
		{
			float const threehalfs = 1.5f;

			float const x2 = number * 0.5f;
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = number;											// evil floating point bit level hacking
			fni.i = 0x5f375a86 - (fni.i >> 1);						// what the fuck?
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));	// 1st iteration
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));		// 2nd iteration, this can be removed

			return fni.f;
		}

		float pow(float x, float y) noexcept
		{
			return std::pow(x, y);
		}

		float exp(float x) noexcept
		{
			return std::exp(x);
		}

		float log(float x) noexcept
		{
			return std::log(x);
		}

		float log10(float x) noexcept
		{
			return std::log10(x);
		}

		float sin(float x) noexcept
		{
			return std::sin(x);
		}

		float cos(float x) noexcept
		{
			return sin(x + PI / 2);
		}
		
		void sincos(float x, float& s, float& c) noexcept
		{
			s = sin(x);
			c = cos(x);
		}

		float tan(float x) noexcept
		{
			return std::tan(x);
		}

		float asin(float x) noexcept
		{
			return std::asin(x);
		}

		float acos(float x) noexcept
		{
			return std::acos(x);
		}

		float atan(float x) noexcept
		{
			return std::atan(x);
		}

		float sinh(float x) noexcept
		{
			return std::sinh(x);
		}
		
		float cosh(float x) noexcept
		{
			return std::cosh(x);
		}

		float tanh(float x) noexcept
		{
			return std::tanh(x);
		}

		int32_t SignBit(int32_t x) noexcept
		{
			return (x & 0x80000000U) ? -1 : 1;
		}

		float SignBit(float x) noexcept
		{
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = x;
			return static_cast<float>(SignBit(fni.i));
		}


		template int32_t dot(int1 const & lhs, int1 const & rhs) noexcept;
		template int32_t dot(int2 const & lhs, int2 const & rhs) noexcept;
		template int32_t dot(int3 const & lhs, int3 const & rhs) noexcept;
		template int32_t dot(int4 const & lhs, int4 const & rhs) noexcept;
		template uint32_t dot(uint1 const & lhs, uint1 const & rhs) noexcept;
		template uint32_t dot(uint2 const & lhs, uint2 const & rhs) noexcept;
		template uint32_t dot(uint3 const & lhs, uint3 const & rhs) noexcept;
		template uint32_t dot(uint4 const & lhs, uint4 const & rhs) noexcept;
		template float dot(float1 const & lhs, float1 const & rhs) noexcept;
		template float dot(float2 const & lhs, float2 const & rhs) noexcept;
		template float dot(float3 const & lhs, float3 const & rhs) noexcept;
		template float dot(float4 const & lhs, float4 const & rhs) noexcept;
		template float dot(Quaternion const & lhs, Quaternion const & rhs) noexcept;
		template float dot(Color const & lhs, Color const & rhs) noexcept;

		template <typename T>
		typename T::value_type dot(T const & lhs, T const & rhs) noexcept
		{
			return detail::dot_helper<typename T::value_type,
							T::elem_num>::Do(&lhs[0], &rhs[0]);
		}

		template int32_t length_sq(int1 const & rhs) noexcept;
		template int32_t length_sq(int2 const & rhs) noexcept;
		template int32_t length_sq(int3 const & rhs) noexcept;
		template int32_t length_sq(int4 const & rhs) noexcept;
		template uint32_t length_sq(uint1 const & rhs) noexcept;
		template uint32_t length_sq(uint2 const & rhs) noexcept;
		template uint32_t length_sq(uint3 const & rhs) noexcept;
		template uint32_t length_sq(uint4 const & rhs) noexcept;
		template float length_sq(float1 const & rhs) noexcept;
		template float length_sq(float2 const & rhs) noexcept;
		template float length_sq(float3 const & rhs) noexcept;
		template float length_sq(float4 const & rhs) noexcept;
		template float length_sq(Quaternion const & rhs) noexcept;
		template float length_sq(Plane const & rhs) noexcept;

		template <typename T>
		typename T::value_type length_sq(T const & rhs) noexcept
		{
			return dot(rhs, rhs);
		}

		template float length(float1 const & rhs) noexcept;
		template float length(float2 const & rhs) noexcept;
		template float length(float3 const & rhs) noexcept;
		template float length(float4 const & rhs) noexcept;
		template float length(Quaternion const & rhs) noexcept;
		template float length(Plane const & rhs) noexcept;

		template <typename T>
		typename T::value_type length(T const & rhs) noexcept
		{
			return sqrt(length_sq(rhs));
		}

		template float lerp(float const & lhs, float const & rhs, float s) noexcept;
		template float1 lerp(float1 const & lhs, float1 const & rhs, float s) noexcept;
		template float2 lerp(float2 const & lhs, float2 const & rhs, float s) noexcept;
		template float3 lerp(float3 const & lhs, float3 const & rhs, float s) noexcept;
		template float4 lerp(float4 const & lhs, float4 const & rhs, float s) noexcept;
		template Color lerp(Color const & lhs, Color const & rhs, float s) noexcept;

		template <typename T>
		T lerp(T const & lhs, T const & rhs, float s) noexcept
		{
			return lhs + (rhs - lhs) * s;
		}

		template int1 maximize(int1 const & lhs, int1 const & rhs) noexcept;
		template int2 maximize(int2 const & lhs, int2 const & rhs) noexcept;
		template int3 maximize(int3 const & lhs, int3 const & rhs) noexcept;
		template int4 maximize(int4 const & lhs, int4 const & rhs) noexcept;
		template uint1 maximize(uint1 const & lhs, uint1 const & rhs) noexcept;
		template uint2 maximize(uint2 const & lhs, uint2 const & rhs) noexcept;
		template uint3 maximize(uint3 const & lhs, uint3 const & rhs) noexcept;
		template uint4 maximize(uint4 const & lhs, uint4 const & rhs) noexcept;
		template float1 maximize(float1 const & lhs, float1 const & rhs) noexcept;
		template float2 maximize(float2 const & lhs, float2 const & rhs) noexcept;
		template float3 maximize(float3 const & lhs, float3 const & rhs) noexcept;
		template float4 maximize(float4 const & lhs, float4 const & rhs) noexcept;

		template <typename T>
		T maximize(T const & lhs, T const & rhs) noexcept
		{
			T ret;
			detail::max_minimize_helper<typename T::value_type, T::elem_num>::DoMax(&ret[0], &lhs[0], &rhs[0]);
			return ret;
		}

		template int1 minimize(int1 const & lhs, int1 const & rhs) noexcept;
		template int2 minimize(int2 const & lhs, int2 const & rhs) noexcept;
		template int3 minimize(int3 const & lhs, int3 const & rhs) noexcept;
		template int4 minimize(int4 const & lhs, int4 const & rhs) noexcept;
		template uint1 minimize(uint1 const & lhs, uint1 const & rhs) noexcept;
		template uint2 minimize(uint2 const & lhs, uint2 const & rhs) noexcept;
		template uint3 minimize(uint3 const & lhs, uint3 const & rhs) noexcept;
		template uint4 minimize(uint4 const & lhs, uint4 const & rhs) noexcept;
		template float1 minimize(float1 const & lhs, float1 const & rhs) noexcept;
		template float2 minimize(float2 const & lhs, float2 const & rhs) noexcept;
		template float3 minimize(float3 const & lhs, float3 const & rhs) noexcept;
		template float4 minimize(float4 const & lhs, float4 const & rhs) noexcept;

		template <typename T>
		T minimize(T const & lhs, T const & rhs) noexcept
		{
			T ret;
			detail::max_minimize_helper<typename T::value_type, T::elem_num>::DoMin(&ret[0], &lhs[0], &rhs[0]);
			return ret;
		}

		template float4 transform(float2 const & v, float4x4 const & mat) noexcept;
		template float4 transform(float3 const & v, float4x4 const & mat) noexcept;
		template float4 transform(float4 const & v, float4x4 const & mat) noexcept;

		template <typename T>
		Vector_T<typename T::value_type, 4> transform(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept
		{
			return detail::transform_helper<typename T::value_type, T::elem_num>::Do(v, mat);
		}

		template float2 transform_coord(float2 const & v, float4x4 const & mat) noexcept;
		template float3 transform_coord(float3 const & v, float4x4 const & mat) noexcept;

		template <typename T>
		T transform_coord(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept
		{
			static_assert(T::elem_num < 4, "Must be at most 4D vector.");

			Vector_T<typename T::value_type, 4> temp(detail::transform_helper<typename T::value_type, T::elem_num>::Do(v, mat));
			Vector_T<typename T::value_type, T::elem_num> ret(&temp[0]);
			if (equal(temp.w(), typename T::value_type(0)))
			{
				ret = T::Zero();
			}
			else
			{
				ret /= temp.w();
			}
			return ret;
		}

		template float2 transform_normal(float2 const & v, float4x4 const & mat) noexcept;
		template float3 transform_normal(float3 const & v, float4x4 const & mat) noexcept;

		template <typename T>
		T transform_normal(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept
		{
			static_assert(T::elem_num < 4, "Must be at most 4D vector.");

			return detail::transform_normal_helper<typename T::value_type, T::elem_num>::Do(v, mat);
		}

		template float1 bary_centric(float1 const & v1, float1 const & v2, float1 const & v3, float const & f, float const & g) noexcept;
		template float2 bary_centric(float2 const & v1, float2 const & v2, float2 const & v3, float const & f, float const & g) noexcept;
		template float3 bary_centric(float3 const & v1, float3 const & v2, float3 const & v3, float const & f, float const & g) noexcept;
		template float4 bary_centric(float4 const & v1, float4 const & v2, float4 const & v3, float const & f, float const & g) noexcept;

		template <typename T>
		T bary_centric(T const & v1, T const & v2, T const & v3, typename T::value_type const & f, typename T::value_type const & g) noexcept
		{
			return (1 - f - g) * v1 + f * v2 + g * v3;
		}

		template float1 normalize(float1 const & rhs) noexcept;
		template float2 normalize(float2 const & rhs) noexcept;
		template float3 normalize(float3 const & rhs) noexcept;
		template float4 normalize(float4 const & rhs) noexcept;
		template Quaternion normalize(Quaternion const & rhs) noexcept;

		template <typename T>
		T normalize(T const & rhs) noexcept
		{
			return rhs * recip_sqrt(length_sq(rhs));
		}

		template Plane normalize(Plane const & rhs) noexcept;

		template <typename T>
		Plane_T<T> normalize(Plane_T<T> const & rhs) noexcept
		{
			T const inv(T(1) / length(rhs.Normal()));
			return Plane_T<T>(rhs.a() * inv, rhs.b() * inv, rhs.c() * inv, rhs.d() * inv);
		}

		template float3 reflect(float3 const & incident, float3 const & normal) noexcept;

		template <typename T>
		T reflect(T const & incident, T const & normal) noexcept
		{
			return incident - 2 * dot(incident, normal) * normal;
		}

		template float3 refract(float3 const & incident, float3 const & normal, float const & refraction_index) noexcept;

		template <typename T>
		T refract(T const & incident, T const & normal, typename T::value_type const & refraction_index) noexcept
		{
			typename T::value_type t = dot(incident, normal);
			typename T::value_type r = typename T::value_type(1) - refraction_index * refraction_index * (typename T::value_type(1) - t * t);

			if (r < typename T::value_type(0))
			{
				// Total internal reflection
				return T::Zero();
			}
			else
			{
				typename T::value_type s = refraction_index * t + sqrt(abs(r));
				return refraction_index * incident - s * normal;
			}
		}

		template float fresnel_term(float const & cos_theta, float const & refraction_index) noexcept;

		template <typename T>
		T fresnel_term(T const & cos_theta, T const & refraction_index) noexcept
		{
			T g = sqrt(sqr(refraction_index) + sqr(cos_theta) - 1);
			return T(0.5) * sqr(g + cos_theta) / sqr(g - cos_theta)
					* (sqr(cos_theta * (g + cos_theta) - 1) / sqr(cos_theta * (g - cos_theta) + 1) + 1);
		}

		template float2 catmull_rom(float2 const & v0, float2 const & v1, float2 const & v2,
			float2 const & v3, float s) noexcept;
		template float3 catmull_rom(float3 const & v0, float3 const & v1, float3 const & v2,
			float3 const & v3, float s) noexcept;
		template float4 catmull_rom(float4 const & v0, float4 const & v1, float4 const & v2,
			float4 const & v3, float s) noexcept;

		template <typename T, int N>
		Vector_T<T, N> catmull_rom(Vector_T<T, N> const & v0, Vector_T<T, N> const & v1,
			Vector_T<T, N> const & v2, Vector_T<T, N> const & v3, T s) noexcept
		{
			T const s2 = s * s;
			T const s3 = s2 * s;
			return ((-s3 + 2 * s2 - s) * v0 + (3 * s3 - 5 * s2 + 2) * v1
				+ (-3 * s3 + 4 * s2 + s) * v2 + (s3 - s2) * v3) * T(0.5);
		}

		template float2 hermite(float2 const & v1, float2 const & t1, float2 const & v2,
			float2 const & t2, float s) noexcept;
		template float3 hermite(float3 const & v1, float3 const & t1, float3 const & v2,
			float3 const & t2, float s) noexcept;
		template float4 hermite(float4 const & v1, float4 const & t1, float4 const & v2,
			float4 const & t2, float s) noexcept;

		template <typename T, int N>
		Vector_T<T, N> hermite(Vector_T<T, N> const & v1, Vector_T<T, N> const & t1,
			Vector_T<T, N> const & v2, Vector_T<T, N> const & t2, T s) noexcept
		{
			T const s2 = s * s;
			T const s3 = s2 * s;
			T const h1 = T(2) * s3 - T(3) * s2 + T(1);
			T const h2 = s3 - T(2) * s2 + s;
			T const h3 = T(-2) * s3 + T(3) * s2;
			T const h4 = s3 - s2;

			return h1 * v1 + h2 * t1 + h3 * v2 + h4 * t2;
		}

		template float2 cubic_b_spline(float2 const & v0, float2 const & v1, float2 const & v2,
			float2 const & v3, float s) noexcept;
		template float3 cubic_b_spline(float3 const & v0, float3 const & v1, float3 const & v2,
			float3 const & v3, float s) noexcept;
		template float4 cubic_b_spline(float4 const & v0, float4 const & v1, float4 const & v2,
			float4 const & v3, float s) noexcept;

		template <typename T, int N>
		Vector_T<T, N> cubic_b_spline(Vector_T<T, N> const & v0, Vector_T<T, N> const & v1,
			Vector_T<T, N> const & v2, Vector_T<T, N> const & v3, T s) noexcept
		{
			// From http://en.wikipedia.org/wiki/B-spline

			T const s2 = s * s;
			T const s3 = s2 * s;
			return ((-s3 + 3 * s2 - 3 * s + 1) * v0 + (3 * s3 - 6 * s2 + 4) * v1
				+ (-3 * s3 + 3 * s2 + 3 * s + 1) * v2 + s3 * v3) / T(6);
		}

		template float2 cubic_bezier(float2 const & v0, float2 const & v1, float2 const & v2,
			float2 const & v3, float s) noexcept;
		template float3 cubic_bezier(float3 const & v0, float3 const & v1, float3 const & v2,
			float3 const & v3, float s) noexcept;
		template float4 cubic_bezier(float4 const & v0, float4 const & v1, float4 const & v2,
			float4 const & v3, float s) noexcept;

		template <typename T, int N>
		Vector_T<T, N> cubic_bezier(Vector_T<T, N> const & v0, Vector_T<T, N> const & v1,
			Vector_T<T, N> const & v2, Vector_T<T, N> const & v3, T s) noexcept
		{
			// From http://en.wikipedia.org/wiki/B%C3%A9zier_curve

			T const s2 = s * s;
			T const s3 = s2 * s;
			return ((-s3 + 3 * s2 - 3 * s + 1) * v0 + (3 * s3 - 6 * s2 + 3 * s) * v1
				+ (-3 * s3 + 3 * s2) * v2 + s3 * v3);
		}

		// 2D Vector
		///////////////////////////////////////////////////////////////////////////////

		template int32_t cross(int2 const & lhs, int2 const & rhs) noexcept;
		template uint32_t cross(uint2 const & lhs, uint2 const & rhs) noexcept;
		template float cross(float2 const & lhs, float2 const & rhs) noexcept;

		template <typename T>
		T cross(Vector_T<T, 2> const & lhs, Vector_T<T, 2> const & rhs) noexcept
		{
			return lhs.x() * rhs.y() - lhs.y() * rhs.x();
		}

		// 3D Vector
		///////////////////////////////////////////////////////////////////////////////

		template float angle(float3 const & lhs, float3 const & rhs) noexcept;

		template <typename T>
		T angle(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs) noexcept
		{
			return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
		}

		template int3 cross(int3 const & lhs, int3 const & rhs) noexcept;
		template uint3 cross(uint3 const & lhs, uint3 const & rhs) noexcept;
		template float3 cross(float3 const & lhs, float3 const & rhs) noexcept;

		template <typename T>
		Vector_T<T, 3> cross(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs) noexcept
		{
			return Vector_T<T, 3>(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
				lhs.z() * rhs.x() - lhs.x() * rhs.z(),
				lhs.x() * rhs.y() - lhs.y() * rhs.x());
		}

		template float3 transform_quat(float3 const & v, Quaternion const & quat) noexcept;

		template <typename T>
		Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat) noexcept
		{
			return v + cross(quat.v(), cross(quat.v(), v) + quat.w() * v) * T(2);
		}

		template float3 project(float3 const & vec,
			float4x4 const & world, float4x4 const & view, float4x4 const & proj,
			int const viewport[4], float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Vector_T<T, 3> project(Vector_T<T, 3> const & vec,
			Matrix4_T<T> const & world, Matrix4_T<T> const & view, Matrix4_T<T> const & proj,
			int const viewport[4], T const & nearPlane, T const & farPlane) noexcept
		{
			Vector_T<T, 4> temp(transform(vec, world));
			temp = transform(temp, view);
			temp = transform(temp, proj);
			if (!MathLib::equal(temp.w(), T(0)))
			{
				temp /= temp.w();
			}

			Vector_T<T, 3> ret;
			ret.x() = (temp.x() + 1) * viewport[2] / 2 + viewport[0];
			ret.y() = (-temp.y() + 1) * viewport[3] / 2 + viewport[1];
			ret.z() = temp.z() * (farPlane - nearPlane) + nearPlane;
			return ret;
		}

		template float3 unproject(float3 const & winVec, float const & clipW,
			float4x4 const & world, float4x4 const & view, float4x4 const & proj,
			int const viewport[4], float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Vector_T<T, 3> unproject(Vector_T<T, 3> const & winVec, T const & clipW,
			Matrix4_T<T> const & world, Matrix4_T<T> const & view, Matrix4_T<T> const & proj,
			int const viewport[4], T const & nearPlane, T const & farPlane) noexcept
		{
			Vector_T<T, 4> temp;
			temp.x() = 2 * (winVec.x() - viewport[0]) / viewport[2] - 1;
			temp.y() = -(2 * (winVec.y() - viewport[1]) / viewport[3] - 1);
			temp.z() = (winVec.z() - nearPlane) / (farPlane - nearPlane);
			temp.w() = clipW;

			Matrix4_T<T> const mat(inverse(world * view * proj));
			temp = transform(temp, mat);

			return Vector_T<T, 3>(temp.x(), temp.y(), temp.z()) / temp.w();
		}

		template float ortho_area(float3 const & view_dir, AABBox const & aabbox) noexcept;

		// From http://www.codersnotes.com/notes/projected-area-of-an-aabb, mentioned by Ming Tu
		template <typename T>
		T ortho_area(Vector_T<T, 3> const & view_dir, AABBox_T<T> const & aabbox) noexcept
		{
			Vector_T<T, 3> size = aabbox.Max() - aabbox.Min();
			return dot(Vector_T<T, 3>(abs(view_dir.x()), abs(view_dir.y()), abs(view_dir.z())),
				Vector_T<T, 3>(size.y() * size.z(), size.z() * size.x(), size.x() * size.y()));
		}

		template float perspective_area(float3 const & view_pos, float4x4 const & view_proj, AABBox const & aabbox) noexcept;

		template <typename T>
		T perspective_area(Vector_T<T, 3> const & view_pos, Matrix4_T<T> const & view_proj, AABBox_T<T> const & aabbox) noexcept
		{
			static uint8_t const HULL_VERTEX[64][7] = 
			{
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 4, 7, 3, 0, 0, 4 },
				{ 1, 2, 6, 5, 0, 0, 4 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 1, 5, 4, 0, 0, 4 },
				{ 0, 1, 5, 4, 7, 3, 6 },
				{ 0, 1, 2, 6, 5, 4, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 2, 3, 7, 6, 0, 0, 4 },
				{ 4, 7, 6, 2, 3, 0, 6 },
				{ 2, 3, 7, 6, 5, 1, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 3, 2, 1, 0, 0, 4 },
				{ 0, 4, 7, 3, 2, 1, 6 },
				{ 0, 3, 2, 6, 5, 1, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 3, 2, 1, 5, 4, 6 },
				{ 2, 1, 5, 4, 7, 3, 6 },
				{ 0, 3, 2, 6, 5, 4, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 3, 7, 6, 2, 1, 6 },
				{ 0, 4, 7, 6, 2, 1, 6 },
				{ 0, 3, 7, 6, 5, 1, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 4, 5, 6, 7, 0, 0, 4 },
				{ 4, 5, 6, 7, 3, 0, 6 },
				{ 1, 2, 6, 7, 4, 5, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 1, 5, 6, 7, 4, 6 },
				{ 0, 1, 5, 6, 7, 3, 6 },
				{ 0, 1, 2, 6, 7, 4, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 2, 3, 7, 4, 5, 6, 6 },
				{ 0, 4, 5, 6, 2, 3, 6 },
				{ 1, 2, 3, 7, 4, 5, 6 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0 }
			};

			uint32_t const pos = ((view_pos.x() < aabbox.Min().x()))
				| ((view_pos.x() > aabbox.Max().x()) << 1)
				| ((view_pos.y() < aabbox.Min().y()) << 2)
				| ((view_pos.y() > aabbox.Max().y()) << 3)
				| ((view_pos.z() < aabbox.Min().z()) << 4)
				| ((view_pos.z() > aabbox.Max().z()) << 5);
			if (0 == pos)
			{
				return 1;
			}
			uint32_t const num = HULL_VERTEX[pos][6];
			if (0 == num)
			{
				return 0;
			}

			Vector_T<T, 2> dst[8];
			for (uint32_t i = 0; i < num; ++ i)
			{
				Vector_T<T, 3> v = MathLib::transform_coord(aabbox.Corner(HULL_VERTEX[pos][i]), view_proj);
				dst[i] = Vector_T<T, 2>(v.x(), v.y()) * T(0.5) + Vector_T<T, 2>(0.5, 0.5);
			}

			T sum = abs((dst[num - 1].x() - dst[0].x()) * (dst[num - 1].y() + dst[0].y()));
			for (uint32_t i = 0; i < num - 1; ++ i)
			{
				uint32_t const next = i + 1;
				sum += abs((dst[i].x() - dst[next].x()) * (dst[i].y() + dst[next].y()));
			}
			return sum / 2;
		}


		// 4D Vector
		///////////////////////////////////////////////////////////////////////////////
		
		template int4 cross(int4 const & v1, int4 const & v2, int4 const & v3) noexcept;
		template float4 cross(float4 const & v1, float4 const & v2, float4 const & v3) noexcept;

		template <typename T>
		Vector_T<T, 4> cross(Vector_T<T, 4> const & v1, Vector_T<T, 4> const & v2, Vector_T<T, 4> const & v3) noexcept
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


		// 4D Matrix
		///////////////////////////////////////////////////////////////////////////////

		template float4x4 mul(float4x4 const & lhs, float4x4 const & rhs) noexcept;

		template <typename T>
		Matrix4_T<T> mul(Matrix4_T<T> const & lhs, Matrix4_T<T> const & rhs) noexcept
		{
			Matrix4_T<T> const tmp(transpose(rhs));

			return Matrix4_T<T>(
				lhs(0, 0) * tmp(0, 0) + lhs(0, 1) * tmp(0, 1) + lhs(0, 2) * tmp(0, 2) + lhs(0, 3) * tmp(0, 3),
				lhs(0, 0) * tmp(1, 0) + lhs(0, 1) * tmp(1, 1) + lhs(0, 2) * tmp(1, 2) + lhs(0, 3) * tmp(1, 3),
				lhs(0, 0) * tmp(2, 0) + lhs(0, 1) * tmp(2, 1) + lhs(0, 2) * tmp(2, 2) + lhs(0, 3) * tmp(2, 3),
				lhs(0, 0) * tmp(3, 0) + lhs(0, 1) * tmp(3, 1) + lhs(0, 2) * tmp(3, 2) + lhs(0, 3) * tmp(3, 3),

				lhs(1, 0) * tmp(0, 0) + lhs(1, 1) * tmp(0, 1) + lhs(1, 2) * tmp(0, 2) + lhs(1, 3) * tmp(0, 3),
				lhs(1, 0) * tmp(1, 0) + lhs(1, 1) * tmp(1, 1) + lhs(1, 2) * tmp(1, 2) + lhs(1, 3) * tmp(1, 3),
				lhs(1, 0) * tmp(2, 0) + lhs(1, 1) * tmp(2, 1) + lhs(1, 2) * tmp(2, 2) + lhs(1, 3) * tmp(2, 3),
				lhs(1, 0) * tmp(3, 0) + lhs(1, 1) * tmp(3, 1) + lhs(1, 2) * tmp(3, 2) + lhs(1, 3) * tmp(3, 3),

				lhs(2, 0) * tmp(0, 0) + lhs(2, 1) * tmp(0, 1) + lhs(2, 2) * tmp(0, 2) + lhs(2, 3) * tmp(0, 3),
				lhs(2, 0) * tmp(1, 0) + lhs(2, 1) * tmp(1, 1) + lhs(2, 2) * tmp(1, 2) + lhs(2, 3) * tmp(1, 3),
				lhs(2, 0) * tmp(2, 0) + lhs(2, 1) * tmp(2, 1) + lhs(2, 2) * tmp(2, 2) + lhs(2, 3) * tmp(2, 3),
				lhs(2, 0) * tmp(3, 0) + lhs(2, 1) * tmp(3, 1) + lhs(2, 2) * tmp(3, 2) + lhs(2, 3) * tmp(3, 3),

				lhs(3, 0) * tmp(0, 0) + lhs(3, 1) * tmp(0, 1) + lhs(3, 2) * tmp(0, 2) + lhs(3, 3) * tmp(0, 3),
				lhs(3, 0) * tmp(1, 0) + lhs(3, 1) * tmp(1, 1) + lhs(3, 2) * tmp(1, 2) + lhs(3, 3) * tmp(1, 3),
				lhs(3, 0) * tmp(2, 0) + lhs(3, 1) * tmp(2, 1) + lhs(3, 2) * tmp(2, 2) + lhs(3, 3) * tmp(2, 3),
				lhs(3, 0) * tmp(3, 0) + lhs(3, 1) * tmp(3, 1) + lhs(3, 2) * tmp(3, 2) + lhs(3, 3) * tmp(3, 3));
		}

		template float determinant(float4x4 const & rhs) noexcept;

		template <typename T>
		T determinant(Matrix4_T<T> const & rhs) noexcept
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

		template float4x4 inverse(float4x4 const & rhs) noexcept;

		template <typename T>
		Matrix4_T<T> inverse(Matrix4_T<T> const & rhs) noexcept
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
			T const det(determinant(rhs));
			if (equal<T>(det, 0))
			{
				return rhs;
			}
			else
			{
				T invDet(T(1) / det);

				return Matrix4_T<T>(
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
		}

		template float4x4 look_at_lh(float3 const & vEye, float3 const & vAt) noexcept;

		template <typename T>
		Matrix4_T<T> look_at_lh(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt) noexcept
		{
			return look_at_lh(vEye, vAt, Vector_T<T, 3>(0, 1, 0));
		}

		template float4x4 look_at_lh(float3 const & vEye, float3 const & vAt,
			float3 const & vUp) noexcept;

		template <typename T>
		Matrix4_T<T> look_at_lh(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt,
			Vector_T<T, 3> const & vUp) noexcept
		{
			Vector_T<T, 3> zAxis(normalize(vAt - vEye));
			Vector_T<T, 3> xAxis(normalize(cross(vUp, zAxis)));
			Vector_T<T, 3> yAxis(cross(zAxis, xAxis));

			return Matrix4_T<T>(
				xAxis.x(),			yAxis.x(),			zAxis.x(),			0,
				xAxis.y(),			yAxis.y(),			zAxis.y(),			0,
				xAxis.z(),			yAxis.z(),			zAxis.z(),			0,
				-dot(xAxis, vEye),	-dot(yAxis, vEye),	-dot(zAxis, vEye),	1);
		}

		template float4x4 look_at_rh(float3 const & vEye, float3 const & vAt) noexcept;

		template <typename T>
		Matrix4_T<T> look_at_rh(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt) noexcept
		{
			return look_at_rh(vEye, vAt, Vector_T<T, 3>(0, 1, 0));
		}

		template float4x4 look_at_rh(float3 const & vEye, float3 const & vAt,
			float3 const & vUp) noexcept;

		template <typename T>
		Matrix4_T<T> look_at_rh(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt,
			Vector_T<T, 3> const & vUp) noexcept
		{
			Vector_T<T, 3> zAxis(normalize(vEye - vAt));
			Vector_T<T, 3> xAxis(normalize(cross(vUp, zAxis)));
			Vector_T<T, 3> yAxis(cross(zAxis, xAxis));

			return Matrix4_T<T>(
				xAxis.x(),			yAxis.x(),			zAxis.x(),			0,
				xAxis.y(),			yAxis.y(),			zAxis.y(),			0,
				xAxis.z(),			yAxis.z(),			zAxis.z(),			0,
				-dot(xAxis, vEye),	-dot(yAxis, vEye),	-dot(zAxis, vEye),	1);
		}

		template float4x4 ortho_lh(float const & w, float const & h,
			float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> ortho_lh(T const & w, T const & h, T const & nearPlane, T const & farPlane) noexcept
		{
			T const w_2(w / 2);
			T const h_2(h / 2);
			return ortho_off_center_lh(-w_2, w_2, -h_2, h_2, nearPlane, farPlane);
		}

		template float4x4 ortho_off_center_lh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> ortho_off_center_lh(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane) noexcept
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

		template float4x4 perspective_lh(float const & width, float const & height, float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> perspective_lh(T const & width, T const & height, T const & nearPlane, T const & farPlane) noexcept
		{
			T const q(farPlane / (farPlane - nearPlane));
			T const near2(nearPlane + nearPlane);

			return Matrix4_T<T>(
				near2 / width,	0,				0,				0,
				0,				near2 / height,	0,				0,
				0,				0,				q,				1,
				0,				0,				-nearPlane * q, 0);
		}

		template float4x4 perspective_fov_lh(float const & fov, float const & aspect, float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> perspective_fov_lh(T const & fov, T const & aspect, T const & nearPlane, T const & farPlane) noexcept
		{
			T const h(T(1) / tan(fov / 2));
			T const w(h / aspect);
			T const q(farPlane / (farPlane - nearPlane));

			return Matrix4_T<T>(
				w,		0,		0,				0,
				0,		h,		0,				0,
				0,		0,		q,				1,
				0,		0,		-nearPlane * q, 0);
		}

		template float4x4 perspective_off_center_lh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> perspective_off_center_lh(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane) noexcept
		{
			T const q(farPlane / (farPlane - nearPlane));
			T const near2(nearPlane + nearPlane);
			T const invWidth(T(1) / (right - left));
			T const invHeight(T(1) / (top - bottom));

			return Matrix4_T<T>(
				near2 * invWidth,			0,								0,				0,
				0,							near2 * invHeight,				0,				0,
				-(left + right) * invWidth,	-(top + bottom) * invHeight,	q,				1,
				0,							0,								-nearPlane * q, 0);
		}

		template float4x4 reflect(Plane const & p) noexcept;

		template <typename T>
		Matrix4_T<T> reflect(Plane_T<T> const & p) noexcept
		{
			Plane_T<T> P(normalize(p));
			T const aa2(-2 * P.a() * P.a()), ab2(-2 * P.a() * P.b()), ac2(-2 * P.a() * P.c()), ad2(-2 * P.a() * P.d());
			T const bb2(-2 * P.b() * P.b()), bc2(-2 * P.b() * P.c()), bd2(-2 * P.b() * P.d());
			T const cc2(-2 * P.c() * P.c()), cd2(-2 * P.c() * P.d());

			return Matrix4_T<T>(
				aa2 + 1,	ab2,		ac2,		0,
				ab2,		bb2 + 1,	bc2,		0,
				ac2,		bc2,		cc2 + 1,	0,
				ad2,		bd2,		cd2,		1);
		}

		template float4x4 rotation_x(float const & x) noexcept;

		template <typename T>
		Matrix4_T<T> rotation_x(T const & x) noexcept
		{
			float sx, cx;
			sincos(x, sx, cx);

			return Matrix4_T<T>(
				1,	0,		0,		0,
				0,	cx,		sx,		0,
				0,	-sx,	cx,		0,
				0,	0,		0,		1);
		}

		template float4x4 rotation_y(float const & y) noexcept;

		template <typename T>
		Matrix4_T<T> rotation_y(T const & y) noexcept
		{
			float sy, cy;
			sincos(y, sy, cy);

			return Matrix4_T<T>(
				cy,		0,		-sy,	0,
				0,		1,		0,		0,
				sy,		0,		cy,		0,
				0,		0,		0,		1);
		}

		template float4x4 rotation_z(float const & z) noexcept;

		template <typename T>
		Matrix4_T<T> rotation_z(T const & z) noexcept
		{
			float sz, cz;
			sincos(z, sz, cz);

			return Matrix4_T<T>(
				cz,		sz,		0,		0,
				-sz,	cz,		0,		0,
				0,		0,		1,		0,
				0,		0,		0,		1);
		}

		template float4x4 rotation(float const & angle, float const & x, float const & y, float const & z) noexcept;

		template <typename T>
		Matrix4_T<T> rotation(T const & angle, T const & x, T const & y, T const & z) noexcept
		{
			Quaternion_T<T> const quat(rotation_axis(Vector_T<T, 3>(x, y, z), angle));
			return to_matrix(quat);
		}

		template float4x4 rotation_matrix_yaw_pitch_roll(float const & yaw, float const & pitch, float const & roll) noexcept;

		template <typename T>
		Matrix4_T<T> rotation_matrix_yaw_pitch_roll(T const & yaw, T const & pitch, T const & roll) noexcept
		{
			Matrix4_T<T> const rotX(rotation_x(pitch));
			Matrix4_T<T> const rotY(rotation_y(yaw));
			Matrix4_T<T> const rotZ(rotation_z(roll));
			return rotZ * rotX * rotY;
		}

		template float4x4 scaling(float const & sx, float const & sy, float const & sz) noexcept;

		template <typename T>
		Matrix4_T<T> scaling(T const & sx, T const & sy, T const & sz) noexcept
		{
			return Matrix4_T<T>(
				sx,	0,	0,	0,
				0,	sy,	0,	0,
				0,	0,	sz,	0,
				0,	0,	0,	1);
		}

		template float4x4 scaling(float3 const & s) noexcept;

		template <typename T>
		Matrix4_T<T> scaling(Vector_T<T, 3> const & s) noexcept
		{
			return scaling(s.x(), s.y(), s.z());
		}

		template float4x4 shadow(float4 const & l, Plane const & p) noexcept;

		template <typename T>
		Matrix4_T<T> shadow(Vector_T<T, 4> const & l, Plane_T<T> const & p) noexcept
		{
			Vector_T<T, 4> const v(-l);
			Plane_T<T> P(normalize(p));
			T const d(-dot(P, v));

			return Matrix4_T<T>(
				P.a() * v.x() + d,	P.a() * v.y(),		P.a() * v.z(),		P.a() * v.w(),
				P.b() * v.x(),		P.b() * v.y() + d,	P.b() * v.z(),		P.b() * v.w(),
				P.c() * v.x(),		P.c() * v.y(),		P.c() * v.z() + d,	P.c() * v.w(),
				P.d() * v.x(),		P.d() * v.y(),		P.d() * v.z(),		P.d() * v.w() + d);
		}

		template float4x4 to_matrix(Quaternion const & quat) noexcept;

		template <typename T>
		Matrix4_T<T> to_matrix(Quaternion_T<T> const & quat) noexcept
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

		template float4x4 translation(float const & x, float const & y, float const & z) noexcept;

		template <typename T>
		Matrix4_T<T> translation(T const & x, T const & y, T const & z) noexcept
		{
			return Matrix4_T<T>(
				1,	0,	0,	0,
				0,	1,	0,	0,
				0,	0,	1,	0,
				x,	y,	z,	1);
		}

		template float4x4 translation(float3 const & pos) noexcept;

		template <typename T>
		Matrix4_T<T> translation(Vector_T<T, 3> const & pos) noexcept
		{
			return translation(pos.x(), pos.y(), pos.z());
		}

		template float4x4 transpose(float4x4 const & rhs) noexcept;

		template <typename T>
		Matrix4_T<T> transpose(Matrix4_T<T> const & rhs) noexcept
		{
			return Matrix4_T<T>(
				rhs(0, 0), rhs(1, 0), rhs(2, 0), rhs(3, 0),
				rhs(0, 1), rhs(1, 1), rhs(2, 1), rhs(3, 1),
				rhs(0, 2), rhs(1, 2), rhs(2, 2), rhs(3, 2),
				rhs(0, 3), rhs(1, 3), rhs(2, 3), rhs(3, 3));
		}

		template float4x4 lh_to_rh(float4x4 const & rhs) noexcept;

		template <typename T>
		Matrix4_T<T> lh_to_rh(Matrix4_T<T> const & rhs) noexcept
		{
			Matrix4_T<T> ret = rhs;
			ret(2, 0) = -ret(2, 0);
			ret(2, 1) = -ret(2, 1);
			ret(2, 2) = -ret(2, 2);
			ret(2, 3) = -ret(2, 3);
			return ret;
		}

		template void decompose(float3& scale, Quaternion& rot, float3& trans, float4x4 const & rhs) noexcept;

		template <typename T>
		void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, Matrix4_T<T> const & rhs) noexcept
		{
			scale.x() = sqrt(rhs(0, 0) * rhs(0, 0) + rhs(0, 1) * rhs(0, 1) + rhs(0, 2) * rhs(0, 2));
			scale.y() = sqrt(rhs(1, 0) * rhs(1, 0) + rhs(1, 1) * rhs(1, 1) + rhs(1, 2) * rhs(1, 2));
			scale.z() = sqrt(rhs(2, 0) * rhs(2, 0) + rhs(2, 1) * rhs(2, 1) + rhs(2, 2) * rhs(2, 2));

			trans = Vector_T<T, 3>(rhs(3, 0), rhs(3, 1), rhs(3, 2));

			Matrix4_T<T> rot_mat;
			rot_mat(0, 0) = rhs(0, 0) / scale.x();
			rot_mat(0, 1) = rhs(0, 1) / scale.x();
			rot_mat(0, 2) = rhs(0, 2) / scale.x();
			rot_mat(0, 3) = 0;
			rot_mat(1, 0) = rhs(1, 0) / scale.y();
			rot_mat(1, 1) = rhs(1, 1) / scale.y();
			rot_mat(1, 2) = rhs(1, 2) / scale.y();
			rot_mat(1, 3) = 0;
			rot_mat(2, 0) = rhs(2, 0) / scale.z();
			rot_mat(2, 1) = rhs(2, 1) / scale.z();
			rot_mat(2, 2) = rhs(2, 2) / scale.z();
			rot_mat(2, 3) = 0;
			rot_mat(3, 0) = 0;
			rot_mat(3, 1) = 0;
			rot_mat(3, 2) = 0;
			rot_mat(3, 3) = 1;
			rot = to_quaternion(rot_mat);
		}

		template float4x4 transformation(float3 const * scaling_center, Quaternion const * scaling_rotation, float3 const * scale,
			float3 const * rotation_center, Quaternion const * rotation, float3 const * trans) noexcept;

		template <typename T>
		Matrix4_T<T> transformation(Vector_T<T, 3> const * scaling_center, Quaternion_T<T> const * scaling_rotation, Vector_T<T, 3> const * scale,
			Vector_T<T, 3> const * rotation_center, Quaternion_T<T> const * rotation, Vector_T<T, 3> const * trans) noexcept
		{
			Vector_T<T, 3> psc, prc, pt;
			if (scaling_center)
			{
				psc = *scaling_center;
			}
			else
			{
				psc = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			if (rotation_center)
			{
				prc = *rotation_center;
			}
			else
			{
				prc = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			if (trans)
			{
				pt = *trans;
			}
			else
			{
				pt = Vector_T<T, 3>(T(0), T(0), T(0));
			}

			Matrix4_T<T> m1, m2, m3, m4, m5, m6, m7;
			m1 = translation(-psc);
			if (scaling_rotation)
			{
				m4 = to_matrix(*scaling_rotation);
				m2 = inverse(m4);
			}
			else
			{
				m2 = m4 = Matrix4_T<T>::Identity();
			}
			if (scale)
			{
				m3 = scaling(*scale);
			}
			else
			{
				m3 = Matrix4_T<T>::Identity();
			}
			if (rotation)
			{
				m6 = to_matrix(*rotation);
			}
			else
			{
				m6 = Matrix4_T<T>::Identity();
			}
			m5 = translation(psc - prc);
			m7 = translation(prc + pt);

			return m1 * m2 * m3 * m4 * m5 * m6 * m7;
		}

		template float4x4 ortho_rh(float const & width, float const & height, float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> ortho_rh(T const & width, T const & height, T const & nearPlane, T const & farPlane) noexcept
		{
			return lh_to_rh(ortho_lh(width, height, nearPlane, farPlane));
		}

		template float4x4 ortho_off_center_rh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane) noexcept;
		
		template <typename T>
		Matrix4_T<T> ortho_off_center_rh(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane) noexcept
		{
			return lh_to_rh(ortho_off_center_lh(left, right, bottom, top, nearPlane, farPlane));
		}

		template float4x4 perspective_rh(float const & width, float const & height,
			float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> perspective_rh(T const & width, T const & height,
			T const & nearPlane, T const & farPlane) noexcept
		{
			return lh_to_rh(perspective_lh(width, height, nearPlane, farPlane));
		}

		template float4x4 perspective_fov_rh(float const & fov, float const & aspect,
			float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> perspective_fov_rh(T const & fov, T const & aspect,
			T const & nearPlane, T const & farPlane) noexcept
		{
			return lh_to_rh(perspective_fov_lh(fov, aspect, nearPlane, farPlane));
		}

		template float4x4 perspective_off_center_rh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane) noexcept;

		template <typename T>
		Matrix4_T<T> perspective_off_center_rh(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane) noexcept
		{
			return lh_to_rh(perspective_off_center_lh(left, right, bottom, top, nearPlane, farPlane));
		}

		template float4x4 rh_to_lh(float4x4 const & rhs) noexcept;

		template <typename T>
		Matrix4_T<T> rh_to_lh(Matrix4_T<T> const & rhs) noexcept
		{
			return lh_to_rh(rhs);
		}

		template float4x4 rotation_matrix_yaw_pitch_roll(float3 const & ang) noexcept;

		template <typename T>
		Matrix4_T<T> rotation_matrix_yaw_pitch_roll(Vector_T<T, 3> const & ang) noexcept
		{
			return rotation_matrix_yaw_pitch_roll(ang.x(), ang.y(), ang.z());
		}


		template Quaternion conjugate(Quaternion const & rhs) noexcept;

		template <typename T>
		Quaternion_T<T> conjugate(Quaternion_T<T> const & rhs) noexcept
		{
			return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
		}

		template Quaternion axis_to_axis(float3 const & from, float3 const & to) noexcept;

		template <typename T>
		Quaternion_T<T> axis_to_axis(Vector_T<T, 3> const & from, Vector_T<T, 3> const & to) noexcept
		{
			Vector_T<T, 3> const a(normalize(from));
			Vector_T<T, 3> const b(normalize(to));

			return unit_axis_to_unit_axis(a, b);
		}

		template Quaternion unit_axis_to_unit_axis(float3 const & from, float3 const & to) noexcept;

		template <typename T>
		Quaternion_T<T> unit_axis_to_unit_axis(Vector_T<T, 3> const & from, Vector_T<T, 3> const & to) noexcept
		{
			T const cos_theta = dot(from, to);
			if (equal(cos_theta, T(1)))
			{
				return Quaternion_T<T>::Identity();
			}
			else
			{
				if (equal(cos_theta, T(-1)))
				{
					return Quaternion_T<T>(1, 0, 0, 0);
				}
				else
				{
					// From http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors

					Vector_T<T, 3> w = cross(from, to);
					return normalize(Quaternion_T<T>(w.x(), w.y(), w.z(), 1 + cos_theta));
				}
			}
		}

		template Quaternion bary_centric(Quaternion const & q1, Quaternion const & q2,
			Quaternion const & q3, Quaternion::value_type const & f, Quaternion::value_type const & g) noexcept;

		template <typename T>
		Quaternion_T<T> bary_centric(Quaternion_T<T> const & q1, Quaternion_T<T> const & q2,
			Quaternion_T<T> const & q3, T f, T g) noexcept
		{
			Quaternion_T<T> ret;
			T const s = f + g;
			if (s != T(0))
			{
				ret = slerp(slerp(q1, q2, s), slerp(q1, q3, s), g / s);
			}
			else
			{
				ret = q1;
			}

			return ret;
		}

		template Quaternion exp(Quaternion const & rhs) noexcept;

		template <typename T>
		Quaternion_T<T> exp(Quaternion_T<T> const & rhs) noexcept
		{
			T const theta(length(rhs.v()));
			return Quaternion_T<T>(normalize(rhs.v()) * sin(theta), cos(theta));
		}

		template Quaternion ln(Quaternion const & rhs) noexcept;

		template <typename T>
		Quaternion_T<T> ln(Quaternion_T<T> const & rhs) noexcept
		{
			T const theta_2(acos(rhs.w()));
			return Quaternion_T<T>(normalize(rhs.v()) * (theta_2 + theta_2), 0);
		}

		template Quaternion inverse(Quaternion const & rhs) noexcept;

		template <typename T>
		Quaternion_T<T> inverse(Quaternion_T<T> const & rhs) noexcept
		{
			T const inv(T(1) / length(rhs));
			return Quaternion(-rhs.x() * inv, -rhs.y() * inv, -rhs.z() * inv, rhs.w() * inv);
		}

		template Quaternion mul(Quaternion const & lhs, Quaternion const & rhs) noexcept;

		template <typename T>
		Quaternion_T<T> mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs) noexcept
		{
			return Quaternion_T<T>(
				lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
				lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
				lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
				lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
		}

		template Quaternion rotation_quat_yaw_pitch_roll(float const & yaw, float const & pitch, float const & roll) noexcept;

		template <typename T>
		Quaternion_T<T> rotation_quat_yaw_pitch_roll(T const & yaw, T const & pitch, T const & roll) noexcept
		{
			T const angX(pitch / 2), angY(yaw / 2), angZ(roll / 2);
			T sx, sy, sz;
			T cx, cy, cz;
			sincos(angX, sx, cx);
			sincos(angY, sy, cy);
			sincos(angZ, sz, cz);

			return Quaternion_T<T>(
				sx * cy * cz + cx * sy * sz,
				cx * sy * cz - sx * cy * sz,
				cx * cy * sz - sx * sy * cz,
				sx * sy * sz + cx * cy * cz);
		}

		template void to_yaw_pitch_roll(float& yaw, float& pitch, float& roll, Quaternion const & quat) noexcept;

		// From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
		template <typename T>
		void to_yaw_pitch_roll(T& yaw, T& pitch, T& roll, Quaternion_T<T> const & quat) noexcept
		{
			T sqx = quat.x() * quat.x();
			T sqy = quat.y() * quat.y();
			T sqz = quat.z() * quat.z();
			T sqw = quat.w() * quat.w();
			T unit = sqx + sqy + sqz + sqw;
			T test = quat.w() * quat.x() + quat.y() * quat.z();
			if (test > T(0.499) * unit)
			{
				// singularity at north pole
				yaw = 2 * atan2(quat.z(), quat.w());
				pitch = PI / 2;
				roll = 0;
			}
			else
			{
				if (test < -T(0.499) * unit)
				{
					// singularity at south pole
					yaw = -2 * atan2(quat.z(), quat.w());
					pitch = -PI / 2;
					roll = 0;
				}
				else
				{
					yaw = atan2(2 * (quat.y() * quat.w() - quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
					pitch = asin(2 * test / unit);
					roll = atan2(2 * (quat.z() * quat.w() - quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
				}
			}
		}

		template void to_axis_angle(float3& vec, float& ang, Quaternion const & quat) noexcept;

		template <typename T>
		void to_axis_angle(Vector_T<T, 3>& vec, T& ang, Quaternion_T<T> const & quat) noexcept
		{
			T const tw(acos(quat.w()));
			T const stw = sin(tw);

			ang = tw + tw;
			vec = quat.v();
			if (!equal<T>(stw, 0))
			{
				vec /= stw;
			}
		}

		template Quaternion to_quaternion(float4x4 const & mat) noexcept;

		template <typename T>
		Quaternion_T<T> to_quaternion(Matrix4_T<T> const & mat) noexcept
		{
			Quaternion_T<T> quat;
			T s;
			T const tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

			// check the diagonal
			if (tr > 1)
			{
				s = sqrt(tr);
				quat.w() = s * T(0.5);
				s = T(0.5) / s;
				quat.x() = (mat(1, 2) - mat(2, 1)) * s;
				quat.y() = (mat(2, 0) - mat(0, 2)) * s;
				quat.z() = (mat(0, 1) - mat(1, 0)) * s;
			}
			else
			{
				int maxi = 0;
				T maxdiag = mat(0, 0);
				for (int i = 1; i < 3; ++ i)
				{
					if (mat(i, i) > maxdiag)
					{
						maxi = i;
						maxdiag = mat(i, i);
					}
				}

				switch (maxi)
				{
				case 0:
					s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

					quat.x() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(1, 2) - mat(2, 1)) * s;
					quat.y() = (mat(1, 0) + mat(0, 1)) * s;
					quat.z() = (mat(2, 0) + mat(0, 2)) * s;
					break;

				case 1:
					s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
					quat.y() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(2, 0) - mat(0, 2)) * s;
					quat.z() = (mat(2, 1) + mat(1, 2)) * s;
					quat.x() = (mat(0, 1) + mat(1, 0)) * s;
					break;

				case 2:
				default:
					s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

					quat.z() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(0, 1) - mat(1, 0)) * s;
					quat.x() = (mat(0, 2) + mat(2, 0)) * s;
					quat.y() = (mat(1, 2) + mat(2, 1)) * s;
					break;
				}
			}

			return normalize(quat);
		}

		template Quaternion to_quaternion(float3 const & tangent, float3 const & binormal, float3 const & normal, int bits) noexcept;

		template <typename T>
		Quaternion_T<T> to_quaternion(Vector_T<T, 3> const & tangent, Vector_T<T, 3> const & binormal, Vector_T<T, 3> const & normal, int bits) noexcept
		{
			T k = 1;
			if (dot(binormal, cross(normal, tangent)) < 0)
			{
				k = -1;
			}

			Matrix4_T<T> tangent_frame(tangent.x(), tangent.y(), tangent.z(), 0,
				k * binormal.x(), k * binormal.y(), k * binormal.z(), 0,
				normal.x(), normal.y(), normal.z(), 0,
				0, 0, 0, 1);
			Quaternion_T<T> tangent_quat = to_quaternion(tangent_frame);
			if (tangent_quat.w() < 0)
			{
				tangent_quat = -tangent_quat;
			}
			if (bits > 0)
			{
				T const bias = T(1) / ((1UL << (bits - 1)) - 1);
				if (tangent_quat.w() < bias)
				{
					T const factor = sqrt(1 - bias * bias);
					tangent_quat.x() *= factor;
					tangent_quat.y() *= factor;
					tangent_quat.z() *= factor;
					tangent_quat.w() = bias;
				}
			}
			if (k < 0)
			{
				tangent_quat = -tangent_quat;
			}

			return tangent_quat;
		}

		template Quaternion rotation_axis(float3 const & v, float const & angle) noexcept;

		template <typename T>
		Quaternion_T<T> rotation_axis(Vector_T<T, 3> const & v, T const & angle) noexcept
		{
			T sa, ca;
			sincos(angle * T(0.5), sa, ca);

			if (equal<T>(length_sq(v), 0))
			{
				return Quaternion_T<T>(sa, sa, sa, ca);
			}
			else
			{
				return Quaternion_T<T>(sa * normalize(v), ca);
			}
		}

		template Quaternion slerp(Quaternion const & lhs, Quaternion const & rhs, float s) noexcept;

		template <typename T>
		Quaternion_T<T> slerp(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs, T s) noexcept
		{
			T scale0, scale1;

			// DOT the quats to get the cosine of the angle between them
			T cosom = dot(lhs, rhs);

			T dir = T(1);
			if (cosom < 0)
			{
				dir = T(-1);
				cosom = -cosom;
			}

			// make sure they are different enough to avoid a divide by 0
			if (cosom < T(1) - std::numeric_limits<T>::epsilon())
			{
				// SLERP away
				T const omega = acos(cosom);
				T const isinom = T(1) / sin(omega);
				scale0 = sin((T(1) - s) * omega) * isinom;
				scale1 = sin(s * omega) * isinom;
			}
			else
			{
				// LERP is good enough at this distance
				scale0 = T(1) - s;
				scale1 = s;
			}

			// Compute the result
			return scale0 * lhs + dir * scale1 * rhs;
		}

		template void squad_setup(Quaternion& a, Quaternion& b, Quaternion& c,
			Quaternion const & q0, Quaternion const & q1, Quaternion const & q2,
			Quaternion const & q3) noexcept;

		template <typename T>
		void squad_setup(Quaternion_T<T>& a, Quaternion_T<T>& b, Quaternion_T<T>& c,
			Quaternion_T<T> const & q0, Quaternion_T<T> const & q1, Quaternion_T<T> const & q2,
			Quaternion_T<T> const & q3) noexcept
		{
			Quaternion_T<T> q, temp1, temp2, temp3;

			if (dot(q0, q1) < 0)
			{
				temp2 = -q0;
			}
			else
			{
				temp2 = q0;
			}

			if (dot(q1, q2) < 0)
			{
				c = -q2;
			}
			else
			{
				c = q2;
			}

			if (dot(c, q3) < 0)
			{
				temp3 = -q3;
			}
			else
			{
				temp3 = q3;
			}

			temp1 = inverse(q1);
			temp2 = ln(mul(temp1, temp2));
			q = ln(mul(temp1, c));
			temp1 = temp2 + q;
			temp1 = exp(temp1 * T(-0.25));
			a = mul(q1, temp1);

			temp1 = inverse(c);
			temp2 = ln(mul(temp1, q1));
			q = ln(mul(temp1, temp3));
			temp1 = temp2 + q;
			temp1 = exp(temp1 * T(-0.25));
			b = mul(c, temp1);
		}

		template Quaternion squad(Quaternion const & q1, Quaternion const & a, Quaternion const & b,
			Quaternion const & c, float t) noexcept;

		template <typename T>
		Quaternion_T<T> squad(Quaternion_T<T> const & q1, Quaternion_T<T> const & a, Quaternion_T<T> const & b,
			Quaternion_T<T> const & c, float t) noexcept
		{
			return slerp(slerp(q1, c, t), slerp(a, b, t), T(2) * t * (T(1) - t));
		}

		template Quaternion rotation_quat_yaw_pitch_roll(float3 const & ang) noexcept;

		template <typename T>
		Quaternion_T<T> rotation_quat_yaw_pitch_roll(Vector_T<T, 3> const & ang) noexcept
		{
			return rotation_quat_yaw_pitch_roll(ang.x(), ang.y(), ang.z());
		}

		template float dot(Plane const & lhs, float4 const & rhs) noexcept;

		template <typename T>
		T dot(Plane_T<T> const & lhs, Vector_T<T, 4> const & rhs) noexcept
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d() * rhs.w();
		}

		template float dot_coord(Plane const & lhs, float3 const & rhs) noexcept;

		template <typename T>
		T dot_coord(Plane_T<T> const & lhs, Vector_T<T, 3> const & rhs) noexcept
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d();
		}

		template float dot_normal(Plane const & lhs, float3 const & rhs) noexcept;

		template <typename T>
		T dot_normal(Plane_T<T> const & lhs, Vector_T<T, 3> const & rhs) noexcept
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z();
		}

		template Plane from_point_normal(float3 const & point, float3 const & normal) noexcept;

		template <typename T>
		Plane_T<T> from_point_normal(Vector_T<T, 3> const & point, Vector_T<T, 3> const & normal) noexcept
		{
			return Plane_T<T>(normal.x(), normal.y(), normal.z(), -dot(point, normal));
		}

		template Plane from_points(float3 const & v0, float3 const & v1, float3 const & v2) noexcept;

		template <typename T>
		Plane_T<T> from_points(Vector_T<T, 3> const & v0, Vector_T<T, 3> const & v1, Vector_T<T, 3> const & v2) noexcept
		{
			Vector_T<T, 3> const vec(cross(v1 - v0, v2 - v0));
			return from_point_normal(v0, normalize(vec));
		}

		template Plane mul(Plane const & p, float4x4 const & mat) noexcept;

		template <typename T>
		Plane_T<T> mul(Plane_T<T> const & p, Matrix4_T<T> const & mat) noexcept
		{
			return Plane_T<T>(
				p.a() * mat(0, 0) + p.b() * mat(1, 0) + p.c() * mat(2, 0) + p.d() * mat(3, 0),
				p.a() * mat(0, 1) + p.b() * mat(1, 1) + p.c() * mat(2, 1) + p.d() * mat(3, 1),
				p.a() * mat(0, 2) + p.b() * mat(1, 2) + p.c() * mat(2, 2) + p.d() * mat(3, 2),
				p.a() * mat(0, 3) + p.b() * mat(1, 3) + p.c() * mat(2, 3) + p.d() * mat(3, 3));
		}

		template float intersect_ray(Plane const & p, float3 const & orig, float3 const & dir) noexcept;

		// 求直线和平面的交点，直线orig + t * dir
		template <typename T>
		T intersect_ray(Plane_T<T> const & p, Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir) noexcept
		{
			T deno(dot(dir, p.Normal()));
			if (equal(deno, T(0)))
			{
				deno = T(0.0001);
			}

			return -dot_coord(p, orig) / deno;
		}


		template void oblique_clipping(float4x4& proj, Plane const & clip_plane) noexcept;
		
		// From Game Programming Gems 5, Section 2.6.
		template <typename T>
		void oblique_clipping(Matrix4_T<T>& proj, Plane_T<T> const & clip_plane) noexcept
		{
			Vector_T<T, 4> q;
			q.x() = (sgn(clip_plane.a()) - proj(2, 0)) / proj(0, 0);
			q.y() = (sgn(clip_plane.b()) - proj(2, 1)) / proj(1, 1);
			q.z() = T(1);
			q.w() = (T(1) - proj(2, 2)) / proj(3, 2);

			T c = T(1) / dot(clip_plane, q);

			proj(0, 2) = clip_plane.a() * c;
			proj(1, 2) = clip_plane.b() * c;
			proj(2, 2) = clip_plane.c() * c;
			proj(3, 2) = clip_plane.d() * c;
		}


		template Color negative(Color const & rhs) noexcept;

		template <typename T>
		Color_T<T> negative(Color_T<T> const & rhs) noexcept
		{
			return Color_T<T>(1 - rhs.r(), 1 - rhs.g(), 1 - rhs.b(), rhs.a());
		}

		template Color modulate(Color const & lhs, Color const & rhs) noexcept;

		template <typename T>
		Color_T<T> modulate(Color_T<T> const & lhs, Color_T<T> const & rhs) noexcept
		{
			return Color_T<T>(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
		}


		template AABBox compute_aabbox(float3* first, float3* last) noexcept;
		template AABBox compute_aabbox(float4* first, float4* last) noexcept;
		template AABBox compute_aabbox(float3 const * first, float3 const * last) noexcept;
		template AABBox compute_aabbox(float4 const * first, float4 const * last) noexcept;
		template AABBox compute_aabbox(std::vector<float3>::iterator first, std::vector<float3>::iterator last) noexcept;
		template AABBox compute_aabbox(std::vector<float4>::iterator first, std::vector<float4>::iterator last) noexcept;
		template AABBox compute_aabbox(std::vector<float3>::const_iterator first, std::vector<float3>::const_iterator last) noexcept;
		template AABBox compute_aabbox(std::vector<float4>::const_iterator first, std::vector<float4>::const_iterator last) noexcept;

		template <typename Iterator>
		AABBox_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_aabbox(Iterator first, Iterator last) noexcept
		{
			typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

			Vector_T<value_type, 3> minVec = *first;
			Vector_T<value_type, 3> maxVec = *first;
			Iterator iter = first;
			++ iter;
			for (; iter != last; ++ iter)
			{
				Vector_T<value_type, 3> const & v = *iter;
				minVec = minimize(minVec, v);
				maxVec = maximize(maxVec, v);
			}
			return AABBox_T<value_type>(minVec, maxVec);
		}

		template OBBox compute_obbox(float3* first, float3* last) noexcept;
		template OBBox compute_obbox(float4* first, float4* last) noexcept;
		template OBBox compute_obbox(float3 const * first, float3 const * last) noexcept;
		template OBBox compute_obbox(float4 const * first, float4 const * last) noexcept;
		template OBBox compute_obbox(std::vector<float3>::iterator first, std::vector<float3>::iterator last) noexcept;
		template OBBox compute_obbox(std::vector<float4>::iterator first, std::vector<float4>::iterator last) noexcept;
		template OBBox compute_obbox(std::vector<float3>::const_iterator first, std::vector<float3>::const_iterator last) noexcept;
		template OBBox compute_obbox(std::vector<float4>::const_iterator first, std::vector<float4>::const_iterator last) noexcept;

		template <typename Iterator>
		OBBox_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_obbox(Iterator first, Iterator last) noexcept
		{
			typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

			// Compute the mean of the points.
			Vector_T<value_type, 3> center = *first;
			Iterator iter = first;
			++ iter;
			uint32_t n = 1;
			for (; iter != last; ++ iter, ++ n)
			{
				center += Vector_T<value_type, 3>(*iter);
			}
			value_type inv_num_points = value_type(1) / n;
			center *= inv_num_points;

			// Compute the covariance matrix of the points.
			value_type cov[6];
			for (int i = 0; i < 6; ++ i)
			{
				cov[i] = 0;
			}

			for (iter = first; iter != last; ++ iter)
			{
				Vector_T<value_type, 3> diff = Vector_T<value_type, 3>(*iter) - center;
				cov[0] += diff[0] * diff[0];
				cov[1] += diff[0] * diff[1];
				cov[2] += diff[0] * diff[2];
				cov[3] += diff[1] * diff[1];
				cov[4] += diff[1] * diff[2];
				cov[5] += diff[2] * diff[2];
			}

			for (int i = 0; i < 6; ++ i)
			{
				cov[i] *= inv_num_points;
			}

			// Tridiagonal

			value_type diagonal[3];
			value_type sub_diagonal[3];
			value_type matrix[3][3];
			bool is_rotation = false;

			value_type m00 = cov[0];
			value_type m01 = cov[1];
			value_type m02 = cov[2];
			value_type m11 = cov[3];
			value_type m12 = cov[4];
			value_type m22 = cov[5];

			diagonal[0] = m00;
			sub_diagonal[2] = 0;
			if (abs(m02) > value_type(1e-6))
			{
				value_type length = sqrt(m01 * m01 + m02 * m02);
				value_type inv_length = 1 / length;
				m01 *= inv_length;
				m02 *= inv_length;
				value_type q = 2 * m01 * m12 + m02 * (m22 - m11);
				diagonal[1] = m11 + m02 * q;
				diagonal[2] = m22 - m02 * q;
				sub_diagonal[0] = length;
				sub_diagonal[1] = m12 - m01 * q;
				matrix[0][0] = 1;
				matrix[0][1] = 0;
				matrix[0][2] = 0;
				matrix[1][0] = 0;
				matrix[1][1] = m01;
				matrix[1][2] = m02;
				matrix[2][0] = 0;
				matrix[2][1] = m02;
				matrix[2][2] = -m01;
				is_rotation = false;
			}
			else
			{
				diagonal[1] = m11;
				diagonal[2] = m22;
				sub_diagonal[0] = m01;
				sub_diagonal[1] = m12;
				matrix[0][0] = 1;
				matrix[0][1] = 0;
				matrix[0][2] = 0;
				matrix[1][0] = 0;
				matrix[1][1] = 1;
				matrix[1][2] = 0;
				matrix[2][0] = 0;
				matrix[2][1] = 0;
				matrix[2][2] = 1;
				is_rotation = true;
			}

			// QLAlgorithm

			int const nIterPower = 32;

			for (int i0 = 0; i0 < 3; ++ i0)
			{
				for (int i1 = 0; i1 < nIterPower; ++ i1)
				{
					int i2;
					for (i2 = i0; i2 <= 3 - 2; ++ i2)
					{
						value_type const tmp = abs(diagonal[i2]) + abs(diagonal[i2+1]);

						if (abs(sub_diagonal[i2]) + tmp == tmp)
						{
							break;
						}
					}
					if (i2 == i0)
					{
						break;
					}

					value_type value0 = (diagonal[i0 + 1] - diagonal[i0]) / (2 * sub_diagonal[i0]);
					value_type value1 = sqrt(value0 * value0 + 1);
					if (value0 < 0)
					{
						value0 = diagonal[i2] - diagonal[i0] + sub_diagonal[i0] / (value0 - value1);
					}
					else
					{
						value0 = diagonal[i2] - diagonal[i0] + sub_diagonal[i0] / (value0 + value1);
					}

					value_type sn = 1, cs = 1, value2 = 0;
					for (int i3 = i2 - 1; i3 >= i0; -- i3)
					{
						value_type value3 = sn * sub_diagonal[i3];
						value_type value4 = cs * sub_diagonal[i3];
						if (abs(value3) >= abs(value0))
						{
							cs = value0 / value3;
							value1 = sqrt(cs * cs + 1);
							sub_diagonal[i3 + 1] = value3 * value1;
							sn = 1 / value1;
							cs *= sn;
						}
						else
						{
							sn = value3 / value0;
							value1 = sqrt(sn * sn + 1);
							sub_diagonal[i3 + 1] = value0 * value1;
							cs = 1 / value1;
							sn *= cs;
						}
						value0 = diagonal[i3 + 1] - value2;
						value1 = (diagonal[i3] - value0) * sn + 2 * value4 * cs;
						value2 = sn * value1;
						diagonal[i3 + 1] = value0 + value2;
						value0 = cs * value1 - value4;

						for (int i4 = 0; i4 < 3; ++ i4)
						{
							value3 = matrix[i4][i3 + 1];
							matrix[i4][i3 + 1] = sn * matrix[i4][i3] + cs * value3;
							matrix[i4][i3] = cs * matrix[i4][i3] - sn * value3;
						}
					}
					diagonal[i0] -= value2;
					sub_diagonal[i0] = value0;
					sub_diagonal[i2] = 0;
				}
			}

			// IncreasingSort

			// Sort the eigenvalues in increasing order, e[0] <= ... <= e[mSize-1]
			for (int i0 = 0; i0 <= 3 - 2; ++ i0)
			{ 
				// Locate the minimum eigenvalue.
				int i1 = i0;
				float min_value = diagonal[i1];
				for (int i2 = i0 + 1; i2 < 3; ++ i2)
				{
					if (diagonal[i2] < min_value)
					{
						i1 = i2;
						min_value = diagonal[i1];
					}
				}

				if (i1 != i0)
				{
					// Swap the eigenvalues.
					diagonal[i1] = diagonal[i0];
					diagonal[i0] = min_value;

					// Swap the eigenvectors corresponding to the eigenvalues.
					for (int i2 = 0; i2 < 3; ++ i2)
					{
						value_type tmp = matrix[i2][i0];
						matrix[i2][i0] = matrix[i2][i1];
						matrix[i2][i1] = tmp;
						is_rotation = !is_rotation;
					}
				}
			}

			// GuaranteeRotation

			if (!is_rotation)
			{
				// Change sign on the first column.
				for (int row = 0; row < 3; ++ row)
				{
					matrix[row][0] = -matrix[row][0];
				}
			}

			Vector_T<value_type, 3> axis[3];
			Vector_T<value_type, 3> extent;
			for (int i = 0; i < 3; ++ i)
			{
				extent[i] = diagonal[i];
				for (int row = 0; row < 3; ++row)
				{
					axis[i][row] = matrix[row][i];
				}
			}

			// Let C be the box center and let U0, U1, and U2 be the box axes.  Each
			// input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.  The
			// following code computes min(y0), max(y0), min(y1), max(y1), min(y2),
			// and max(y2).  The box center is then adjusted to be
			//   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1 +
			//        0.5*(min(y2)+max(y2))*U2

			Vector_T<value_type, 3> diff = Vector_T<value_type, 3>(*first) - center;
			Vector_T<value_type, 3> pmin(dot(diff, axis[0]), dot(diff, axis[1]), dot(diff, axis[2]));
			Vector_T<value_type, 3> pmax = pmin;
			iter = first;
			++ iter;
			for (; iter != last; ++ iter)
			{
				diff = Vector_T<value_type, 3>(*iter) - center;
				for (int j = 0; j < 3; ++ j)
				{
					float d = dot(diff, axis[j]);
					if (d < pmin[j])
					{
						pmin[j] = d;
					}
					else if (d > pmax[j])
					{
						pmax[j] = d;
					}
				}
			}

			center += (value_type(0.5) * (pmin[0] + pmax[0])) * axis[0]
				+ (value_type(0.5) * (pmin[1] + pmax[1])) * axis[1]
				+ (value_type(0.5) * (pmin[2] + pmax[2])) * axis[2];

			extent[0] = value_type(0.5) * (pmax[0] - pmin[0]);
			extent[1] = value_type(0.5) * (pmax[1] - pmin[1]);
			extent[2] = value_type(0.5) * (pmax[2] - pmin[2]);

			return OBBox_T<value_type>(center, axis[0], axis[1], axis[2], extent);
		}

		template Sphere compute_sphere(float3* first, float3* last) noexcept;
		template Sphere compute_sphere(float4* first, float4* last) noexcept;
		template Sphere compute_sphere(float3 const * first, float3 const * last) noexcept;
		template Sphere compute_sphere(float4 const * first, float4 const * last) noexcept;
		template Sphere compute_sphere(std::vector<float3>::iterator first, std::vector<float3>::iterator last) noexcept;
		template Sphere compute_sphere(std::vector<float4>::iterator first, std::vector<float4>::iterator last) noexcept;
		template Sphere compute_sphere(std::vector<float3>::const_iterator first, std::vector<float3>::const_iterator last) noexcept;
		template Sphere compute_sphere(std::vector<float4>::const_iterator first, std::vector<float4>::const_iterator last) noexcept;

		template <typename Iterator>
		Sphere_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_sphere(Iterator first, Iterator last) noexcept
		{
			// from Graphics Gems I p301

			typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

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

			value_type x_span = length_sq(x_max - x_min);
			value_type y_span = length_sq(y_max - y_min);
			value_type z_span = length_sq(z_max - z_min);

			Vector_T<value_type, 3> dia1 = x_min;
			Vector_T<value_type, 3> dia2 = x_max;
			value_type max_span = x_span;
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

			Vector_T<value_type, 3> center((dia1 + dia2) / value_type(2));
			value_type r = length(dia2 - center);

			for (Iterator iter = first; iter != last; ++ iter)
			{
				value_type d = length(Vector_T<value_type, 3>(*iter) - center);

				if (d > r)
				{
					r = (d + r) / 2;
					center = (r * center + (d - r) * Vector_T<value_type, 3>(*iter)) / d;
				}
			}

			return Sphere_T<value_type>(center, r);
		}


		template AABBox convert_to_aabbox(OBBox const & obb) noexcept;

		template <typename T>
		AABBox_T<T> convert_to_aabbox(OBBox_T<T> const & obb) noexcept
		{
			Vector_T<T, 3> min(+1e10f, +1e10f, +1e10f);
			Vector_T<T, 3> max(-1e10f, -1e10f, -1e10f);

			Vector_T<T, 3> const & center = obb.Center();
			Vector_T<T, 3> const & extent = obb.HalfSize();
			Vector_T<T, 3> const extent_x = extent.x() * obb.Axis(0);
			Vector_T<T, 3> const extent_y = extent.y() * obb.Axis(1);
			Vector_T<T, 3> const extent_z = extent.z() * obb.Axis(2);
			for (int i = 0; i < 8; ++ i)
			{
				Vector_T<T, 3> const corner = center + ((i & 1) ? extent_x : -extent_x)
					+ ((i & 2) ? extent_y : -extent_y) + ((i & 4) ? extent_z : -extent_z);

				min = minimize(min, corner);
				max = maximize(max, corner);
			}

			return AABBox_T<T>(min, max);
		}

		template OBBox convert_to_obbox(AABBox const & aabb) noexcept;

		template <typename T>
		OBBox_T<T> convert_to_obbox(AABBox_T<T> const & aabb) noexcept
		{
			return OBBox_T<T>(aabb.Center(), Quaternion_T<T>::Identity(), aabb.HalfSize());
		}


		template AABBox transform_aabb(AABBox const & aabb, float4x4 const & mat) noexcept;

		template <typename T>
		AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Matrix4_T<T> const & mat) noexcept
		{
			Vector_T<T, 3> scale, trans;
			Quaternion_T<T> rot;
			decompose(scale, rot, trans, mat);

			return transform_aabb(aabb, scale, rot, trans);
		}

		template AABBox transform_aabb(AABBox const & aabb, float3 const & scale, Quaternion const & rot, float3 const & trans) noexcept;

		template <typename T>
		AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Vector_T<T, 3> const & scale, Quaternion_T<T> const & rot, Vector_T<T, 3> const & trans) noexcept
		{
			Vector_T<T, 3> min, max;
			min = max = transform_quat(aabb.Corner(0) * scale, rot) + trans;
			for (size_t j = 1; j < 8; ++ j)
			{
				Vector_T<T, 3> const vec = transform_quat(aabb.Corner(j) * scale, rot) + trans;
				min = minimize(min, vec);
				max = maximize(max, vec);
			}

			return AABBox_T<T>(min, max);
		}

		template OBBox transform_obb(OBBox const & obb, float4x4 const & mat) noexcept;

		template <typename T>
		OBBox_T<T> transform_obb(OBBox_T<T> const & obb, Matrix4_T<T> const & mat) noexcept
		{
			Vector_T<T, 3> scale, trans;
			Quaternion_T<T> rot;
			decompose(scale, rot, trans, mat);

			return transform_obb(obb, scale, rot, trans);
		}

		template OBBox transform_obb(OBBox const & obb, float3 const & scale, Quaternion const & rot, float3 const & trans) noexcept;

		template <typename T>
		OBBox_T<T> transform_obb(OBBox_T<T> const & obb, Vector_T<T, 3> const & scale, Quaternion_T<T> const & rot, Vector_T<T, 3> const & trans) noexcept
		{
			Vector_T<T, 3> center = transform_quat(obb.Center() * scale, rot) + trans;
			Quaternion_T<T> rotation = mul(obb.Rotation(), rot);
			Vector_T<T, 3> extent = obb.HalfSize() * scale;
			return OBBox_T<T>(center, rotation, extent);
		}

		template Sphere transform_sphere(Sphere const & sphere, float4x4 const & mat) noexcept;

		template <typename T>
		Sphere_T<T> transform_sphere(Sphere_T<T> const & sphere, Matrix4_T<T> const & mat) noexcept
		{
			Vector_T<T, 3> scale, trans;
			Quaternion_T<T> rot;
			decompose(scale, rot, trans, mat);

			return transform_sphere(sphere, scale.x(), rot, trans);
		}

		template Sphere transform_sphere(Sphere const & sphere, float scale, Quaternion const & rot, float3 const & trans) noexcept;

		template <typename T>
		Sphere_T<T> transform_sphere(Sphere_T<T> const & sphere, T scale, Quaternion_T<T> const & rot, Vector_T<T, 3> const & trans) noexcept
		{
			Vector_T<T, 3> center = transform_quat(sphere.Center() * scale, rot) + trans;
			T radius = sphere.Radius() * scale;
			return Sphere_T<T>(center, radius);
		}

		template Frustum transform_frustum(Frustum const & frustum, float4x4 const & mat) noexcept;

		template <typename T>
		Frustum_T<T> transform_frustum(Frustum_T<T> const & frustum, Matrix4_T<T> const & mat) noexcept
		{
			Frustum_T<T> ret;
			for (int i = 0; i < 6; ++ i)
			{
				ret.FrustumPlane(i, normalize(mul(frustum.FrustumPlane(i), mat)));
			}
			for (int i = 0; i < 8; ++ i)
			{
				ret.Corner(i, transform_coord(frustum.Corner(i), mat));
			}

			return ret;
		}

		template Frustum transform_frustum(Frustum const & frustum, float scale, Quaternion const & rot, float3 const & trans) noexcept;

		template <typename T>
		Frustum_T<T> transform_frustum(Frustum_T<T> const & frustum, T scale, Quaternion_T<T> const & rot, Vector_T<T, 3> const & trans) noexcept
		{
			Vector_T<T, 3> vscale(scale, scale, scale);
			return transform_frustum(frustum, transformation<T>(nullptr, nullptr, &vscale, nullptr, &rot, &trans));
		}


		template bool intersect_point_aabb(float3 const & v, AABBox const & aabb) noexcept;

		template <typename T>
		bool intersect_point_aabb(Vector_T<T, 3> const & v, AABBox_T<T> const & aabb) noexcept
		{
			return (in_bound(v.x(), aabb.Min().x(), aabb.Max().x()))
				&& (in_bound(v.y(), aabb.Min().y(), aabb.Max().y()))
				&& (in_bound(v.z(), aabb.Min().z(), aabb.Max().z()));
		}

		template bool intersect_point_obb(float3 const & v, OBBox const & obb) noexcept;

		template <typename T>
		bool intersect_point_obb(Vector_T<T, 3> const & v, OBBox_T<T> const & obb) noexcept
		{
			Vector_T<T, 3> const d = v - obb.Center();
			return (dot(d, obb.Axis(0)) <= obb.HalfSize().x())
				&& (dot(d, obb.Axis(1)) <= obb.HalfSize().y())
				&& (dot(d, obb.Axis(2)) <= obb.HalfSize().z());
		}

		template bool intersect_point_sphere(float3 const & v, Sphere const & sphere) noexcept;

		template <typename T>
		bool intersect_point_sphere(Vector_T<T, 3> const & v, Sphere_T<T> const & sphere) noexcept
		{
			return length(v - sphere.Center()) < sphere.Radius();
		}

		template bool intersect_point_frustum(float3 const & v, Frustum const & frustum) noexcept;

		template <typename T>
		bool intersect_point_frustum(Vector_T<T, 3> const & v, Frustum_T<T> const & frustum) noexcept
		{
			for (int i = 0; i < 6; ++ i)
			{
				if (dot_coord(frustum.FrustumPlane(i), v) < 0)
				{
					return false;
				}
			}
			return true;
		}


		template bool intersect_ray_aabb(float3 const & orig, float3 const & dir, AABBox const & aabb) noexcept;

		template <typename T>
		bool intersect_ray_aabb(Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir, AABBox_T<T> const & aabb) noexcept
		{
			T t_near = T(-1e10);
			T t_far = T(+1e10);

			for (int i = 0; i < 3; ++ i)
			{
				if (equal(dir[i], T(0)))
				{
					if ((dir[i] < aabb.Min()[i]) || (dir[i] > aabb.Max()[i]))
					{
						return false;
					}
				}
				else
				{
					float t1 = (aabb.Min()[i] - orig[i]) / dir[i];
					float t2 = (aabb.Max()[i] - orig[i]) / dir[i];
					if (t1 > t2)
					{
						std::swap(t1, t2);
					}
					if (t1 > t_near)
					{
						t_near = t1;
					}
					if (t2 < t_far)
					{
						t_far = t2;
					}

					if (t_near > t_far)
					{
						// box is missed
						return false;
					}
					if (t_far < 0)
					{
						// box is behind ray
						return false;
					}
				}
			}

			return true;
		}

		template bool intersect_ray_obb(float3 const & orig, float3 const & dir, OBBox const & obb) noexcept;

		template <typename T>
		bool intersect_ray_obb(Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir, OBBox_T<T> const & obb) noexcept
		{
			T t_near = T(-1e10);
			T t_far = T(+1e10);
			
			Vector_T<T, 3> const p = obb.Center() - orig;
			Vector_T<T, 3> const & extent = obb.HalfSize();
			for (int i = 0; i < 3; ++ i)
			{
				T const e = dot(obb.Axis(i), p);
				T const f = dot(obb.Axis(i), dir);
				if (equal(f, T(0)))
				{
					if ((e < -extent[i]) || (e > extent[i]))
					{
						return false;
					}
				}
				else
				{
					float t1 = (e + extent[i]) / f;
					float t2 = (e - extent[i]) / f;
					if (t1 > t2)
					{
						std::swap(t1, t2);
					}
					if (t1 > t_near)
					{
						t_near = t1;
					}
					if (t2 < t_far)
					{
						t_far = t2;
					}

					if (t_near > t_far)
					{
						// box is missed
						return false;
					}
					if (t_far < 0)
					{
						// box is behind ray
						return false;
					}
				}
			}

			return true;
		}
		
		template bool intersect_ray_sphere(float3 const & orig, float3 const & dir, Sphere const & sphere) noexcept;

		template <typename T>
		bool intersect_ray_sphere(Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir, Sphere_T<T> const & sphere) noexcept
		{
			T const a = length_sq(dir);
			T const b = 2 * dot(dir, orig - sphere.Center());
			T const c = length_sq(orig - sphere.Center()) - sphere.Radius() * sphere.Radius();

			if (b * b - 4 * a * c < 0)
			{
				return false;
			}
			return true;
		}


		template bool intersect_aabb_aabb(AABBox const & lhs, AABBox const & aabb) noexcept;

		template <typename T>
		bool intersect_aabb_aabb(AABBox_T<T> const & lhs, AABBox_T<T> const & aabb) noexcept
		{
			Vector_T<T, 3> const t = aabb.Center() - lhs.Center();
			Vector_T<T, 3> const e = aabb.HalfSize() + lhs.HalfSize();
			return (MathLib::abs(t.x()) <= e.x()) && (MathLib::abs(t.y()) <= e.y()) && (MathLib::abs(t.z()) <= e.z());
		}

		template bool intersect_aabb_obb(AABBox const & lhs, OBBox const & obb) noexcept;

		template <typename T>
		bool intersect_aabb_obb(AABBox_T<T> const & lhs, OBBox_T<T> const & obb) noexcept
		{
			return obb.Intersect(convert_to_obbox(lhs));
		}

		template bool intersect_aabb_sphere(AABBox const & lhs, Sphere const & sphere) noexcept;

		template <typename T>
		bool intersect_aabb_sphere(AABBox_T<T> const & lhs, Sphere_T<T> const & sphere) noexcept
		{
			Vector_T<T, 3> const half_size = lhs.HalfSize();
			Vector_T<T, 3> const d = sphere.Center() - lhs.Center();
			Vector_T<T, 3> closest_point_on_obb = lhs.Center();
			for (int i = 0; i < 3; ++ i)
			{
				Vector_T<T, 3> axis(0, 0, 0);
				axis[i] = 1;
				T dist = dot(d, axis);
				if (dist > half_size[i])
				{
					dist = half_size[i];
				}
				if (dist < -half_size[i])
				{
					dist = -half_size[i];
				}
				closest_point_on_obb += dist * axis;
			}

			Vector_T<T, 3> const v = closest_point_on_obb - sphere.Center();
			return length_sq(v) <= sphere.Radius() * sphere.Radius();
		}

		template bool intersect_obb_obb(OBBox const & lhs, OBBox const & obb) noexcept;

		template <typename T>
		bool intersect_obb_obb(OBBox_T<T> const & lhs, OBBox_T<T> const & obb) noexcept
		{
			// From Real-Time Collision Detection, p. 101-106. See http://realtimecollisiondetection.net/

			T epsilon = T(1e-3);

			Matrix4_T<T> r_mat = Matrix4_T<T>::Identity();
			for (int i = 0; i < 3; ++ i)
			{
				for (int j = 0; j < 3; ++ j)
				{
					r_mat(i, j) = dot(lhs.Axis(i), obb.Axis(j));
				}
			}

			Vector_T<T, 3> t = obb.Center() - lhs.Center();
			t = Vector_T<T, 3>(dot(t, lhs.Axis(0)), dot(t, lhs.Axis(1)), dot(t, lhs.Axis(2)));

			Matrix4_T<T> abs_r_mat = Matrix4_T<T>::Identity();
			for (int i = 0; i < 3; ++ i)
			{
				for (int j = 0; j < 3; ++ j)
				{
					abs_r_mat(i, j) = MathLib::abs(r_mat(i, j)) + epsilon;
				}
			}

			Vector_T<T, 3> const & lr = lhs.HalfSize();
			Vector_T<T, 3> const & rr = obb.HalfSize();

			// Test the three major axes of this OBB.
			for (int i = 0; i < 3; ++ i)
			{
				T const ra = lr[i];
				T const rb = rr[0] * abs_r_mat(i, 0) +  rr[1] * abs_r_mat(i, 1) + rr[2] * abs_r_mat(i, 2);
				if (MathLib::abs(t[i]) > ra + rb) 
				{
					return false;
				}
			}

			// Test the three major axes of the OBB b.
			for (int i = 0; i < 3; ++ i)
			{
				T const ra = lr[0] * abs_r_mat(0, i) + lr[1] * abs_r_mat(1, i) + lr[2] * abs_r_mat(2, i);
				T const rb = rr[i];
				if (MathLib::abs(t.x() * r_mat(0, i) + t.y() * r_mat(1, i) + t.z() * r_mat(2, i)) > ra + rb)
				{
					return false;
				}
			}

			// Test the 9 different cross-axes.

			// A.x <cross> B.x
			T ra = lr.y() * abs_r_mat(2, 0) + lr.z() * abs_r_mat(1, 0);
			T rb = rr.y() * abs_r_mat(0, 2) + rr.z() * abs_r_mat(0, 1);
			if (MathLib::abs(t.z() * r_mat(1, 0) - t.y() * r_mat(2, 0)) > ra + rb)
			{
				return false;
			}

			// A.x < cross> B.y
			ra = lr.y() * abs_r_mat(2, 1) + lr.z() * abs_r_mat(1, 1);
			rb = rr.x() * abs_r_mat(0, 2) + rr.z() * abs_r_mat(0, 0);
			if (MathLib::abs(t.z() * r_mat(1, 1) - t.y() * r_mat(2, 1)) > ra + rb)
			{
				return false;
			}

			// A.x <cross> B.z
			ra = lr.y() * abs_r_mat(2, 2) + lr.z() * abs_r_mat(1, 2);
			rb = rr.x() * abs_r_mat(0, 1) + rr.y() * abs_r_mat(0, 0);
			if (MathLib::abs(t.z() * r_mat(1, 2) - t.y() * r_mat(2, 2)) > ra + rb)
			{
				return false;
			}

			// A.y <cross> B.x
			ra = lr.x() * abs_r_mat(2, 0) + lr.z() * abs_r_mat(0, 0);
			rb = rr.y() * abs_r_mat(1, 2) + rr.z() * abs_r_mat(1, 1);
			if (MathLib::abs(t.x() * r_mat(2, 0) - t.z() * r_mat(0, 0)) > ra + rb)
			{
				return false;
			}

			// A.y <cross> B.y
			ra = lr.x() * abs_r_mat(2, 1) + lr.z() * abs_r_mat(0, 1);
			rb = rr.x() * abs_r_mat(1, 2) + rr.z() * abs_r_mat(1, 0);
			if (MathLib::abs(t.x() * r_mat(2, 1) - t.z() * r_mat(0, 1)) > ra + rb)
			{
				return false;
			}

			// A.y <cross> B.z
			ra = lr.x() * abs_r_mat(2, 2) + lr.z() * abs_r_mat(0, 2);
			rb = rr.x() * abs_r_mat(1, 1) + rr.y() * abs_r_mat(1, 0);
			if (MathLib::abs(t.x() * r_mat(2, 2) - t.z() * r_mat(0, 2)) > ra + rb)
			{
				return false;
			}

			// A.z <cross> B.x
			ra = lr.x() * abs_r_mat(1, 0) + lr.y() * abs_r_mat(0, 0);
			rb = rr.y() * abs_r_mat(2, 2) + rr.z() * abs_r_mat(2, 1);
			if (MathLib::abs(t.y() * r_mat(0, 0) - t.x() * r_mat(1, 0)) > ra + rb)
			{
				return false;
			}

			// A.z <cross> B.y
			ra = lr.x() * abs_r_mat(1, 1) + lr.y() * abs_r_mat(0, 1);
			rb = rr.x() * abs_r_mat(2, 2) + rr.z() * abs_r_mat(2, 0);
			if (MathLib::abs(t.y() * r_mat(0, 1) - t.x() * r_mat(1, 1)) > ra + rb)
			{
				return false;
			}

			// A.z <cross> B.z
			ra = lr.x() * abs_r_mat(1, 2) + lr.y() * abs_r_mat(0, 2);
			rb = rr.x() * abs_r_mat(2, 1) + rr.y() * abs_r_mat(2, 0);
			if (MathLib::abs(t.y() * r_mat(0, 2) - t.x() * r_mat(1, 2)) > ra + rb)
			{
				return false;
			}

			return true;
		}

		template bool intersect_obb_sphere(OBBox const & lhs, Sphere const & sphere) noexcept;

		template <typename T>
		bool intersect_obb_sphere(OBBox_T<T> const & lhs, Sphere_T<T> const & sphere) noexcept
		{
			Vector_T<T, 3> const d = sphere.Center() - lhs.Center();
			Vector_T<T, 3> closest_point_on_obb = lhs.Center();
			for (int i = 0; i < 3; ++ i)
			{
				T dist = dot(d, lhs.Axis(i));
				if (dist > lhs.HalfSize()[i])
				{
					dist = lhs.HalfSize()[i];
				}
				if (dist < -lhs.HalfSize()[i])
				{
					dist = -lhs.HalfSize()[i];
				}
				closest_point_on_obb += dist * lhs.Axis(i);
			}

			Vector_T<T, 3> const v = closest_point_on_obb - sphere.Center();
			return length_sq(v) <= sphere.Radius() * sphere.Radius();
		}

		template bool intersect_sphere_sphere(Sphere const & lhs, Sphere const & sphere) noexcept;

		template <typename T>
		bool intersect_sphere_sphere(Sphere_T<T> const & lhs, Sphere_T<T> const & sphere) noexcept
		{
			Vector_T<T, 3> const d = lhs.Center() - sphere.Center();
			float const r = lhs.Radius() + sphere.Radius();
			return length_sq(d) <= r * r;
		}


		template BoundOverlap intersect_aabb_frustum(AABBox const & aabb, Frustum const & frustum) noexcept;

		template <typename T>
		BoundOverlap intersect_aabb_frustum(AABBox_T<T> const & aabb, Frustum_T<T> const & frustum) noexcept
		{
			Vector_T<T, 3> const & min_pt = aabb.Min();
			Vector_T<T, 3> const & max_pt = aabb.Max();

			bool intersect = false;
			for (int i = 0; i < 6; ++ i)
			{
				Plane_T<T> const & plane = frustum.FrustumPlane(i);

				// v1 is diagonally opposed to v0
				Vector_T<T, 3> v0((plane.a() < 0) ? min_pt.x() : max_pt.x(), (plane.b() < 0) ? min_pt.y() : max_pt.y(), (plane.c() < 0) ? min_pt.z() : max_pt.z());
				Vector_T<T, 3> v1((plane.a() < 0) ? max_pt.x() : min_pt.x(), (plane.b() < 0) ? max_pt.y() : min_pt.y(), (plane.c() < 0) ? max_pt.z() : min_pt.z());

				if (dot_coord(plane, v0) < 0)
				{
					return BO_No;
				}
				if (dot_coord(plane, v1) < 0)
				{
					intersect = true;
				}
			}

			return intersect ? BO_Partial : BO_Yes;
		}

		template BoundOverlap intersect_obb_frustum(OBBox const & obb, Frustum const & frustum) noexcept;

		template <typename T>
		BoundOverlap intersect_obb_frustum(OBBox_T<T> const & obb, Frustum_T<T> const & frustum) noexcept
		{
			Vector_T<T, 3> min_pt = obb.Corner(0);
			Vector_T<T, 3> max_pt = min_pt;
			for (int i = 1; i < 8; ++ i)
			{
				Vector_T<T, 3> const corner = obb.Corner(i);

				min_pt = minimize(min_pt, corner);
				max_pt = maximize(max_pt, corner);
			}

			bool intersect = false;
			for (int i = 0; i < 6; ++ i)
			{
				Plane_T<T> const & plane = frustum.FrustumPlane(i);

				// v1 is diagonally opposed to v0
				Vector_T<T, 3> v0((plane.a() < 0) ? min_pt.x() : max_pt.x(), (plane.b() < 0) ? min_pt.y() : max_pt.y(), (plane.c() < 0) ? min_pt.z() : max_pt.z());
				Vector_T<T, 3> v1((plane.a() < 0) ? max_pt.x() : min_pt.x(), (plane.b() < 0) ? max_pt.y() : min_pt.y(), (plane.c() < 0) ? max_pt.z() : min_pt.z());

				if (dot_coord(plane, v0) < 0)
				{
					return BO_No;
				}
				if (dot_coord(plane, v1) < 0)
				{
					intersect = true;
				}
			}

			return intersect ? BO_Partial : BO_Yes;
		}

		template BoundOverlap intersect_sphere_frustum(Sphere const & sphere, Frustum const & frustum) noexcept;

		template <typename T>
		BoundOverlap intersect_sphere_frustum(Sphere_T<T> const & sphere, Frustum_T<T> const & frustum) noexcept
		{
			bool intersect = false;
			for (int i = 0; i < 6; ++ i)
			{
				Plane_T<T> const & plane = frustum.FrustumPlane(i);

				float const d = dot_coord(plane, sphere.Center());
				if (d <= -sphere.Radius())
				{
					return BO_No;
				}
				if (d > sphere.Radius())
				{
					intersect = true;
				}
			}

			return intersect ? BO_Partial : BO_Yes;
		}

		template BoundOverlap intersect_frustum_frustum(Frustum const & lhs, Frustum const & rhs) noexcept;

		template <typename T>
		BoundOverlap intersect_frustum_frustum(Frustum_T<T> const & lhs, Frustum_T<T> const & rhs) noexcept
		{
			bool outside = false;
			bool inside_all = true;
			for (int i = 0; i < 6; ++ i)
			{
				Plane_T<T> const & p = lhs.FrustumPlane(i);

				T min_p, max_p;
				min_p = max_p = dot_coord(p, rhs.Corner(0));
				for (int j = 1; j < 8; ++ j)
				{
					T tmp = dot_coord(p, rhs.Corner(j));
					min_p = std::min(min_p, tmp);
					max_p = std::max(max_p, tmp);
				}

				outside |= (min_p > 0);
				inside_all &= (max_p <= 0);
			}
			if (outside)
			{
				return BO_No;
			}
			if (inside_all)
			{
				return BO_Yes;
			}

			for (int i = 0; i < 6; ++ i)
			{
				Plane_T<T> const & p = rhs.FrustumPlane(i);

				T min_p = dot_coord(p, lhs.Corner(0));
				for (int j = 1; j < 8; ++ j)
				{
					T tmp = dot_coord(p, lhs.Corner(j));
					min_p = std::min(min_p, tmp);
				}

				outside |= (min_p > 0);
			}
			if (outside)
			{
				return BO_No;
			}

			Vector_T<T, 3> edge_axis_l[6];
			edge_axis_l[0] = rhs.Corner(6);
			edge_axis_l[1] = rhs.Corner(4);
			edge_axis_l[2] = rhs.Corner(5);
			edge_axis_l[3] = rhs.Corner(7);
			edge_axis_l[4] = rhs.Corner(6) - rhs.Corner(5);
			edge_axis_l[5] = rhs.Corner(7) - rhs.Corner(5);

			Vector_T<T, 3> edge_axis_r[6];
			edge_axis_r[0] = lhs.Corner(6);
			edge_axis_r[1] = lhs.Corner(4);
			edge_axis_r[2] = lhs.Corner(5);
			edge_axis_r[3] = lhs.Corner(7);
			edge_axis_r[4] = lhs.Corner(6) - lhs.Corner(5);
			edge_axis_r[5] = lhs.Corner(7) - lhs.Corner(5);

			for (int i = 0; i < 6; ++ i)
			{
				for (int j = 0; j < 6; ++ j)
				{
					Vector_T<T, 3> const axis = cross(edge_axis_l[i], edge_axis_r[j]);

					T min_l, max_l, min_r, max_r;
					min_l = max_l = dot(axis, rhs.Corner(0));
					min_r = max_r = dot(axis, lhs.Corner(0));
					for (int k = 1; k < 8; ++ k)
					{
						T tmp = dot(axis, rhs.Corner(k));
						min_l = std::min(min_l, tmp);
						max_l = std::max(max_l, tmp);

						tmp = dot(axis, lhs.Corner(k));
						min_r = std::min(min_r, tmp);
						max_r = std::max(max_r, tmp);
					}

					outside |= min_l > max_r;
					outside |= min_r > max_l;
				}
			}
			if (outside)
			{
				return BO_No;
			}

			return BO_Partial;
		}


		template void intersect(float3 const & v0, float3 const & v1, float3 const & v2,
						float3 const & ray_orig, float3 const & ray_dir,
						float& t, float& u, float& v) noexcept;

		template <typename T>
		void intersect(Vector_T<T, 3> const & v0, Vector_T<T, 3> const & v1, Vector_T<T, 3> const & v2,
						Vector_T<T, 3> const & ray_orig, Vector_T<T, 3> const & ray_dir,
						T& t, T& u, T& v) noexcept
		{
			// Find vectors for two edges sharing vert0
			Vector_T<T, 3> const edge1 = v1 - v0;
			Vector_T<T, 3> const edge2 = v2 - v0;

			// Begin calculating determinant - also used to calculate U parameter
			Vector_T<T, 3> const pvec(cross(ray_dir, edge2));

			// If determinant is near zero, ray lies in plane of triangle
			T det = dot(edge1, pvec);

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
			u = dot(tvec, pvec);

			// Prepare to test V parameter
			Vector_T<T, 3> const qvec(cross(tvec, edge1));

			// Calculate V parameter
			v = dot(ray_dir, qvec);

			// Calculate t, scale parameters, ray intersects triangle
			t = dot(edge2, qvec);

			T const inv_det = T(1) / det;
			v *= inv_det;
			u *= inv_det;
			t *= inv_det;
		}

		template bool bary_centric_in_triangle(float const & u, float const & v) noexcept;

		template <typename T>
		bool bary_centric_in_triangle(T const & u, T const & v) noexcept
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

		float linear_to_srgb(float linear) noexcept
		{
			if (linear < 0.0031308f)
			{
				return 12.92f * linear;
			}
			else
			{
				float const ALPHA = 0.055f;
				return (1 + ALPHA) * pow(linear, 1 / 2.4f) - ALPHA;
			}
		}

		float srgb_to_linear(float srgb) noexcept
		{
			if (srgb < 0.04045f)
			{
				return srgb / 12.92f;
			}
			else
			{
				float const ALPHA = 0.055f;
				return pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
			}
		}

		template Quaternion quat_trans_to_udq(Quaternion const & q, float3 const & t) noexcept;

		template <typename T>
		Quaternion_T<T> quat_trans_to_udq(Quaternion_T<T> const & q, Vector_T<T, 3> const & t) noexcept
		{
			return mul(q, Quaternion_T<T>(T(0.5) * t.x(), T(0.5) * t.y(), T(0.5) * t.z(), T(0.0)));
		}

		template float3 udq_to_trans(Quaternion const & real, Quaternion const & dual) noexcept;

		template <typename T>
		Vector_T<T, 3> udq_to_trans(Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			Quaternion_T<T> qeq0 = mul(conjugate(real), dual);
			return T(2.0) * Vector_T<T, 3>(qeq0.x(), qeq0.y(), qeq0.z());
		}

		template float3 dq_to_trans(Quaternion const & real, Quaternion const & dual) noexcept;

		template <typename T>
		Vector_T<T, 3> dq_to_trans(Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			return udq_to_trans(real, dual) / length(real);
		}

		template float4x4 udq_to_matrix(Quaternion const & real, Quaternion const & dual) noexcept;

		template <typename T>
		Matrix4_T<T> udq_to_matrix(Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			Matrix4_T<T> m;

			float len2 = dot(real, real);
			float w = real.w(), x = real.x(), y = real.y(), z = real.z();
			float t0 = dual.w(), t1 = dual.x(), t2 = dual.y(), t3 = dual.z();

			m(0, 0) = w * w + x * x - y * y - z * z;
			m(1, 0) = 2 * x * y - 2 * w * z;
			m(2, 0) = 2 * x * z + 2 * w * y;
			m(0, 1) = 2 * x * y + 2 * w * z;
			m(1, 1) = w * w + y * y - x * x - z * z;
			m(2, 1) = 2 * y * z - 2 * w * x;
			m(0, 2) = 2 * x * z - 2 * w * y;
			m(1, 2) = 2 * y * z + 2 * w * x;
			m(2, 2) = w * w + z * z - x * x - y * y;

			m(3, 0) = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
			m(3, 1) = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
			m(3, 2) = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;

			m(0, 3) = 0;
			m(1, 3) = 0;
			m(2, 3) = 0;
			m(3, 3) = len2;

			m /= len2;

			return m;
		}

		template std::pair<Quaternion, Quaternion> conjugate(Quaternion const & real, Quaternion const & dual) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> conjugate(Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			return std::make_pair(conjugate(real), conjugate(dual));
		}

		template std::pair<Quaternion, Quaternion> inverse(Quaternion const & real, Quaternion const & dual) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> inverse(Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			float const sqr_len_0 = dot(real, real);
			float const sqr_len_e = 2.0f * dot(real, dual);
			float const inv_sqr_len_0 = 1.0f / sqr_len_0;
			float const inv_sqr_len_e = -sqr_len_e / (sqr_len_0 * sqr_len_0);
			std::pair<Quaternion_T<T>, Quaternion_T<T>> conj = conjugate(real, dual);
			return std::make_pair(inv_sqr_len_0 * conj.first, inv_sqr_len_0 * conj.second + inv_sqr_len_e * conj.first);
		}

		template Quaternion mul_real(Quaternion const & lhs_real, Quaternion const & rhs_real) noexcept;

		template <typename T>
		Quaternion_T<T> mul_real(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & rhs_real) noexcept
		{
			return lhs_real * rhs_real;
		}

		template Quaternion mul_dual(Quaternion const & lhs_real, Quaternion const & lhs_dual,
			Quaternion const & rhs_real, Quaternion const & rhs_dual) noexcept;

		template <typename T>
		Quaternion_T<T> mul_dual(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & lhs_dual,
			Quaternion_T<T> const & rhs_real, Quaternion_T<T> const & rhs_dual) noexcept
		{
			return lhs_real * rhs_dual + lhs_dual * rhs_real;
		}

		template void udq_to_screw(float& angle, float& pitch, float3& dir, float3& moment,
			Quaternion const & real, Quaternion const & dual) noexcept;

		template <typename T>
		void udq_to_screw(T& angle, T& pitch, Vector_T<T, 3>& dir, Vector_T<T, 3>& moment,
			Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			if (abs(real.w()) >= 1)
			{
				// pure translation

				angle = 0;
				dir = dual.v();

				T dir_sq_len = length_sq(dir);

				if (dir_sq_len > T(1e-6))
				{
					T dir_len = sqrt(dir_sq_len);
					pitch = 2 * dir_len;
					dir /= dir_len;
				}
				else
				{
					pitch = 0;
				}

				moment = Vector_T<T, 3>::Zero();
			}
			else
			{ 
				angle = 2 * acos(real.w());

				float const s = length_sq(real.v());
				if (s < T(1e-6))
				{
					dir = Vector_T<T, 3>::Zero();
					pitch = 0;
					moment = Vector_T<T, 3>::Zero();
				}
				else
				{
					float oos = recip_sqrt(s);
					dir = real.v() * oos;

					pitch = -2 * dual.w() * oos;

					moment = (dual.v() - dir * pitch * real.w() * T(0.5)) * oos;
				}
			}
		}

		template std::pair<Quaternion, Quaternion> udq_from_screw(float const & angle, float const & pitch, float3 const & dir, float3 const & moment) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> udq_from_screw(T const & angle, T const & pitch, Vector_T<T, 3> const & dir, Vector_T<T, 3> const & moment) noexcept
		{
			T sa, ca;
			sincos(angle * T(0.5), sa, ca);
			return std::make_pair(Quaternion_T<T>(dir * sa, ca),
				Quaternion_T<T>(sa * moment + T(0.5) * pitch * ca * dir, -pitch * sa * T(0.5)));
		}


		template std::pair<Quaternion, Quaternion> sclerp(Quaternion const & lhs_real, Quaternion const & lhs_dual,
			Quaternion const & rhs_real, Quaternion const & rhs_dual, float s) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> sclerp(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & lhs_dual,
			Quaternion_T<T> const & rhs_real, Quaternion_T<T> const & rhs_dual, T s) noexcept
		{
			// Make sure dot product is >= 0
			float const quat_dot = dot(lhs_real, rhs_real);
			Quaternion to_sign_corrected_real = rhs_real;
			Quaternion to_sign_corrected_dual = rhs_dual;
			if (quat_dot < 0)
			{
				to_sign_corrected_real = -to_sign_corrected_real;
				to_sign_corrected_dual = -to_sign_corrected_dual;
			}

			std::pair<Quaternion_T<T>, Quaternion_T<T>> dif_dq = inverse(lhs_real, lhs_dual);
			dif_dq.second = mul_dual(dif_dq.first, dif_dq.second, to_sign_corrected_real, to_sign_corrected_dual);
			dif_dq.first = mul_real(dif_dq.first, to_sign_corrected_real);
	
			float angle, pitch;
			float3 direction, moment;
			udq_to_screw(angle, pitch, direction, moment, dif_dq.first, dif_dq.second);

			angle *= s; 
			pitch *= s;
			dif_dq = udq_from_screw(angle, pitch, direction, moment);

			dif_dq.second = mul_dual(lhs_real, lhs_dual, dif_dq.first, dif_dq.second);
			dif_dq.first = mul_real(lhs_real, dif_dq.first);

			return dif_dq;
		}
	}
}
