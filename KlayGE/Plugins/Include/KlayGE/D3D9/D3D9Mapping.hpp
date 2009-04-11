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

#pragma KLAYGE_ONCE

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderStateObject.hpp>
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

		static D3DCMPFUNC Mapping(CompareFunction func);

		static D3DSTENCILOP Mapping(StencilOperation op);

		static uint32_t Mapping(AlphaBlendFactor factor);
		static uint32_t Mapping(CullMode mode, bool front_face_ccw);
		static uint32_t Mapping(PolygonMode mode);
		static uint32_t Mapping(ShadeMode mode);
		static uint32_t Mapping(BlendOperation bo);
		static uint32_t Mapping(TexAddressingMode mode);

		static void Mapping(D3DPRIMITIVETYPE& primType, uint32_t& primCount, RenderLayout const & rl);
		static void Mapping(std::vector<D3DVERTEXELEMENT9>& elements, size_t stream, vertex_elements_type const & vet);

		static RenderDeviceCaps Mapping(D3DCAPS9 const & d3d_caps);

		static D3DFORMAT MappingFormat(ElementFormat pf);
		static ElementFormat MappingFormat(D3DFORMAT d3dfmt);
	};
}

#endif			// _D3D9MAPPING_HPP
