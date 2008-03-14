// D3D9Mapping.hpp
// KlayGE RenderEngine和D3D9本地之间的映射 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.19)
//
// 2.4.0
// 初次建立 (2005.3.20)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9MAPPING_HPP
#define _D3D9MAPPING_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderLayout.hpp>

namespace KlayGE
{
	class D3D9Mapping
	{
	public:
		static D3DMATRIX Mapping(float4x4 const & mat);
		static float4x4 Mapping(D3DMATRIX const & mat);
		static D3DVECTOR Mapping(float3 const & vec);
		static D3DCOLOR MappingToUInt32Color(Color const & clr);
		static D3DCOLORVALUE MappingToFloat4Color(Color const & clr);
		static uint32_t MappingColorMask(uint32_t mask);

		static D3DCMPFUNC Mapping(RenderStateObject::CompareFunction func);

		static D3DSTENCILOP Mapping(RenderStateObject::StencilOperation op);

		static uint32_t Mapping(RenderStateObject::AlphaBlendFactor factor);
		static uint32_t Mapping(RenderStateObject::CullMode mode);
		static uint32_t Mapping(RenderStateObject::PolygonMode mode);
		static uint32_t Mapping(RenderStateObject::ShadeMode mode);
		static uint32_t Mapping(RenderStateObject::BlendOperation bo);
		static uint32_t Mapping(Sampler::TexAddressingMode mode);

		static void Mapping(D3DPRIMITIVETYPE& primType, uint32_t& primCount, RenderLayout const & rl);
		static void Mapping(std::vector<D3DVERTEXELEMENT9>& elements, size_t stream, vertex_elements_type const & vet);

		static RenderDeviceCaps Mapping(D3DCAPS9 const & d3d_caps, uint32_t adaptor_no, D3DDEVTYPE device_type);

		static D3DFORMAT MappingFormat(ElementFormat pf);
		static ElementFormat MappingFormat(D3DFORMAT d3dfmt);
	};
}

#endif			// _D3D9MAPPING_HPP
