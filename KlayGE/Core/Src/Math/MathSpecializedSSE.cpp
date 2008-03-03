// MathSSE.cpp
// KlayGE 数学函数库SSE优化版 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.2.24)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/detail/MathSpecialized.hpp>

#include <xmmintrin.h>

namespace KlayGE
{
	namespace detail
	{
		MathSpecializedSSE::~MathSpecializedSSE()
		{
		}

		float2 MathSpecializedSSE::maximize(float2 const & lhs, float2 const & rhs)
		{
			__m128 l = _mm_set_ps(0, 0, lhs.y(), lhs.x());
			__m128 r = _mm_set_ps(0, 0, rhs.y(), rhs.x());
			__m128 e = _mm_max_ps(l, r);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return float2(ret.x(), ret.y());
		}

		float2 MathSpecializedSSE::minimize(float2 const & lhs, float2 const & rhs)
		{
			__m128 l = _mm_set_ps(0, 0, lhs.y(), lhs.x());
			__m128 r = _mm_set_ps(0, 0, rhs.y(), rhs.x());
			__m128 e = _mm_min_ps(l, r);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return float2(ret.x(), ret.y());
		}

		float3 MathSpecializedSSE::maximize(float3 const & lhs, float3 const & rhs)
		{
			__m128 l = _mm_set_ps(0, lhs.z(), lhs.y(), lhs.x());
			__m128 r = _mm_set_ps(0, rhs.z(), rhs.y(), rhs.x());
			__m128 e = _mm_max_ps(l, r);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return float3(ret.x(), ret.y(), ret.z());
		}

		float3 MathSpecializedSSE::minimize(float3 const & lhs, float3 const & rhs)
		{
			__m128 l = _mm_set_ps(0, lhs.z(), lhs.y(), lhs.x());
			__m128 r = _mm_set_ps(0, rhs.z(), rhs.y(), rhs.x());
			__m128 e = _mm_min_ps(l, r);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return float3(ret.x(), ret.y(), ret.z());
		}

		float4 MathSpecializedSSE::maximize(float4 const & lhs, float4 const & rhs)
		{
			__m128 l = _mm_loadu_ps(&lhs[0]);
			__m128 r = _mm_loadu_ps(&rhs[0]);
			__m128 e = _mm_max_ps(l, r);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return ret;
		}

		float4 MathSpecializedSSE::minimize(float4 const & lhs, float4 const & rhs)
		{
			__m128 l = _mm_loadu_ps(&lhs[0]);
			__m128 r = _mm_loadu_ps(&rhs[0]);
			__m128 e = _mm_min_ps(l, r);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return ret;
		}

