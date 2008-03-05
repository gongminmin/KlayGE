// MathSpecialized.cpp
// KlayGE 数学函数库特化版 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.2.24)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	namespace detail
	{
		float2 maximize_float2_sse(float2 const & lhs, float2 const & rhs);
		float2 minimize_float2_sse(float2 const & lhs, float2 const & rhs);
		float3 maximize_float3_sse(float3 const & lhs, float3 const & rhs);
		float3 minimize_float3_sse(float3 const & lhs, float3 const & rhs);
		float4 maximize_float4_sse(float4 const & lhs, float4 const & rhs);
		float4 minimize_float4_sse(float4 const & lhs, float4 const & rhs);

		float2 normalize_float2_sse(float2 const & rhs);
		float3 normalize_float3_sse(float3 const & rhs);
		float4 normalize_float4_sse(float4 const & rhs);

		float dot_float2_sse(float2 const & lhs, float2 const & rhs);
		float dot_float3_sse(float3 const & lhs, float3 const & rhs);
		float dot_float4_sse(float4 const & lhs, float4 const & rhs);

		float4 transform_float4_sse(float4 const & v, float4x4 const & mat);
		float4 transform_float3_sse(float3 const & v, float4x4 const & mat);
		float3 transform_coord_float3_sse(float3 const & v, float4x4 const & mat);
		float3 transform_normal_float3_sse(float3 const & v, float4x4 const & mat);

		float4x4 mul_float4x4_sse(float4x4 const & lhs, float4x4 const & rhs);

		float4x4 transpose_float4x4_sse(float4x4 const & rhs);
	}
}
