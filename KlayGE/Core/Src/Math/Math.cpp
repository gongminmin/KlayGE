// Math.cpp
// KlayGE Math implement file
// Ver 3.12.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 3.12.0
// First release (2011.2.15)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Detail/MathHelper.hpp>

#include <KlayGE/Math.hpp>

namespace KlayGE
{
	namespace MathLib
	{
		float abs(float x)
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
		
		float sqrt(float x)
		{
			return std::sqrt(x);
		}

		// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
		float recip_sqrt(float number)
		{
			float const threehalfs = 1.5f;

			float x2 = number * 0.5f;
			union FNI
			{
				float f;
				int32_t i;
			} fni;
			fni.f = number;											// evil floating point bit level hacking
			fni.i = 0x5f375a86 - (fni.i >> 1);						// what the fuck?
			fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));	// 1st iteration
			//fni.f = f * (threehalfs - (x2 * fni.f * fni.f));		// 2nd iteration, this can be removed

			return fni.f;
		}

		float pow(float x, float y)
		{
			return std::pow(x, y);
		}

		float exp(float x)
		{
			return std::exp(x);
		}

		float log(float x)
		{
			return std::log(x);
		}

		float log10(float x)
		{
			return std::log10(x);
		}

		float sin(float x)
		{
			return std::sin(x);
		}

		float cos(float x)
		{
			return sin(x + PI / 2);
		}
		
		void sincos(float x, float& s, float& c)
		{
			s = sin(x);
			c = cos(x);
		}

		float tan(float x)
		{
			return std::tan(x);
		}

		float asin(float x)
		{
			return std::asin(x);
		}

		float acos(float x)
		{
			return std::acos(x);
		}

		float atan(float x)
		{
			return std::atan(x);
		}

		float sinh(float x)
		{
			return std::sinh(x);
		}
		
		float cosh(float x)
		{
			return std::cosh(x);
		}

		float tanh(float x)
		{
			return std::tanh(x);
		}


		template KLAYGE_CORE_API int32_t dot(int1 const & lhs, int1 const & rhs);
		template KLAYGE_CORE_API int32_t dot(int2 const & lhs, int2 const & rhs);
		template KLAYGE_CORE_API int32_t dot(int3 const & lhs, int3 const & rhs);
		template KLAYGE_CORE_API int32_t dot(int4 const & lhs, int4 const & rhs);
		template KLAYGE_CORE_API uint32_t dot(uint1 const & lhs, uint1 const & rhs);
		template KLAYGE_CORE_API uint32_t dot(uint2 const & lhs, uint2 const & rhs);
		template KLAYGE_CORE_API uint32_t dot(uint3 const & lhs, uint3 const & rhs);
		template KLAYGE_CORE_API uint32_t dot(uint4 const & lhs, uint4 const & rhs);
		template KLAYGE_CORE_API float dot(float1 const & lhs, float1 const & rhs);
		template KLAYGE_CORE_API float dot(float2 const & lhs, float2 const & rhs);
		template KLAYGE_CORE_API float dot(float3 const & lhs, float3 const & rhs);
		template KLAYGE_CORE_API float dot(float4 const & lhs, float4 const & rhs);
		template KLAYGE_CORE_API float dot(Quaternion const & lhs, Quaternion const & rhs);
		template KLAYGE_CORE_API float dot(Color const & lhs, Color const & rhs);

		template <typename T>
		typename T::value_type dot(T const & lhs, T const & rhs)
		{
			return detail::dot_helper<typename T::value_type,
							T::elem_num>::Do(&lhs[0], &rhs[0]);
		}

		template KLAYGE_CORE_API int32_t length_sq(int1 const & rhs);
		template KLAYGE_CORE_API int32_t length_sq(int2 const & rhs);
		template KLAYGE_CORE_API int32_t length_sq(int3 const & rhs);
		template KLAYGE_CORE_API int32_t length_sq(int4 const & rhs);
		template KLAYGE_CORE_API uint32_t length_sq(uint1 const & rhs);
		template KLAYGE_CORE_API uint32_t length_sq(uint2 const & rhs);
		template KLAYGE_CORE_API uint32_t length_sq(uint3 const & rhs);
		template KLAYGE_CORE_API uint32_t length_sq(uint4 const & rhs);
		template KLAYGE_CORE_API float length_sq(float1 const & rhs);
		template KLAYGE_CORE_API float length_sq(float2 const & rhs);
		template KLAYGE_CORE_API float length_sq(float3 const & rhs);
		template KLAYGE_CORE_API float length_sq(float4 const & rhs);
		template KLAYGE_CORE_API float length_sq(Quaternion const & rhs);
		template KLAYGE_CORE_API float length_sq(Plane const & rhs);

		template <typename T>
		typename T::value_type length_sq(T const & rhs)
		{
			return dot(rhs, rhs);
		}

		template KLAYGE_CORE_API float length(float1 const & rhs);
		template KLAYGE_CORE_API float length(float2 const & rhs);
		template KLAYGE_CORE_API float length(float3 const & rhs);
		template KLAYGE_CORE_API float length(float4 const & rhs);
		template KLAYGE_CORE_API float length(Quaternion const & rhs);
		template KLAYGE_CORE_API float length(Plane const & rhs);

		template <typename T>
		typename T::value_type length(T const & rhs)
		{
			return sqrt(length_sq(rhs));
		}

		template KLAYGE_CORE_API float lerp(float const & lhs, float const & rhs, float s);
		template KLAYGE_CORE_API float1 lerp(float1 const & lhs, float1 const & rhs, float s);
		template KLAYGE_CORE_API float2 lerp(float2 const & lhs, float2 const & rhs, float s);
		template KLAYGE_CORE_API float3 lerp(float3 const & lhs, float3 const & rhs, float s);
		template KLAYGE_CORE_API float4 lerp(float4 const & lhs, float4 const & rhs, float s);
		template KLAYGE_CORE_API Color lerp(Color const & lhs, Color const & rhs, float s);

		template <typename T>
		T lerp(T const & lhs, T const & rhs, float s)
		{
			return lhs + (rhs - lhs) * s;
		}

		template KLAYGE_CORE_API int1 maximize(int1 const & lhs, int1 const & rhs);
		template KLAYGE_CORE_API int2 maximize(int2 const & lhs, int2 const & rhs);
		template KLAYGE_CORE_API int3 maximize(int3 const & lhs, int3 const & rhs);
		template KLAYGE_CORE_API int4 maximize(int4 const & lhs, int4 const & rhs);
		template KLAYGE_CORE_API uint1 maximize(uint1 const & lhs, uint1 const & rhs);
		template KLAYGE_CORE_API uint2 maximize(uint2 const & lhs, uint2 const & rhs);
		template KLAYGE_CORE_API uint3 maximize(uint3 const & lhs, uint3 const & rhs);
		template KLAYGE_CORE_API uint4 maximize(uint4 const & lhs, uint4 const & rhs);
		template KLAYGE_CORE_API float1 maximize(float1 const & lhs, float1 const & rhs);
		template KLAYGE_CORE_API float2 maximize(float2 const & lhs, float2 const & rhs);
		template KLAYGE_CORE_API float3 maximize(float3 const & lhs, float3 const & rhs);
		template KLAYGE_CORE_API float4 maximize(float4 const & lhs, float4 const & rhs);

		template <typename T>
		T maximize(T const & lhs, T const & rhs)
		{
			T ret;
			detail::max_minimize_helper<typename T::value_type, T::elem_num>::DoMax(&ret[0], &lhs[0], &rhs[0]);
			return ret;
		}

		template KLAYGE_CORE_API int1 minimize(int1 const & lhs, int1 const & rhs);
		template KLAYGE_CORE_API int2 minimize(int2 const & lhs, int2 const & rhs);
		template KLAYGE_CORE_API int3 minimize(int3 const & lhs, int3 const & rhs);
		template KLAYGE_CORE_API int4 minimize(int4 const & lhs, int4 const & rhs);
		template KLAYGE_CORE_API uint1 minimize(uint1 const & lhs, uint1 const & rhs);
		template KLAYGE_CORE_API uint2 minimize(uint2 const & lhs, uint2 const & rhs);
		template KLAYGE_CORE_API uint3 minimize(uint3 const & lhs, uint3 const & rhs);
		template KLAYGE_CORE_API uint4 minimize(uint4 const & lhs, uint4 const & rhs);
		template KLAYGE_CORE_API float1 minimize(float1 const & lhs, float1 const & rhs);
		template KLAYGE_CORE_API float2 minimize(float2 const & lhs, float2 const & rhs);
		template KLAYGE_CORE_API float3 minimize(float3 const & lhs, float3 const & rhs);
		template KLAYGE_CORE_API float4 minimize(float4 const & lhs, float4 const & rhs);

		template <typename T>
		T minimize(T const & lhs, T const & rhs)
		{
			T ret;
			detail::max_minimize_helper<typename T::value_type, T::elem_num>::DoMin(&ret[0], &lhs[0], &rhs[0]);
			return ret;
		}

		template KLAYGE_CORE_API float4 transform(float2 const & v, float4x4 const & mat);
		template KLAYGE_CORE_API float4 transform(float3 const & v, float4x4 const & mat);
		template KLAYGE_CORE_API float4 transform(float4 const & v, float4x4 const & mat);

		template <typename T>
		Vector_T<typename T::value_type, 4> transform(T const & v, Matrix4_T<typename T::value_type> const & mat)
		{
			return detail::transform_helper<typename T::value_type, T::elem_num>::Do(v, mat);
		}

		template KLAYGE_CORE_API float2 transform_coord(float2 const & v, float4x4 const & mat);
		template KLAYGE_CORE_API float3 transform_coord(float3 const & v, float4x4 const & mat);

		template <typename T>
		T transform_coord(T const & v, Matrix4_T<typename T::value_type> const & mat)
		{
			BOOST_STATIC_ASSERT(T::elem_num < 4);

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

		template KLAYGE_CORE_API float2 transform_normal(float2 const & v, float4x4 const & mat);
		template KLAYGE_CORE_API float3 transform_normal(float3 const & v, float4x4 const & mat);

		template <typename T>
		T transform_normal(T const & v, Matrix4_T<typename T::value_type> const & mat)
		{
			BOOST_STATIC_ASSERT(T::elem_num < 4);

			return detail::transform_normal_helper<typename T::value_type, T::elem_num>::Do(v, mat);
		}

		template KLAYGE_CORE_API float1 bary_centric(float1 const & v1, float1 const & v2, float1 const & v3, float const & f, float const & g);
		template KLAYGE_CORE_API float2 bary_centric(float2 const & v1, float2 const & v2, float2 const & v3, float const & f, float const & g);
		template KLAYGE_CORE_API float3 bary_centric(float3 const & v1, float3 const & v2, float3 const & v3, float const & f, float const & g);
		template KLAYGE_CORE_API float4 bary_centric(float4 const & v1, float4 const & v2, float4 const & v3, float const & f, float const & g);

		template <typename T>
		T bary_centric(T const & v1, T const & v2, T const & v3, typename T::value_type const & f, typename T::value_type const & g)
		{
			return (1 - f - g) * v1 + f * v2 + g * v3;
		}

		template KLAYGE_CORE_API float1 normalize(float1 const & rhs);
		template KLAYGE_CORE_API float2 normalize(float2 const & rhs);
		template KLAYGE_CORE_API float3 normalize(float3 const & rhs);
		template KLAYGE_CORE_API float4 normalize(float4 const & rhs);
		template KLAYGE_CORE_API Quaternion normalize(Quaternion const & rhs);

		template <typename T>
		T normalize(T const & rhs)
		{
			return rhs * recip_sqrt(length_sq(rhs));
		}

		template KLAYGE_CORE_API Plane normalize(Plane const & rhs);

		template <typename T>
		Plane_T<T> normalize(Plane_T<T> const & rhs)
		{
			T const inv(T(1) / length(rhs));
			return Plane_T<T>(rhs.a() * inv, rhs.b() * inv, rhs.c() * inv, rhs.d() * inv);
		}

		template KLAYGE_CORE_API float3 reflect(float3 const & incident, float3 const & normal);

		template <typename T>
		T reflect(T const & incident, T const & normal)
		{
			return incident - 2 * dot(incident, normal) * normal;
		}

		template KLAYGE_CORE_API float3 refract(float3 const & incident, float3 const & normal, float const & refraction_index);

		template <typename T>
		T refract(T const & incident, T const & normal, typename T::value_type const & refraction_index)
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

		template KLAYGE_CORE_API float fresnel_term(float const & cos_theta, float const & refraction_index);

		template <typename T>
		T fresnel_term(T const & cos_theta, T const & refraction_index)
		{
			T g = sqrt(sqr(refraction_index) + sqr(cos_theta) - 1);
			return T(0.5) * sqr(g + cos_theta) / sqr(g - cos_theta)
					* (sqr(cos_theta * (g + cos_theta) - 1) / sqr(cos_theta * (g - cos_theta) + 1) + 1);
		}

		// 2D Vector
		///////////////////////////////////////////////////////////////////////////////

		template KLAYGE_CORE_API int32_t cross(int2 const & lhs, int2 const & rhs);
		template KLAYGE_CORE_API uint32_t cross(uint2 const & lhs, uint2 const & rhs);
		template KLAYGE_CORE_API float cross(float2 const & lhs, float2 const & rhs);

		template <typename T>
		T cross(Vector_T<T, 2> const & lhs, Vector_T<T, 2> const & rhs)
		{
			return lhs.x() * rhs.y() - lhs.y() * rhs.x();
		}

		// 3D Vector
		///////////////////////////////////////////////////////////////////////////////

		template KLAYGE_CORE_API float angle(float3 const & lhs, float3 const & rhs);

		template <typename T>
		T angle(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
		}

		template KLAYGE_CORE_API int3 cross(int3 const & lhs, int3 const & rhs);
		template KLAYGE_CORE_API uint3 cross(uint3 const & lhs, uint3 const & rhs);
		template KLAYGE_CORE_API float3 cross(float3 const & lhs, float3 const & rhs);

		template <typename T>
		Vector_T<T, 3> cross(Vector_T<T, 3> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return Vector_T<T, 3>(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
				lhs.z() * rhs.x() - lhs.x() * rhs.z(),
				lhs.x() * rhs.y() - lhs.y() * rhs.x());
		}

		template KLAYGE_CORE_API float3 transform_quat(float3 const & v, Quaternion const & quat);

		template <typename T>
		Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat)
		{
			// result = av + bq + c(q.v CROSS v)
			// where
			//  a = q.w()^2 - (q.v DOT q.v)
			//  b = 2 * (q.v DOT v)
			//  c = 2q.w()
			T const a(quat.w() * quat.w() - length_sq(quat.v()));
			T const b(2 * dot(quat.v(), v));
			T const c(quat.w() + quat.w());

			// Must store this, because result may alias v
			Vector_T<T, 3> cross_v(cross(quat.v(), v));		// q.v CROSS v

			return Vector_T<T, 3>(a * v.x() + b * quat.x() + c * cross_v.x(),
				a * v.y() + b * quat.y() + c * cross_v.y(),
				a * v.z() + b * quat.z() + c * cross_v.z());
		}

		template KLAYGE_CORE_API float3 project(float3 const & vec,
			float4x4 const & world, float4x4 const & view, float4x4 const & proj,
			int const viewport[4], float const & nearPlane, float const & farPlane);

		template <typename T>
		Vector_T<T, 3> project(Vector_T<T, 3> const & vec,
			Matrix4_T<T> const & world, Matrix4_T<T> const & view, Matrix4_T<T> const & proj,
			int const viewport[4], T const & nearPlane, T const & farPlane)
		{
			Vector_T<T, 4> temp(transform(vec, world));
			temp = transform(temp, view);
			temp = transform(temp, proj);
			temp /= temp.w();

			Vector_T<T, 3> ret;
			ret.x() = (temp.x() + 1) * viewport[2] / 2 + viewport[0];
			ret.y() = (-temp.y() + 1) * viewport[3] / 2 + viewport[1];
			ret.z() = temp.z() * (farPlane - nearPlane) + nearPlane;
			return ret;
		}

		template KLAYGE_CORE_API float3 unproject(float3 const & winVec, float const & clipW,
			float4x4 const & world, float4x4 const & view, float4x4 const & proj,
			int const viewport[4], float const & nearPlane, float const & farPlane);

		template <typename T>
		Vector_T<T, 3> unproject(Vector_T<T, 3> const & winVec, T const & clipW,
			Matrix4_T<T> const & world, Matrix4_T<T> const & view, Matrix4_T<T> const & proj,
			int const viewport[4], T const & nearPlane, T const & farPlane)
		{
			Vector_T<T, 4> temp;
			temp.x() = 2 * (winVec.x() - viewport[0]) / viewport[2] - 1;
			temp.y() = -(2 * (winVec.y() - viewport[1]) / viewport[3] - 1);
			temp.z() = (winVec.z() - nearPlane) / (farPlane - nearPlane);
			temp.w() = clipW;

			Matrix4_T<T> mat(inverse(world * view * proj));
			temp = transform(temp, mat);

			return Vector_T<T, 3>(temp.x(), temp.y(), temp.z()) / temp.w();
		}


		// 4D Vector
		///////////////////////////////////////////////////////////////////////////////
		
		template KLAYGE_CORE_API int4 cross(int4 const & v1, int4 const & v2, int4 const & v3);
		template KLAYGE_CORE_API float4 cross(float4 const & v1, float4 const & v2, float4 const & v3);

		template <typename T>
		Vector_T<T, 4> cross(Vector_T<T, 4> const & v1, Vector_T<T, 4> const & v2, Vector_T<T, 4> const & v3)
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

		template KLAYGE_CORE_API float4x4 mul(float4x4 const & lhs, float4x4 const & rhs);

		template <typename T>
		Matrix4_T<T> mul(Matrix4_T<T> const & lhs, Matrix4_T<T> const & rhs)
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

		template KLAYGE_CORE_API float determinant(float4x4 const & rhs);

		template <typename T>
		T determinant(Matrix4_T<T> const & rhs)
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

		template KLAYGE_CORE_API float4x4 inverse(float4x4 const & rhs);

		template <typename T>
		Matrix4_T<T> inverse(Matrix4_T<T> const & rhs)
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
			if (!equal<T>(det, 0))
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
			else
			{
				return rhs;
			}
		}

		template KLAYGE_CORE_API float4x4 look_at_lh(float3 const & vEye, float3 const & vAt,
			float3 const & vUp);

		template <typename T>
		Matrix4_T<T> look_at_lh(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt,
			Vector_T<T, 3> const & vUp)
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

		template KLAYGE_CORE_API float4x4 look_at_rh(float3 const & vEye, float3 const & vAt,
			float3 const & vUp);

		template <typename T>
		Matrix4_T<T> look_at_rh(Vector_T<T, 3> const & vEye, Vector_T<T, 3> const & vAt,
			Vector_T<T, 3> const & vUp)
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

		template KLAYGE_CORE_API float4x4 ortho_lh(float const & w, float const & h,
			float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> ortho_lh(T const & w, T const & h, T const & nearPlane, T const & farPlane)
		{
			T const w_2(w / 2);
			T const h_2(h / 2);
			return ortho_off_center_lh(-w_2, w_2, -h_2, h_2, nearPlane, farPlane);
		}

		template KLAYGE_CORE_API float4x4 ortho_off_center_lh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> ortho_off_center_lh(T const & left, T const & right, T const & bottom, T const & top,
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

		template KLAYGE_CORE_API float4x4 perspective_lh(float const & width, float const & height, float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> perspective_lh(T const & width, T const & height, T const & nearPlane, T const & farPlane)
		{
			T const q(farPlane / (farPlane - nearPlane));
			T const near2(nearPlane + nearPlane);

			return Matrix4_T<T>(
				near2 / width,	0,				0,				0,
				0,				near2 / height,	0,				0,
				0,				0,				q,				1,
				0,				0,				-nearPlane * q, 0);
		}

		template KLAYGE_CORE_API float4x4 perspective_fov_lh(float const & fov, float const & aspect, float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> perspective_fov_lh(T const & fov, T const & aspect, T const & nearPlane, T const & farPlane)
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

		template KLAYGE_CORE_API float4x4 perspective_off_center_lh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> perspective_off_center_lh(T const & left, T const & right, T const & bottom, T const & top,
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

		template KLAYGE_CORE_API float4x4 reflect(Plane const & p);

		template <typename T>
		Matrix4_T<T> reflect(Plane_T<T> const & p)
		{
			Plane_T<T> P(normalize(p));
			T const aa2(-2 * P.a() * P.a()), ab2(-2 * P.a() * P.b()), ac2(-2 * P.a() * P.c()), ad2(-2 * P.a() * P.d());
			T const bb2(-2 * P.b() * P.b()), bc2(-2 * P.b() * P.c()), bd2(-2 * P.a() * P.c());
			T const cc2(-2 * P.c() * P.c()), cd2(-2 * P.c() * P.d());

			return Matrix4_T<T>(
				aa2 + 1,	ab2,		ac2,		0,
				ab2,		bb2 + 1,	bc2,		0,
				ac2,		bc2,		cc2 + 1,	0,
				ad2,		bd2,		cd2,		1);
		}

		template KLAYGE_CORE_API float4x4 rotation_x(float const & x);

		template <typename T>
		Matrix4_T<T> rotation_x(T const & x)
		{
			float sx, cx;
			sincos(x, sx, cx);

			return Matrix4_T<T>(
				1,	0,		0,		0,
				0,	cx,		sx,		0,
				0,	-sx,	cx,		0,
				0,	0,		0,		1);
		}

		template KLAYGE_CORE_API float4x4 rotation_y(float const & y);

		template <typename T>
		Matrix4_T<T> rotation_y(T const & y)
		{
			float sy, cy;
			sincos(y, sy, cy);

			return Matrix4_T<T>(
				cy,		0,		-sy,	0,
				0,		1,		0,		0,
				sy,		0,		cy,		0,
				0,		0,		0,		1);
		}

		template KLAYGE_CORE_API float4x4 rotation_z(float const & z);

		template <typename T>
		Matrix4_T<T> rotation_z(T const & z)
		{
			float sz, cz;
			sincos(z, sz, cz);

			return Matrix4_T<T>(
				cz,		sz,		0,		0,
				-sz,	cz,		0,		0,
				0,		0,		1,		0,
				0,		0,		0,		1);
		}

		template KLAYGE_CORE_API float4x4 rotation(float const & angle, float const & x, float const & y, float const & z);

		template <typename T>
		Matrix4_T<T> rotation(T const & angle, T const & x, T const & y, T const & z)
		{
			Quaternion_T<T> quat(rotation_axis(Vector_T<T, 3>(x, y, z), angle));
			return to_matrix(quat);
		}

		template KLAYGE_CORE_API float4x4 rotation_matrix_yaw_pitch_roll(float const & yaw, float const & pitch, float const & roll);

		template <typename T>
		Matrix4_T<T> rotation_matrix_yaw_pitch_roll(T const & yaw, T const & pitch, T const & roll)
		{
			Matrix4_T<T> rotX(rotation_x(pitch));
			Matrix4_T<T> rotY(rotation_y(yaw));
			Matrix4_T<T> rotZ(rotation_z(roll));
			return rotZ * rotX * rotY;
		}

		template KLAYGE_CORE_API float4x4 scaling(float const & sx, float const & sy, float const & sz);

		template <typename T>
		Matrix4_T<T> scaling(T const & sx, T const & sy, T const & sz)
		{
			return Matrix4_T<T>(
				sx,	0,	0,	0,
				0,	sy,	0,	0,
				0,	0,	sz,	0,
				0,	0,	0,	1);
		}

		template KLAYGE_CORE_API float4x4 scaling(float3 const & s);

		template <typename T>
		Matrix4_T<T> scaling(Vector_T<T, 3> const & s)
		{
			return scaling(s.x(), s.y(), s.z());
		}

		template KLAYGE_CORE_API float4x4 shadow(float4 const & l, Plane const & p);

		template <typename T>
		Matrix4_T<T> shadow(Vector_T<T, 4> const & l, Plane_T<T> const & p)
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

		template KLAYGE_CORE_API float4x4 to_matrix(Quaternion const & quat);

		template <typename T>
		Matrix4_T<T> to_matrix(Quaternion_T<T> const & quat)
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

		template KLAYGE_CORE_API float4x4 translation(float const & x, float const & y, float const & z);

		template <typename T>
		Matrix4_T<T> translation(T const & x, T const & y, T const & z)
		{
			return Matrix4_T<T>(
				1,	0,	0,	0,
				0,	1,	0,	0,
				0,	0,	1,	0,
				x,	y,	z,	1);
		}

		template KLAYGE_CORE_API float4x4 translation(float3 const & pos);

		template <typename T>
		Matrix4_T<T> translation(Vector_T<T, 3> const & pos)
		{
			return translation(pos.x(), pos.y(), pos.z());
		}

		template KLAYGE_CORE_API float4x4 transpose(float4x4 const & rhs);

		template <typename T>
		Matrix4_T<T> transpose(Matrix4_T<T> const & rhs)
		{
			return Matrix4_T<T>(
				rhs(0, 0), rhs(1, 0), rhs(2, 0), rhs(3, 0),
				rhs(0, 1), rhs(1, 1), rhs(2, 1), rhs(3, 1),
				rhs(0, 2), rhs(1, 2), rhs(2, 2), rhs(3, 2),
				rhs(0, 3), rhs(1, 3), rhs(2, 3), rhs(3, 3));
		}

		template KLAYGE_CORE_API float4x4 lh_to_rh(float4x4 const & rhs);

		template <typename T>
		Matrix4_T<T> lh_to_rh(Matrix4_T<T> const & rhs)
		{
			Matrix4_T<T> ret = rhs;
			ret(2, 0) = -ret(2, 0);
			ret(2, 1) = -ret(2, 1);
			ret(2, 2) = -ret(2, 2);
			ret(2, 3) = -ret(2, 3);
			return ret;
		}

		template KLAYGE_CORE_API void decompose(float3& scale, Quaternion& rot, float3& trans, float4x4 const & rhs);

		template <typename T>
		void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, Matrix4_T<T> const & rhs)
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

		template KLAYGE_CORE_API float4x4 transformation(float3 const * scaling_center, Quaternion const * scaling_rotation, float3 const * scale,
			float3 const * rotation_center, Quaternion const * rotation, float3 const * trans);

		template <typename T>
		Matrix4_T<T> transformation(Vector_T<T, 3> const * scaling_center, Quaternion_T<T> const * scaling_rotation, Vector_T<T, 3> const * scale,
			Vector_T<T, 3> const * rotation_center, Quaternion_T<T> const * rotation, Vector_T<T, 3> const * trans)
		{
			Vector_T<T, 3> psc, prc, pt;
			if (!scaling_center)
			{
				psc = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			else
			{
				psc = *scaling_center;
			}
			if (!rotation_center)
			{
				prc = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			else
			{
				prc = *rotation_center;
			}
			if (!trans)
			{
				pt = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			else
			{
				pt = *trans;
			}

			Matrix4_T<T> m1, m2, m3, m4, m5, m6, m7;
			m1 = translation(-psc);
			if (!scaling_rotation)
			{
				m2 = m4 = Matrix4_T<T>::Identity();
			}
			else
			{
				m4 = to_matrix(*scaling_rotation);
				m2 = inverse(m4);
			}
			if (!scale)
			{
				m3 = Matrix4_T<T>::Identity();
			}
			else
			{
				m3 = scaling(*scale);
			}
			if (!rotation)
			{
				m6 = Matrix4_T<T>::Identity();
			}
			else
			{
				m6 = to_matrix(*rotation);
			}
			m5 = translation(psc - prc);
			m7 = translation(prc + pt);

			return m1 * m2 * m3 * m4 * m5 * m6 * m7;
		}

		template KLAYGE_CORE_API float4x4 ortho_rh(float const & width, float const & height, float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> ortho_rh(T const & width, T const & height, T const & nearPlane, T const & farPlane)
		{
			return lh_to_rh(ortho_lh(width, height, nearPlane, farPlane));
		}

		template KLAYGE_CORE_API float4x4 ortho_off_center_rh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane);
		
		template <typename T>
		Matrix4_T<T> ortho_off_center_rh(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane)
		{
			return lh_to_rh(ortho_off_center_lh(left, right, bottom, top, nearPlane, farPlane));
		}

		template KLAYGE_CORE_API float4x4 perspective_rh(float const & width, float const & height,
			float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> perspective_rh(T const & width, T const & height,
			T const & nearPlane, T const & farPlane)
		{
			return lh_to_rh(perspective_lh(width, height, nearPlane, farPlane));
		}

		template KLAYGE_CORE_API float4x4 perspective_fov_rh(float const & fov, float const & aspect,
			float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> perspective_fov_rh(T const & fov, T const & aspect,
			T const & nearPlane, T const & farPlane)
		{
			return lh_to_rh(perspective_fov_lh(fov, aspect, nearPlane, farPlane));
		}

		template KLAYGE_CORE_API float4x4 perspective_off_center_rh(float const & left, float const & right, float const & bottom, float const & top,
			float const & nearPlane, float const & farPlane);

		template <typename T>
		Matrix4_T<T> perspective_off_center_rh(T const & left, T const & right, T const & bottom, T const & top,
			T const & nearPlane, T const & farPlane)
		{
			return lh_to_rh(perspective_off_center_lh(left, right, bottom, top, nearPlane, farPlane));
		}

		template KLAYGE_CORE_API float4x4 rh_to_lh(float4x4 const & rhs);

		template <typename T>
		Matrix4_T<T> rh_to_lh(Matrix4_T<T> const & rhs)
		{
			return lh_to_rh(rhs);
		}

		template KLAYGE_CORE_API float4x4 rotation_matrix_yaw_pitch_roll(float3 const & ang);

		template <typename T>
		Matrix4_T<T> rotation_matrix_yaw_pitch_roll(Vector_T<T, 3> const & ang)
		{
			return rotation_matrix_yaw_pitch_roll(ang.x(), ang.y(), ang.z());
		}


		template KLAYGE_CORE_API Quaternion conjugate(Quaternion const & rhs);

		template <typename T>
		Quaternion_T<T> conjugate(Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
		}

		template KLAYGE_CORE_API Quaternion axis_to_axis(float3 const & from, float3 const & to);

		template <typename T>
		Quaternion_T<T> axis_to_axis(Vector_T<T, 3> const & from, Vector_T<T, 3> const & to)
		{
			Vector_T<T, 3> a(normalize(from));
			Vector_T<T, 3> b(normalize(to));

			return unit_axis_to_unit_axis(a, b);
		}

		template KLAYGE_CORE_API Quaternion unit_axis_to_unit_axis(float3 const & from, float3 const & to);

		template <typename T>
		Quaternion_T<T> unit_axis_to_unit_axis(Vector_T<T, 3> const & from, Vector_T<T, 3> const & to)
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
					Vector_T<T, 3> axis = cross(from, to);

					T const sin_theta = sqrt(1 - cos_theta * cos_theta);
					T const sin_half_theta = sqrt((1 - cos_theta) / 2);
					T const cos_half_theta = sin_theta / (2 * sin_half_theta);

					return Quaternion_T<T>(axis * sin_half_theta, cos_half_theta);
				}
			}
		}

		template KLAYGE_CORE_API Quaternion bary_centric(Quaternion const & q1, Quaternion const & q2,
			Quaternion const & q3, Quaternion::value_type const & f, Quaternion::value_type const & g);

		template <typename T>
		Quaternion_T<T> bary_centric(Quaternion_T<T> const & q1, Quaternion_T<T> const & q2,
			Quaternion_T<T> const & q3, typename Quaternion_T<T>::value_type const & f, typename Quaternion_T<T>::value_type const & g)
		{
			T const temp(f + g);
			Quaternion_T<T> qT1(slerp(q1, q2, temp));
			Quaternion_T<T> qT2(slerp(q1, q3, temp));

			return slerp(qT1, qT2, g / temp);
		}

		template KLAYGE_CORE_API Quaternion exp(Quaternion const & rhs);

		template <typename T>
		Quaternion_T<T> exp(Quaternion_T<T> const & rhs)
		{
			T const theta(length(rhs.v()));
			return Quaternion_T<T>(normalize(rhs.v()) * sin(theta), cos(theta));
		}

		template KLAYGE_CORE_API Quaternion ln(Quaternion const & rhs);

		template <typename T>
		Quaternion_T<T> ln(Quaternion_T<T> const & rhs)
		{
			T const theta_2(acos(rhs.w()));
			return Quaternion_T<T>(normalize(rhs.v()) * (theta_2 + theta_2), 0);
		}

		template KLAYGE_CORE_API Quaternion inverse(Quaternion const & rhs);

		template <typename T>
		Quaternion_T<T> inverse(Quaternion_T<T> const & rhs)
		{
			T const inv(T(1) / length(rhs));
			return Quaternion(-rhs.x() * inv, -rhs.y() * inv, -rhs.z() * inv, rhs.w() * inv);
		}

		template KLAYGE_CORE_API Quaternion mul(Quaternion const & lhs, Quaternion const & rhs);

		template <typename T>
		Quaternion_T<T> mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs)
		{
			return Quaternion_T<T>(
				lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
				lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
				lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
				lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
		}

		template KLAYGE_CORE_API Quaternion rotation_quat_yaw_pitch_roll(float const & yaw, float const & pitch, float const & roll);

		template <typename T>
		Quaternion_T<T> rotation_quat_yaw_pitch_roll(T const & yaw, T const & pitch, T const & roll)
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

		template KLAYGE_CORE_API void to_yaw_pitch_roll(float& yaw, float& pitch, float& roll, Quaternion const & quat);

		// From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
		template <typename T>
		void to_yaw_pitch_roll(T& yaw, T& pitch, T& roll, Quaternion_T<T> const & quat)
		{
			T sqx = quat.x() * quat.x();
			T sqy = quat.y() * quat.y();
			T sqz = quat.z() * quat.z();
			T sqw = quat.w() * quat.w();
			T unit = sqx + sqy + sqz + sqw;
			T test = quat.w() * quat.x() - quat.y() * quat.z();
			if (test > T(0.499) * unit)
			{
				// singularity at north pole
				yaw = 2 * atan2(quat.z(), quat.w());
				roll = PI / 2;
				pitch = 0;
			}
			else
			{
				if (test < -T(0.499) * unit)
				{
					// singularity at south pole
					yaw = -2 * atan2(quat.z(), quat.w());
					roll = -PI / 2;
					pitch = 0;
				}
				else
				{
					yaw = atan2(2 * (quat.y() * quat.w() + quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
					pitch = asin(2 * test / unit);
					roll = atan2(2 * (quat.z() * quat.w() + quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
				}
			}
		}

		template KLAYGE_CORE_API void to_axis_angle(float3& vec, float& ang, Quaternion const & quat);

		template <typename T>
		void to_axis_angle(Vector_T<T, 3>& vec, T& ang, Quaternion_T<T> const & quat)
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

		template KLAYGE_CORE_API Quaternion to_quaternion(float4x4 const & mat);

		template <typename T>
		Quaternion_T<T> to_quaternion(Matrix4_T<T> const & mat)
		{
			Quaternion_T<T> quat;
			T s;
			T const tr(mat(0, 0) + mat(1, 1) + mat(2, 2));

			// check the diagonal
			if (tr > 0)
			{
				s = sqrt(tr + 1);
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
					s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);

					quat.y() = s * T(0.5);

					if (!equal<T>(s, 0))
					{
						s = T(0.5) / s;
					}

					quat.w() = (mat(2, 0) - mat(0, 2)) * s;
					quat.z() = (mat(2, 1) + mat(1, 2)) * s;
					quat.x() = (mat(0, 1) + mat(1, 0)) * s;
				}
				else
				{
					if (((mat(1, 1) <= mat(0, 0)) && (mat(2, 2) > mat(0, 0))) || (mat(2, 2) > mat(1, 1)))
					{
						s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

						quat.z() = s * T(0.5);

						if (!equal<T>(s, 0))
						{
							s = T(0.5) / s;
						}

						quat.w() = (mat(0, 1) - mat(1, 0)) * s;
						quat.x() = (mat(0, 2) + mat(2, 0)) * s;
						quat.y() = (mat(1, 2) + mat(2, 1)) * s;
					}
					else
					{
						s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

						quat.x() = s * T(0.5);

						if (!equal<T>(s, 0))
						{
							s = T(0.5) / s;
						}

						quat.w() = (mat(1, 2) - mat(2, 1)) * s;
						quat.y() = (mat(1, 0) + mat(0, 1)) * s;
						quat.z() = (mat(2, 0) + mat(0, 2)) * s;
					}
				}
			}

			return normalize(quat);
		}

		template KLAYGE_CORE_API Quaternion rotation_axis(float3 const & v, float const & angle);

		template <typename T>
		Quaternion_T<T> rotation_axis(Vector_T<T, 3> const & v, T const & angle)
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

		template KLAYGE_CORE_API Quaternion slerp(Quaternion const & lhs, Quaternion const & rhs, float const & slerp);

		template <typename T>
		Quaternion_T<T> slerp(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs, T const & slerp)
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
				scale0 = sin((T(1) - slerp) * omega) * isinom;
				scale1 = sin(slerp * omega) * isinom;
			}
			else
			{
				// LERP is good enough at this distance
				scale0 = T(1) - slerp;
				scale1 = slerp;
			}

			// Compute the result
			return scale0 * lhs + dir * scale1 * rhs;
		}

		template KLAYGE_CORE_API Quaternion rotation_quat_yaw_pitch_roll(float3 const & ang);

		template <typename T>
		Quaternion_T<T> rotation_quat_yaw_pitch_roll(Vector_T<T, 3> const & ang)
		{
			return rotation_quat_yaw_pitch_roll(ang.x(), ang.y(), ang.z());
		}

		template KLAYGE_CORE_API float dot(Plane const & lhs, float4 const & rhs);

		template <typename T>
		T dot(Plane_T<T> const & lhs, Vector_T<T, 4> const & rhs)
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d() * rhs.w();
		}

		template KLAYGE_CORE_API float dot_coord(Plane const & lhs, float3 const & rhs);

		template <typename T>
		T dot_coord(Plane_T<T> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z() + lhs.d();
		}

		template KLAYGE_CORE_API float dot_normal(Plane const & lhs, float3 const & rhs);

		template <typename T>
		T dot_normal(Plane_T<T> const & lhs, Vector_T<T, 3> const & rhs)
		{
			return lhs.a() * rhs.x() + lhs.b() * rhs.y() + lhs.c() * rhs.z();
		}

		template KLAYGE_CORE_API Plane from_point_normal(float3 const & point, float3 const & normal);

		template <typename T>
		Plane_T<T> from_point_normal(Vector_T<T, 3> const & point, Vector_T<T, 3> const & normal)
		{
			return Plane(normal.x(), normal.y(), normal.z(), -dot(point, normal));
		}

		template KLAYGE_CORE_API Plane from_points(float3 const & v0, float3 const & v1, float3 const & v2);

		template <typename T>
		Plane_T<T> from_points(Vector_T<T, 3> const & v0, Vector_T<T, 3> const & v1, Vector_T<T, 3> const & v2)
		{
			Vector_T<T, 3> vec(cross(v1 - v0, v2 - v0));
			return from_point_normal(v0, normalize(vec));
		}

		template KLAYGE_CORE_API Plane mul(Plane const & p, float4x4 const & mat);

		template <typename T>
		Plane_T<T> mul(Plane_T<T> const & p, Matrix4_T<T> const & mat)
		{
			return Plane_T<T>(
				p.a() * mat(0, 0) + p.b() * mat(1, 0) + p.c() * mat(2, 0) + p.d() * mat(3, 0),
				p.a() * mat(0, 1) + p.b() * mat(1, 1) + p.c() * mat(2, 1) + p.d() * mat(3, 1),
				p.a() * mat(0, 2) + p.b() * mat(1, 2) + p.c() * mat(2, 2) + p.d() * mat(3, 2),
				p.a() * mat(0, 3) + p.b() * mat(1, 3) + p.c() * mat(2, 3) + p.d() * mat(3, 3));
		}

		template KLAYGE_CORE_API float intersect_ray(Plane const & p, float3 const & orig, float3 const & dir);

		// 求直线和平面的交点，直线orig + t * dir
		template <typename T>
		T intersect_ray(Plane_T<T> const & p, Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir)
		{
			T deno(dot(dir, p.Normal()));
			if (equal(deno, T(0)))
			{
				deno = T(0.0001);
			}

			return -dot_coord(p, orig) / deno;
		}

		template KLAYGE_CORE_API void oblique_clipping(float4x4& proj, Plane const & clip_plane);
		
		// From Game Programming Gems 5, Section 2.6.
		template <typename T>
		void oblique_clipping(Matrix4_T<T>& proj, Plane_T<T> const & clip_plane)
		{
			Vector_T<T, 4> q;
			q.x() = (MathLib::sgn(clip_plane.a()) - proj(2, 0)) / proj(0, 0);
			q.y() = (MathLib::sgn(clip_plane.b()) - proj(2, 1)) / proj(1, 1);
			q.z() = T(1);
			q.w() = (T(1) - proj(2, 2)) / proj(3, 2);

			T c = T(1) / MathLib::dot(clip_plane, q);

			proj(0, 2) = clip_plane.a() * c;
			proj(1, 2) = clip_plane.b() * c;
			proj(2, 2) = clip_plane.c() * c;
			proj(3, 2) = clip_plane.d() * c;
		}


		template KLAYGE_CORE_API Color negative(Color const & rhs);

		template <typename T>
		Color_T<T> negative(Color_T<T> const & rhs)
		{
			return Color_T<T>(1 - rhs.r(), 1 - rhs.g(), 1 - rhs.b(), rhs.a());
		}

		template KLAYGE_CORE_API Color modulate(Color const & lhs, Color const & rhs);

		template <typename T>
		Color_T<T> modulate(Color_T<T> const & lhs, Color_T<T> const & rhs)
		{
			return Color_T<T>(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
		}


		template KLAYGE_CORE_API bool vec_in_sphere(Sphere const & sphere, float3 const & v);

		template <typename T>
		bool vec_in_sphere(Sphere_T<T> const & sphere, Vector_T<T, 3> const & v)
		{
			if (length(v - sphere.Center()) < sphere.Radius())
			{
				return true;
			}
			return false;
		}

		template KLAYGE_CORE_API bool intersect_ray(Sphere const & sphere, float3 const & orig, float3 const & dir);

		template <typename T>
		bool intersect_ray(Sphere_T<T> const & sphere, Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir)
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

		template KLAYGE_CORE_API bool vec_in_box(Box const & box, float3 const & v);

		template <typename T>
		bool vec_in_box(Box_T<T> const & box, Vector_T<T, 3> const & v)
		{
			return (in_bound(v.x(), box.Min().x(), box.Max().x()))
				&& (in_bound(v.y(), box.Min().y(), box.Max().y()))
				&& (in_bound(v.z(), box.Min().z(), box.Max().z()));
		}

		template KLAYGE_CORE_API bool intersect_ray(Box const & box, float3 const & orig, float3 const & dir);

		template <typename T>
		bool intersect_ray(Box_T<T> const & box, Vector_T<T, 3> const & orig, Vector_T<T, 3> const & dir)
		{
			float t_near = -1e10f;
			float t_far = +1e10f;

			for (int i = 0; i < 3; ++ i)
			{
				if (equal(dir[i], T(0)))
				{
					if ((dir[i] < box.Min()[i]) || (dir[i] > box.Max()[i]))
					{
						return false;
					}
				}
				else
				{
					float t1 = (box.Min()[i] - orig[i]) / dir[i];
					float t2 = (box.Max()[i] - orig[i]) / dir[i];
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


		template KLAYGE_CORE_API void intersect(float3 const & v0, float3 const & v1, float3 const & v2,
						float3 const & ray_orig, float3 const & ray_dir,
						float& t, float& u, float& v);

		template <typename T>
		void intersect(Vector_T<T, 3> const & v0, Vector_T<T, 3> const & v1, Vector_T<T, 3> const & v2,
						Vector_T<T, 3> const & ray_orig, Vector_T<T, 3> const & ray_dir,
						T& t, T& u, T& v)
		{
			// Find vectors for two edges sharing vert0
			Vector_T<T, 3> edge1 = v1 - v0;
			Vector_T<T, 3> edge2 = v2 - v0;

			// Begin calculating determinant - also used to calculate U parameter
			Vector_T<T, 3> pvec(cross(ray_dir, edge2));

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
			Vector_T<T, 3> qvec(cross(tvec, edge1));

			// Calculate V parameter
			v = dot(ray_dir, qvec);

			// Calculate t, scale parameters, ray intersects triangle
			t = dot(edge2, qvec);

			T const inv_det = T(1) / det;
			v *= inv_det;
			u *= inv_det;
			t *= inv_det;
		}

		template KLAYGE_CORE_API bool bary_centric_in_triangle(float const & u, float const & v);

		template <typename T>
		bool bary_centric_in_triangle(T const & u, T const & v)
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

		float linear_to_srgb(float linear)
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

		float srgb_to_linear(float srgb)
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
	}
}
