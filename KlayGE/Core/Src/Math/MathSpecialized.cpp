// Math.cpp
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

	boost::shared_ptr<MathSpecialized> init_math_funcs()
	{
		boost::shared_ptr<MathSpecialized> ret;

		CPUInfo cpuinfo;
		if (cpuinfo.IsFeatureSupport(CPUInfo::CF_SSE))
		{
			ret.reset(new MathSpecializedSSE);
		}
		else
		{
			ret.reset(new MathSpecialized);
		}

		return ret;
	}

	MathSpecialized& MathSpecializedInstance()
	{
		static boost::shared_ptr<MathSpecialized> instance = init_math_funcs();
		return *instance;
	}
}

namespace KlayGE
{
	namespace detail
	{
		MathSpecialized::~MathSpecialized()
		{
		}

		float2 MathSpecialized::maximize(float2 const & lhs, float2 const & rhs)
		{
			return MathLib::maximize<float2>(lhs, rhs);
		}

		float2 MathSpecialized::minimize(float2 const & lhs, float2 const & rhs)
		{
			return MathLib::minimize<float2>(lhs, rhs);
		}

		float3 MathSpecialized::maximize(float3 const & lhs, float3 const & rhs)
		{
			return MathLib::maximize<float3>(lhs, rhs);
		}
		
		float3 MathSpecialized::minimize(float3 const & lhs, float3 const & rhs)
		{
			return MathLib::minimize<float3>(lhs, rhs);
		}
		
		float4 MathSpecialized::maximize(float4 const & lhs, float4 const & rhs)
		{
			return MathLib::maximize<float4>(lhs, rhs);
		}
		
		float4 MathSpecialized::minimize(float4 const & lhs, float4 const & rhs)
		{
			return MathLib::minimize<float4>(lhs, rhs);
		}

		float2 MathSpecialized::normalize(float2 const & rhs)
		{
			return MathLib::normalize<float2>(rhs);
		}

		float3 MathSpecialized::normalize(float3 const & rhs)
		{
			return MathLib::normalize<float3>(rhs);
		}

		float4 MathSpecialized::normalize(float4 const & rhs)
		{
			return MathLib::normalize<float4>(rhs);
		}

		float MathSpecialized::dot(float2 const & lhs, float2 const & rhs)
		{
			return MathLib::dot<float2>(lhs, rhs);
		}

		float MathSpecialized::dot(float3 const & lhs, float3 const & rhs)
		{
			return MathLib::dot<float3>(lhs, rhs);
		}

		float MathSpecialized::dot(float4 const & lhs, float4 const & rhs)
		{
			return MathLib::dot<float4>(lhs, rhs);
		}

		float4 MathSpecialized::transform(float4 const & v, float4x4 const & mat)
		{
			return MathLib::transform<float4>(v, mat);
		}

		float4 MathSpecialized::transform(float3 const & v, float4x4 const & mat)
		{
			return MathLib::transform<float3>(v, mat);
		}

		float3 MathSpecialized::transform_coord(float3 const & v, float4x4 const & mat)
		{
			return MathLib::transform_coord<float3>(v, mat);
		}

		float3 MathSpecialized::transform_normal(float3 const & v, float4x4 const & mat)
		{
			return MathLib::transform_normal<float3>(v, mat);
		}

		float4x4 MathSpecialized::mul(float4x4 const & lhs, float4x4 const & rhs)
		{
			return MathLib::mul<float>(lhs, rhs);
		}

		float4x4 MathSpecialized::transpose(float4x4 const & rhs)
		{
			return MathLib::transpose<float>(rhs);
		}
	}
}

namespace KlayGE
{
	namespace MathLib
	{
		float2 maximize(float2 const & lhs, float2 const & rhs)
		{
			return MathSpecializedInstance().maximize(lhs, rhs);
		}

		float2 minimize(float2 const & lhs, float2 const & rhs)
		{
			return MathSpecializedInstance().minimize(lhs, rhs);
		}

		float3 maximize(float3 const & lhs, float3 const & rhs)
		{
			return MathSpecializedInstance().maximize(lhs, rhs);
		}

		float3 minimize(float3 const & lhs, float3 const & rhs)
		{
			return MathSpecializedInstance().minimize(lhs, rhs);
		}

		float4 maximize(float4 const & lhs, float4 const & rhs)
		{
			return MathSpecializedInstance().maximize(lhs, rhs);
		}

		float4 minimize(float4 const & lhs, float4 const & rhs)
		{
			return MathSpecializedInstance().minimize(lhs, rhs);
		}

		float2 normalize(float2 const & rhs)
		{
			return MathSpecializedInstance().normalize(rhs);
		}

		float3 normalize(float3 const & rhs)
		{
			return MathSpecializedInstance().normalize(rhs);
		}

		float4 normalize(float4 const & rhs)
		{
			return MathSpecializedInstance().normalize(rhs);
		}

		float dot(float2 const & lhs, float2 const & rhs)
		{
			return MathSpecializedInstance().dot(lhs, rhs);
		}
		
		float dot(float3 const & lhs, float3 const & rhs)
		{
			return MathSpecializedInstance().dot(lhs, rhs);
		}
		
		float dot(float4 const & lhs, float4 const & rhs)
		{
			return MathSpecializedInstance().dot(lhs, rhs);
		}

		float4 transform(float4 const & v, float4x4 const & mat)
		{
			return MathSpecializedInstance().transform(v, mat);
		}

		float4 transform(float3 const & v, float4x4 const & mat)
		{
			return MathSpecializedInstance().transform(v, mat);
		}

		float3 transform_coord(float3 const & v, float4x4 const & mat)
		{
			return MathSpecializedInstance().transform_coord(v, mat);
		}

		float3 transform_normal(float3 const & v, float4x4 const & mat)
		{
			return MathSpecializedInstance().transform_normal(v, mat);
		}

		float4x4 mul(float4x4 const & lhs, float4x4 const & rhs)
		{
			return MathSpecializedInstance().mul(lhs, rhs);
		}

		float4x4 transpose(float4x4 const & rhs)
		{
			return MathSpecializedInstance().transpose(rhs);
		}
	}
}