		float2 MathSpecializedSSE::normalize(float2 const & rhs)
		{
			__m128 r = _mm_set_ps(0, 0, rhs.y(), rhs.x());

			__m128 m0 = _mm_mul_ps(r, r);		// r0 * r0 | r1 * r1 | r2 * r2 | r3 * r3
			__m128 m1 = _mm_movehl_ps(m0, m0);	// r2 * r2 | r3 * r3 | r2 * r2 | r3 * r3
			__m128 m2 = _mm_add_ps(m0, m1);		// r0 * r0 + r2 * r2 | r1 * r1 + r3 * r3 | ??? | ???
			m0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 1, 1, 1));	// r1 * r1 + r3 * r3 | ??? | ??? | ???
			m1 = _mm_add_ss(m0, m2); // r0 * r0 + r2 * r2 + r1 * r1 + r3 * r3
			__m128 w = _mm_rsqrt_ss(m1);
			w = _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0));
			r = _mm_mul_ps(r, w);

			float4 ret;
			_mm_storeu_ps(&ret[0], r);
			return float2(ret.x(), ret.y());
		}

		float3 MathSpecializedSSE::normalize(float3 const & rhs)
		{
			__m128 r = _mm_set_ps(0, rhs.z(), rhs.y(), rhs.x());

			__m128 m0 = _mm_mul_ps(r, r);		// r0 * r0 | r1 * r1 | r2 * r2 | r3 * r3
			__m128 m1 = _mm_movehl_ps(m0, m0);	// r2 * r2 | r3 * r3 | r2 * r2 | r3 * r3
			__m128 m2 = _mm_add_ps(m0, m1);		// r0 * r0 + r2 * r2 | r1 * r1 + r3 * r3 | ??? | ???
			m0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 1, 1, 1));	// r1 * r1 + r3 * r3 | ??? | ??? | ???
			m1 = _mm_add_ss(m0, m2); // r0 * r0 + r2 * r2 + r1 * r1 + r3 * r3
			__m128 w = _mm_rsqrt_ss(m1);
			w = _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0));
			r = _mm_mul_ps(r, w);

			float4 ret;
			_mm_storeu_ps(&ret[0], r);
			return float3(ret.x(), ret.y(), ret.z());
		}

		float4 MathSpecializedSSE::normalize(float4 const & rhs)
		{
			__m128 r = _mm_loadu_ps(&rhs[0]);

			__m128 m0 = _mm_mul_ps(r, r);		// r0 * r0 | r1 * r1 | r2 * r2 | r3 * r3
			__m128 m1 = _mm_movehl_ps(m0, m0);	// r2 * r2 | r3 * r3 | r2 * r2 | r3 * r3
			__m128 m2 = _mm_add_ps(m0, m1);		// r0 * r0 + r2 * r2 | r1 * r1 + r3 * r3 | ??? | ???
			m0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 1, 1, 1));	// r1 * r1 + r3 * r3 | ??? | ??? | ???
			m1 = _mm_add_ss(m0, m2); // r0 * r0 + r2 * r2 + r1 * r1 + r3 * r3
			__m128 w = _mm_rsqrt_ss(m1);
			w = _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0));
			r = _mm_mul_ps(r, w);

			float4 ret;
			_mm_storeu_ps(&ret[0], r);
			return ret;
		}

		float MathSpecializedSSE::dot(float2 const & lhs, float2 const & rhs)
		{
			__m128 l = _mm_set_ps(0, 0, lhs.y(), lhs.x());
			__m128 r = _mm_set_ps(0, 0, rhs.y(), rhs.x());
			__m128 m0 = _mm_mul_ps(l, r);		// l0 * r0 | l1 * r1 | l2 * r2 | l3 * r3
			__m128 m1 = _mm_movehl_ps(m0, m0);	// l2 * r2 | l3 * r3 | l2 * r2 | l3 * r3
			__m128 m2 = _mm_add_ps(m0, m1);		// l0 * r0 + l2 * r2 | l1 * r1 + l3 * r3 | ??? | ???
			m0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 1, 1, 1));	// l1 * r1 + l3 * r3 | ??? | ??? | ???
			m1 = _mm_add_ss(m0, m2); // l0 * r0 + l2 * r2 + l1 * r1 + l3 * r3

			float ret;
			_mm_store_ss(&ret, m1);
			return ret;
		}

		float MathSpecializedSSE::dot(float3 const & lhs, float3 const & rhs)
		{
			__m128 l = _mm_set_ps(0, lhs.z(), lhs.y(), lhs.x());
			__m128 r = _mm_set_ps(0, rhs.z(), rhs.y(), rhs.x());
			__m128 m0 = _mm_mul_ps(l, r);		// l0 * r0 | l1 * r1 | l2 * r2 | l3 * r3
			__m128 m1 = _mm_movehl_ps(m0, m0);	// l2 * r2 | l3 * r3 | l2 * r2 | l3 * r3
			__m128 m2 = _mm_add_ps(m0, m1);		// l0 * r0 + l2 * r2 | l1 * r1 + l3 * r3 | ??? | ???
			m0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 1, 1, 1));	// l1 * r1 + l3 * r3 | ??? | ??? | ???
			m1 = _mm_add_ss(m0, m2); // l0 * r0 + l2 * r2 + l1 * r1 + l3 * r3

			float ret;
			_mm_store_ss(&ret, m1);
			return ret;
		}

		float MathSpecializedSSE::dot(float4 const & lhs, float4 const & rhs)
		{
			__m128 l = _mm_loadu_ps(&lhs[0]);
			__m128 r = _mm_loadu_ps(&rhs[0]);
			__m128 m0 = _mm_mul_ps(l, r);		// l0 * r0 | l1 * r1 | l2 * r2 | l3 * r3
			__m128 m1 = _mm_movehl_ps(m0, m0);	// l2 * r2 | l3 * r3 | l2 * r2 | l3 * r3
			__m128 m2 = _mm_add_ps(m0, m1);		// l0 * r0 + l2 * r2 | l1 * r1 + l3 * r3 | ??? | ???
			m0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 1, 1, 1));	// l1 * r1 + l3 * r3 | ??? | ??? | ???
			m1 = _mm_add_ss(m0, m2); // l0 * r0 + l2 * r2 + l1 * r1 + l3 * r3

			float ret;
			_mm_store_ss(&ret, m1);
			return ret;
		}

		float4 MathSpecializedSSE::transform(float4 const & v, float4x4 const & mat)
		{
			__m128 r0 = _mm_loadu_ps(&mat[0]);
			__m128 r1 = _mm_loadu_ps(&mat[4]);
			__m128 r2 = _mm_loadu_ps(&mat[8]);
			__m128 r3 = _mm_loadu_ps(&mat[12]);

			__m128 l = _mm_loadu_ps(&v[0]);

			__m128 m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);		// l0 * r00 | l0 * r01 | l0 * r02 | l0 * r03
			__m128 m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);		// l1 * r10 | l1 * r11 | l1 * r12 | l1 * r13
			m0 = _mm_add_ps(m0, m1);		// l0 * r00 + l1 * r10 | l0 * r01 + l1 * r11 | l0 * r02 + l1 * r12 | l0 * r03 + l1 * r13
			__m128 m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);		// l2 * r20 | l2 * r21 | l2 * r22 | l2 * r23
			__m128 m3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);		// l3 * r30 | l3 * r31 | l3 * r32 | l3 * r33
			m2 = _mm_add_ps(m2, m3);		// l2 * r20 + l3 * r30 | l2 * r21 + l3 * r31 | l2 * r22 + l3 * r32 | l2 * r23 + l3 * r33
			__m128 e = _mm_add_ps(m0, m2);	// l0 * r00 + l1 * r10 + l2 * r20 + l3 * r30 | l0 * r01 + l1 * r11 + l2 * r21 + l3 * r31 | l0 * r02 + l1 * r12 + l2 * r22 + l3 * r32 | l0 * r03 + l1 * r13 + l2 * r23 + l3 * r33

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return ret;
		}

		float4 MathSpecializedSSE::transform(float3 const & v, float4x4 const & mat)
		{
			__m128 r0 = _mm_loadu_ps(&mat[0]);
			__m128 r1 = _mm_loadu_ps(&mat[4]);
			__m128 r2 = _mm_loadu_ps(&mat[8]);
			__m128 r3 = _mm_loadu_ps(&mat[12]);

			__m128 l = _mm_set_ps(1, v.z(), v.y(), v.x());

			__m128 m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);		// l0 * r00 | l0 * r01 | l0 * r02 | l0 * r03
			__m128 m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);		// l1 * r10 | l1 * r11 | l1 * r12 | l1 * r13
			m0 = _mm_add_ps(m0, m1);		// l0 * r00 + l1 * r10 | l0 * r01 + l1 * r11 | l0 * r02 + l1 * r12 | l0 * r03 + l1 * r13
			__m128 m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);		// l2 * r20 | l2 * r21 | l2 * r22 | l2 * r23
			m2 = _mm_add_ps(m2, r3);		// l2 * r20 + r30 | l2 * r21 + r31 | l2 * r22 + r32 | l2 * r23 + r33
			__m128 e = _mm_add_ps(m0, m2);	// l0 * r00 + l1 * r10 + l2 * r20 + r30 | l0 * r01 + l1 * r11 + l2 * r21 + r31 | l0 * r02 + l1 * r12 + l2 * r22 + r32 | l0 * r03 + l1 * r13 + l2 * r23 + r33

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return ret;
		}

		float3 MathSpecializedSSE::transform_coord(float3 const & v, float4x4 const & mat)
		{
			__m128 r0 = _mm_loadu_ps(&mat[0]);
			__m128 r1 = _mm_loadu_ps(&mat[4]);
			__m128 r2 = _mm_loadu_ps(&mat[8]);
			__m128 r3 = _mm_loadu_ps(&mat[12]);

			__m128 l = _mm_set_ps(1, v.z(), v.y(), v.x());

			__m128 m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);		// l0 * r00 | l0 * r01 | l0 * r02 | l0 * r03
			__m128 m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);		// l1 * r10 | l1 * r11 | l1 * r12 | l1 * r13
			m0 = _mm_add_ps(m0, m1);		// l0 * r00 + l1 * r10 | l0 * r01 + l1 * r11 | l0 * r02 + l1 * r12 | l0 * r03 + l1 * r13
			__m128 m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);		// l2 * r20 | l2 * r21 | l2 * r22 | l2 * r23
			m2 = _mm_add_ps(m2, r3);		// l2 * r20 + r30 | l2 * r21 + r31 | l2 * r22 + r32 | l2 * r23 + r33
			__m128 e = _mm_add_ps(m0, m2);	// l0 * r00 + l1 * r10 + l2 * r20 + r30 | l0 * r01 + l1 * r11 + l2 * r21 + r31 | l0 * r02 + l1 * r12 + l2 * r22 + r32 | l0 * r03 + l1 * r13 + l2 * r23 + r33
			__m128 w = _mm_shuffle_ps(e, e, _MM_SHUFFLE(3, 3, 3, 3));
			w = _mm_rcp_ps(w);
			e = _mm_mul_ps(e, w);

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return float3(ret.x(), ret.y(), ret.z());
		}

		float3 MathSpecializedSSE::transform_normal(float3 const & v, float4x4 const & mat)
		{
			__m128 r0 = _mm_loadu_ps(&mat[0]);
			__m128 r1 = _mm_loadu_ps(&mat[4]);
			__m128 r2 = _mm_loadu_ps(&mat[8]);

			__m128 l = _mm_set_ps(0, v.z(), v.y(), v.x());

			__m128 m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);		// l0 * r00 | l0 * r01 | l0 * r02 | l0 * r03
			__m128 m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);		// l1 * r10 | l1 * r11 | l1 * r12 | l1 * r13
			m0 = _mm_add_ps(m0, m1);		// l0 * r00 + l1 * r10 | l0 * r01 + l1 * r11 | l0 * r02 + l1 * r12 | l0 * r03 + l1 * r13
			__m128 m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);		// l2 * r20 | l2 * r21 | l2 * r22 | l2 * r23
			__m128 e = _mm_add_ps(m0, m2);	// l0 * r00 + l1 * r10 + l2 * r20 | l0 * r01 + l1 * r11 + l2 * r21 | l0 * r02 + l1 * r12 + l2 * r22 | l0 * r03 + l1 * r13 + l2 * r23

			float4 ret;
			_mm_storeu_ps(&ret[0], e);
			return float3(ret.x(), ret.y(), ret.z());
		}

		float4x4 MathSpecializedSSE::mul(float4x4 const & lhs, float4x4 const & rhs)
		{
			__m128 r0 = _mm_loadu_ps(&rhs[0]);
			__m128 r1 = _mm_loadu_ps(&rhs[4]);
			__m128 r2 = _mm_loadu_ps(&rhs[8]);
			__m128 r3 = _mm_loadu_ps(&rhs[12]);

			__m128 l = _mm_loadu_ps(&lhs[0]);

			__m128 m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);		// l00 * r00 | l00 * r01 | l00 * r02 | l00 * r03
			__m128 m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);		// l01 * r10 | l01 * r11 | l01 * r12 | l01 * r13
			m0 = _mm_add_ps(m0, m1);		// l00 * r00 + l01 * r10 | l00 * r01 + l01 * r11 | l00 * r02 + l01 * r12 | l00 * r03 + l01 * r13
			__m128 m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);		// l02 * r20 | l02 * r21 | l02 * r22 | l02 * r23
			__m128 m3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);		// l03 * r30 | l03 * r31 | l03 * r32 | l03 * r33
			m2 = _mm_add_ps(m2, m3);		// l02 * r20 + l03 * r30 | l02 * r21 + l03 * r31 | l02 * r22 + l03 * r32 | l02 * r23 + l03 * r33
			__m128 e0 = _mm_add_ps(m0, m2);	// l00 * r00 + l01 * r10 + l02 * r20 + l03 * r30 | l00 * r01 + l01 * r11 + l02 * r21 + l03 * r31 | l00 * r02 + l01 * r12 + l02 * r22 + l03 * r32 | l00 * r03 + l01 * r13 + l02 * r23 + l03 * r33

			l = _mm_loadu_ps(&lhs[4]);

			m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
			m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
			m0 = _mm_add_ps(m0, m1);
			m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
			m3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
			m2 = _mm_add_ps(m2, m3);
			__m128 e1 = _mm_add_ps(m0, m2);

			l = _mm_loadu_ps(&lhs[8]);

			m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
			m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
			m0 = _mm_add_ps(m0, m1);
			m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
			m3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
			m2 = _mm_add_ps(m2, m3);
			__m128 e2 = _mm_add_ps(m0, m2);

			l = _mm_loadu_ps(&lhs[12]);

			m0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
			m1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
			m0 = _mm_add_ps(m0, m1);
			m2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
			m3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
			m2 = _mm_add_ps(m2, m3);
			__m128 e3 = _mm_add_ps(m0, m2);

			float4x4 ret;
			_mm_storeu_ps(&ret[0], e0);
			_mm_storeu_ps(&ret[4], e1);
			_mm_storeu_ps(&ret[8], e2);
			_mm_storeu_ps(&ret[12], e3);
			return ret;
		}

		float4x4 MathSpecializedSSE::transpose(float4x4 const & rhs)
		{
			__m128 r0 = _mm_loadu_ps(&rhs[0]);
			__m128 r1 = _mm_loadu_ps(&rhs[4]);
			__m128 r2 = _mm_loadu_ps(&rhs[8]);
			__m128 r3 = _mm_loadu_ps(&rhs[12]);
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			float4x4 ret;
			_mm_storeu_ps(&ret[0], r0);
			_mm_storeu_ps(&ret[4], r1);
			_mm_storeu_ps(&ret[8], r2);
			_mm_storeu_ps(&ret[12], r3);
			return ret;
		}
	}
}
