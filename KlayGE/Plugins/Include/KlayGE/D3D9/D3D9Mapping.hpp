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

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Sampler.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9Mapping
	{
	public:
		static D3DMATRIX Mapping(Matrix4 const & mat);
		static Matrix4 Mapping(D3DMATRIX const & mat);
		static D3DVECTOR Mapping(Vector3 const & vec);
		static D3DCOLOR MappingToUInt32Color(Color const & clr);
		static D3DCOLORVALUE MappingToFloat4Color(Color const & clr);

		static D3DCMPFUNC Mapping(RenderEngine::CompareFunction func);

		static D3DSTENCILOP Mapping(RenderEngine::StencilOperation op);

		static uint32_t Mapping(RenderEngine::AlphaBlendFactor factor);
		static uint32_t Mapping(RenderEngine::CullMode mode);
		static uint32_t Mapping(RenderEngine::FillMode mode);
		static uint32_t Mapping(RenderEngine::ShadeOptions so);
		static uint32_t Mapping(Sampler::TexAddressingMode mode);

		static void Mapping(D3DPRIMITIVETYPE& primType, uint32_t& primCount, VertexBuffer const & vb);
		static void Mapping(std::vector<D3DVERTEXELEMENT9>& elements, size_t stream, VertexStream const & vs);

		static RenderDeviceCaps Mapping(D3DCAPS9 const & d3d_caps);
	};
}

#endif			// _D3D9MAPPING_HPP
