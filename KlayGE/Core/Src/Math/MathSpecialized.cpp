// MathSpecialized.cpp
// KlayGE 数学函数库特化版 实现文件
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
#include <KlayGE/CPUInfo.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/detail/MathSpecialized.hpp>

namespace
{
	using namespace KlayGE;
	using namespace KlayGE::detail;

	float2 self_init_maximize_float2(float2 const & lhs, float2 const & rhs);
	float2 self_init_minimize_float2(float2 const & lhs, float2 const & rhs);
	float3 self_init_maximize_float3(float3 const & lhs, float3 const & rhs);
	float3 self_init_minimize_float3(float3 const & lhs, float3 const & rhs);
	float4 self_init_maximize_float4(float4 const & lhs, float4 const & rhs);
	float4 self_init_minimize_float4(float4 const & lhs, float4 const & rhs);

	float2 self_init_normalize_float2(float2 const & rhs);
	float3 self_init_normalize_float3(float3 const & rhs);
	float4 self_init_normalize_float4(float4 const & rhs);

	float self_init_dot_float2(float2 const & lhs, float2 const & rhs);
	float self_init_dot_float3(float3 const & lhs, float3 const & rhs);
	float self_init_dot_float4(float4 const & lhs, float4 const & rhs);

	float4 self_init_transform_float3(float3 const & v, float4x4 const & mat);
	float4 self_init_transform_float4(float4 const & v, float4x4 const & mat);
	float3 self_init_transform_coord_float3(float4 const & v, float4x4 const & mat);
	float3 self_init_transform_normal_float3(float3 const & v, float4x4 const & mat);

	float4x4 self_init_mul_float4x4(float4x4 const & lhs, float4x4 const & rhs);
	float4x4 self_init_transpose_float4x4(float4x4 const & rhs);

	void init_math_funcs()
	{
		using namespace KlayGE::MathLib;

		{
			maximize_float2 = maximize<float2>;
			minimize_float2 = minimize<float2>;
			maximize_float3 = maximize<float3>;
			minimize_float3 = minimize<float3>;
			maximize_float4 = maximize<float4>;
			minimize_float4 = minimize<float4>;

			normalize_float2 = normalize<float2>;
			normalize_float3 = normalize<float3>;
			normalize_float4 = normalize<float4>;

			dot_float2 = dot<float2>;
			dot_float3 = dot<float3>;
			dot_float4 = dot<float4>;

			transform_float3 = transform<float3>;
			transform_float4 = transform<float4>;
			transform_coord_float3 = transform_coord<float3>;
			transform_normal_float3 = transform_normal<float3>;

			mul_float4x4 = mul<float>;
			transpose_float4x4 = transpose<float>;
		}

		CPUInfo cpuinfo;
		if (cpuinfo.IsFeatureSupport(CPUInfo::CF_SSE))
		{
			maximize_float2 = maximize_float2_sse;
			minimize_float2 = minimize_float2_sse;
			maximize_float3 = maximize_float3_sse;
			minimize_float3 = minimize_float3_sse;
			maximize_float4 = maximize_float4_sse;
			minimize_float4 = minimize_float4_sse;

			normalize_float2 = normalize_float2_sse;
			normalize_float3 = normalize_float3_sse;
			normalize_float4 = normalize_float4_sse;

			dot_float2 = dot_float2_sse;
			dot_float3 = dot_float3_sse;
			dot_float4 = dot_float4_sse;

			transform_float3 = transform_float3_sse;
			transform_float4 = transform_float4_sse;
			transform_coord_float3 = transform_coord_float3_sse;
			transform_normal_float3 = transform_normal_float3_sse;

			mul_float4x4 = mul_float4x4_sse;
			transpose_float4x4 = transpose_float4x4_sse;
		}
	}

	float2 self_init_maximize_float2(float2 const & lhs, float2 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::maximize_float2(lhs, rhs);
	}

	float2 self_init_minimize_float2(float2 const & lhs, float2 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::minimize_float2(lhs, rhs);
	}

	float3 self_init_maximize_float3(float3 const & lhs, float3 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::maximize_float3(lhs, rhs);
	}
	
	float3 self_init_minimize_float3(float3 const & lhs, float3 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::minimize_float3(lhs, rhs);
	}
	
	float4 self_init_maximize_float4(float4 const & lhs, float4 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::maximize_float4(lhs, rhs);
	}
	
	float4 self_init_minimize_float4(float4 const & lhs, float4 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::minimize_float4(lhs, rhs);
	}

	float2 self_init_normalize_float2(float2 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::normalize_float2(rhs);
	}

	float3 self_init_normalize_float3(float3 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::normalize_float3(rhs);
	}

	float4 self_init_normalize_float4(float4 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::normalize_float4(rhs);
	}

	float self_init_dot_float2(float2 const & lhs, float2 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::dot_float2(lhs, rhs);
	}

	float self_init_dot_float3(float3 const & lhs, float3 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::dot_float3(lhs, rhs);
	}

	float self_init_dot_float4(float4 const & lhs, float4 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::dot_float4(lhs, rhs);
	}

	float4 self_init_transform_float4(float4 const & v, float4x4 const & mat)
	{
		init_math_funcs();
		return KlayGE::MathLib::transform_float4(v, mat);
	}

	float4 self_init_transform_float3(float3 const & v, float4x4 const & mat)
	{
		init_math_funcs();
		return KlayGE::MathLib::transform_float3(v, mat);
	}

	float3 self_init_transform_coord_float3(float3 const & v, float4x4 const & mat)
	{
		init_math_funcs();
		return KlayGE::MathLib::transform_coord_float3(v, mat);
	}

	float3 self_init_transform_normal_float3(float3 const & v, float4x4 const & mat)
	{
		init_math_funcs();
		return KlayGE::MathLib::transform_normal_float3(v, mat);
	}

	float4x4 self_init_mul_float4x4(float4x4 const & lhs, float4x4 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::mul_float4x4(lhs, rhs);
	}

	float4x4 self_init_transpose_float4x4(float4x4 const & rhs)
	{
		init_math_funcs();
		return KlayGE::MathLib::transpose_float4x4(rhs);
	}
}

namespace KlayGE
{
	namespace MathLib
	{
		maximize_float2_func maximize_float2 = self_init_maximize_float2;
		minimize_float2_func minimize_float2 = self_init_minimize_float2;
		maximize_float3_func maximize_float3 = self_init_maximize_float3;
		minimize_float3_func minimize_float3 = self_init_minimize_float3;
		maximize_float4_func maximize_float4 = self_init_maximize_float4;
		minimize_float4_func minimize_float4 = self_init_minimize_float4;

		normalize_float2_func normalize_float2 = self_init_normalize_float2;
		normalize_float3_func normalize_float3 = self_init_normalize_float3;
		normalize_float4_func normalize_float4 = self_init_normalize_float4;

		dot_float2_func dot_float2 = self_init_dot_float2;
		dot_float3_func dot_float3 = self_init_dot_float3;
		dot_float4_func dot_float4 = self_init_dot_float4;

		transform_float3_func transform_float3 = self_init_transform_float3;
		transform_float4_func transform_float4 = self_init_transform_float4;
		transform_coord_float3_func transform_coord_float3 = self_init_transform_coord_float3;
		transform_normal_float3_func transform_normal_float3 = self_init_transform_normal_float3;

		mul_float4x4_func mul_float4x4 = self_init_mul_float4x4;
		transpose_float4x4_func transpose_float4x4 = self_init_transpose_float4x4;
	}
}
