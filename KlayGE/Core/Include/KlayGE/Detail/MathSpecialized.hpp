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
		class MathSpecialized
		{
		public:
			virtual ~MathSpecialized();

			virtual float2 maximize(float2 const & lhs, float2 const & rhs);
			virtual float2 minimize(float2 const & lhs, float2 const & rhs);
			virtual float3 maximize(float3 const & lhs, float3 const & rhs);
			virtual float3 minimize(float3 const & lhs, float3 const & rhs);
			virtual float4 maximize(float4 const & lhs, float4 const & rhs);
			virtual float4 minimize(float4 const & lhs, float4 const & rhs);

			virtual float2 normalize(float2 const & rhs);
			virtual float3 normalize(float3 const & rhs);
			virtual float4 normalize(float4 const & rhs);

			virtual float dot(float2 const & lhs, float2 const & rhs);
			virtual float dot(float3 const & lhs, float3 const & rhs);
			virtual float dot(float4 const & lhs, float4 const & rhs);

			virtual float4 transform(float4 const & v, float4x4 const & mat);
			virtual float4 transform(float3 const & v, float4x4 const & mat);
			virtual float3 transform_coord(float3 const & v, float4x4 const & mat);
			virtual float3 transform_normal(float3 const & v, float4x4 const & mat);

			virtual float4x4 mul(float4x4 const & lhs, float4x4 const & rhs);

			virtual float4x4 transpose(float4x4 const & rhs);
		};

		class MathSpecializedSSE : public MathSpecialized
		{
		public:
			virtual ~MathSpecializedSSE();

			virtual float2 maximize(float2 const & lhs, float2 const & rhs);
			virtual float2 minimize(float2 const & lhs, float2 const & rhs);
			virtual float3 maximize(float3 const & lhs, float3 const & rhs);
			virtual float3 minimize(float3 const & lhs, float3 const & rhs);
			virtual float4 maximize(float4 const & lhs, float4 const & rhs);
			virtual float4 minimize(float4 const & lhs, float4 const & rhs);

			virtual float2 normalize(float2 const & rhs);
			virtual float3 normalize(float3 const & rhs);
			virtual float4 normalize(float4 const & rhs);

			virtual float dot(float2 const & lhs, float2 const & rhs);
			virtual float dot(float3 const & lhs, float3 const & rhs);
			virtual float dot(float4 const & lhs, float4 const & rhs);

			virtual float4 transform(float4 const & v, float4x4 const & mat);
			virtual float4 transform(float3 const & v, float4x4 const & mat);
			virtual float3 transform_coord(float3 const & v, float4x4 const & mat);
			virtual float3 transform_normal(float3 const & v, float4x4 const & mat);

			virtual float4x4 mul(float4x4 const & lhs, float4x4 const & rhs);

			virtual float4x4 transpose(float4x4 const & rhs);
		};
	}
}
