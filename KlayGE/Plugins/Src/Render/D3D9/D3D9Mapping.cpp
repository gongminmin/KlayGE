// D3D9Mapping.hpp
// KlayGE RenderEngine和D3D9本地之间的映射 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 支持ATI的instancing (2006.1.1)
//
// 3.0.0
// 增加了TAM_Border (2005.8.30)
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.19)
//
// 2.4.0
// 初次建立 (2005.3.20)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/GraphicsBuffer.hpp>

#define NOMINMAX
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <boost/assert.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>

namespace KlayGE
{
	// 从KlayGE的float4x4转换到D3DMATRIX
	/////////////////////////////////////////////////////////////////////////////////
	D3DMATRIX D3D9Mapping::Mapping(float4x4 const & mat)
	{
		D3DMATRIX d3dMat;
		std::copy(mat.begin(), mat.end(), &d3dMat._11);

		return d3dMat;
	}

	// 从D3DMATRIX转换到KlayGE的float4x4
	/////////////////////////////////////////////////////////////////////////////////
	float4x4 D3D9Mapping::Mapping(D3DMATRIX const & mat)
	{
		return float4x4(&mat.m[0][0]);
	}

	// 从KlayGE的Color转换到D3DCOLORVALUE
	/////////////////////////////////////////////////////////////////////////////////
	D3DVECTOR D3D9Mapping::Mapping(float3 const & vec)
	{
		return D3DXVECTOR3(vec.x(), vec.y(), vec.z());
	}

	// 从KlayGE的Color转换到D3DCOLORVALUE
	/////////////////////////////////////////////////////////////////////////////////
	D3DCOLOR D3D9Mapping::MappingToUInt32Color(Color const & clr)
	{
		return D3DCOLOR_COLORVALUE(clr.r(), clr.g(), clr.b(), clr.a());
	}

	// 从KlayGE的Color转换到D3DCOLORVALUE
	/////////////////////////////////////////////////////////////////////////////////
	D3DCOLORVALUE D3D9Mapping::MappingToFloat4Color(Color const & clr)
	{
		return D3DXCOLOR(clr.r(), clr.g(), clr.b(), clr.a());
	}

	// 从KlayGE的ColorMask转换到D3DCOLORWRITEENABLE所使用的格式
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D9Mapping::MappingColorMask(uint32_t mask)
	{
		uint32_t ret = 0;
		if (mask & RenderEngine::CMASK_Red)
		{
			ret |= D3DCOLORWRITEENABLE_RED;
		}
		if (mask & RenderEngine::CMASK_Green)
		{
			ret |= D3DCOLORWRITEENABLE_GREEN;
		}
		if (mask & RenderEngine::CMASK_Blue)
		{
			ret |= D3DCOLORWRITEENABLE_BLUE;
		}
		if (mask & RenderEngine::CMASK_Alpha)
		{
			ret |= D3DCOLORWRITEENABLE_ALPHA;
		}
		return ret;
	}

	// 从RenderEngine::CompareFunction转换到D3DCMPFUNC
	/////////////////////////////////////////////////////////////////////////////////
	D3DCMPFUNC D3D9Mapping::Mapping(RenderEngine::CompareFunction func)
	{
		switch (func)
		{
		case RenderEngine::CF_AlwaysFail:
			return D3DCMP_NEVER;

		case RenderEngine::CF_AlwaysPass:
			return D3DCMP_ALWAYS;

		case RenderEngine::CF_Less:
			return D3DCMP_LESS;

		case RenderEngine::CF_LessEqual:
			return D3DCMP_LESSEQUAL;

		case RenderEngine::CF_Equal:
			return D3DCMP_EQUAL;

		case RenderEngine::CF_NotEqual:
			return D3DCMP_NOTEQUAL;

		case RenderEngine::CF_GreaterEqual:
			return D3DCMP_GREATEREQUAL;

		case RenderEngine::CF_Greater:
			return D3DCMP_GREATER;

		default:
			BOOST_ASSERT(false);
			return D3DCMP_NEVER;
		};
	}

	// 从RenderEngine::StencilOperation转换到D3DSTENCILOP
	/////////////////////////////////////////////////////////////////////////////////
	D3DSTENCILOP D3D9Mapping::Mapping(RenderEngine::StencilOperation op)
	{
		switch (op)
		{
		case RenderEngine::SOP_Keep:
			return D3DSTENCILOP_KEEP;

		case RenderEngine::SOP_Zero:
			return D3DSTENCILOP_ZERO;

		case RenderEngine::SOP_Replace:
			return D3DSTENCILOP_REPLACE;

		case RenderEngine::SOP_Increment:
			return D3DSTENCILOP_INCR;

		case RenderEngine::SOP_Decrement:
			return D3DSTENCILOP_DECR;

		case RenderEngine::SOP_Invert:
			return D3DSTENCILOP_INVERT;

		default:
			BOOST_ASSERT(false);
			return D3DSTENCILOP_KEEP;
		};
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case RenderEngine::ABF_Zero:
			return D3DBLEND_ZERO;

		case RenderEngine::ABF_One:
			return D3DBLEND_ONE;

		case RenderEngine::ABF_Src_Alpha:
			return D3DBLEND_SRCALPHA;

		case RenderEngine::ABF_Dst_Alpha:
			return D3DBLEND_DESTALPHA;

		case RenderEngine::ABF_Inv_Src_Alpha:
			return D3DBLEND_INVSRCALPHA;

		case RenderEngine::ABF_Inv_Dst_Alpha:
			return D3DBLEND_INVDESTALPHA;

		case RenderEngine::ABF_Src_Color:
			return D3DBLEND_SRCCOLOR;

		case RenderEngine::ABF_Dst_Color:
			return D3DBLEND_DESTCOLOR;

		case RenderEngine::ABF_Inv_Src_Color:
			return D3DBLEND_INVSRCCOLOR;

		case RenderEngine::ABF_Inv_Dst_Color:
			return D3DBLEND_INVDESTCOLOR;

		case RenderEngine::ABF_Src_Alpha_Sat:
			return D3DBLEND_SRCALPHASAT;

		default:
			BOOST_ASSERT(false);
			return D3DBLEND_ZERO;
		}
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::CullMode mode)
	{
		switch (mode)
		{
		case RenderEngine::CM_None:
			return D3DCULL_NONE;

		case RenderEngine::CM_Clockwise:
			return D3DCULL_CW;

		case RenderEngine::CM_AntiClockwise:
			return D3DCULL_CCW;

		default:
			BOOST_ASSERT(false);
			return D3DCULL_NONE;
		}
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::PolygonMode mode)
	{
		switch (mode)
		{
		case RenderEngine::PM_Point:
			return D3DFILL_POINT;

		case RenderEngine::PM_Line:
			return D3DFILL_WIREFRAME;

		case RenderEngine::PM_Fill:
			return D3DFILL_SOLID;

		default:
			BOOST_ASSERT(false);
			return D3DFILL_POINT;
		}
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::ShadeMode mode)
	{
		switch (mode)
		{
		case RenderEngine::SM_Flat:
			return D3DSHADE_FLAT;

		case RenderEngine::SM_Gouraud:
			return D3DSHADE_GOURAUD;

		default:
			BOOST_ASSERT(false);
			return D3DSHADE_FLAT;
		}
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::BlendOperation bo)
	{
		switch (bo)
		{
		case RenderEngine::BOP_Add:
			return D3DBLENDOP_ADD;

		case RenderEngine::BOP_Sub:
			return D3DBLENDOP_SUBTRACT;

		case RenderEngine::BOP_Rev_Sub:
			return D3DBLENDOP_REVSUBTRACT;

		case RenderEngine::BOP_Min:
			return D3DBLENDOP_MIN;

		case RenderEngine::BOP_Max:
			return D3DBLENDOP_MAX;

		default:
			BOOST_ASSERT(false);
			return D3DBLENDOP_ADD;
		}
	}

	uint32_t D3D9Mapping::Mapping(Sampler::TexAddressingMode mode)
	{
		switch (mode)
		{
		case Sampler::TAM_Clamp:
			return D3DTADDRESS_CLAMP;

		case Sampler::TAM_Wrap:
			return D3DTADDRESS_WRAP;

		case Sampler::TAM_Mirror:
			return D3DTADDRESS_MIRROR;

		case Sampler::TAM_Border:
			return D3DTADDRESS_BORDER;

		default:
			BOOST_ASSERT(false);
			return D3DTADDRESS_CLAMP;
		}
	}

	void D3D9Mapping::Mapping(D3DPRIMITIVETYPE& primType, uint32_t& primCount, RenderLayout const & rl)
	{
		uint32_t const vertexCount(static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices()));
		switch (rl.Type())
		{
		case RenderLayout::BT_PointList:
			primType = D3DPT_POINTLIST;
			primCount = vertexCount;
			break;

		case RenderLayout::BT_LineList:
			primType = D3DPT_LINELIST;
			primCount = vertexCount / 2;
			break;

		case RenderLayout::BT_LineStrip:
			primType = D3DPT_LINESTRIP;
			primCount = vertexCount - 1;
			break;

		case RenderLayout::BT_TriangleList:
			primType = D3DPT_TRIANGLELIST;
			primCount = vertexCount / 3;
			break;

		case RenderLayout::BT_TriangleStrip:
			primType = D3DPT_TRIANGLESTRIP;
			primCount = vertexCount - 2;
			break;

		case RenderLayout::BT_TriangleFan:
			primType = D3DPT_TRIANGLEFAN;
			primCount = vertexCount - 2;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void D3D9Mapping::Mapping(std::vector<D3DVERTEXELEMENT9>& elements, size_t stream, vertex_elements_type const & vet)
	{
		elements.resize(vet.size());

		uint16_t elem_offset = 0;
		for (uint32_t i = 0; i < elements.size(); ++ i)
		{
			vertex_element const & vs_elem = vet[i];

			D3DVERTEXELEMENT9& element = elements[i];
			element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<uint8_t>(NumComponents(vs_elem.format));
			element.Offset		= elem_offset;
			element.Method		= D3DDECLMETHOD_DEFAULT;
			element.Stream		= static_cast<WORD>(stream);
			element.UsageIndex	= vs_elem.usage_index;

			switch (vs_elem.usage)
			{
			// Vertex xyzs
			case VEU_Position:
				element.Usage = D3DDECLUSAGE_POSITION;
				break;

			// Normal
			case VEU_Normal:
				element.Usage = D3DDECLUSAGE_NORMAL;
				break;

			// Vertex colors
			case VEU_Diffuse:
				element.Usage = D3DDECLUSAGE_COLOR;
				break;

			// Vertex speculars
			case VEU_Specular:
				element.Usage = D3DDECLUSAGE_COLOR;
				break;
			
			// Blend Weights
			case VEU_BlendWeight:
				element.Usage = D3DDECLUSAGE_BLENDWEIGHT;
				break;

			// Blend Indices
			case VEU_BlendIndex:
				element.Type = D3DDECLTYPE_D3DCOLOR;
				element.Usage = D3DDECLUSAGE_BLENDINDICES;
				break;

			// Do texture coords
			case VEU_TextureCoord:
				element.Usage = D3DDECLUSAGE_TEXCOORD;
				break;

			case VEU_Tangent:
				element.Usage = D3DDECLUSAGE_TANGENT;
				break;

			case VEU_Binormal:
				element.Usage = D3DDECLUSAGE_BINORMAL;
				break;
			}

			elem_offset = static_cast<uint16_t>(elem_offset + vs_elem.element_size());
		}
	}

	RenderDeviceCaps D3D9Mapping::Mapping(D3DCAPS9 const & d3d_caps)
	{
		RenderDeviceCaps ret;

		ret.max_shader_model		= std::min((d3d_caps.VertexShaderVersion & 0xFF00) >> 8,
													(d3d_caps.PixelShaderVersion & 0xFF00) >> 8);

		ret.max_texture_width		= d3d_caps.MaxTextureWidth;
		ret.max_texture_height		= d3d_caps.MaxTextureHeight;
		ret.max_texture_depth		= d3d_caps.MaxVolumeExtent;
		ret.max_texture_cube_size	= d3d_caps.MaxTextureWidth;
		ret.max_textures_units		= d3d_caps.MaxSimultaneousTextures;
		ret.max_texture_anisotropy	= d3d_caps.MaxAnisotropy;
		ret.max_vertex_texture_units = 4;

		ret.max_user_clip_planes	= d3d_caps.MaxUserClipPlanes;

		ret.max_simultaneous_rts	= d3d_caps.NumSimultaneousRTs;

		ret.max_vertices = d3d_caps.MaxPrimitiveCount > 0xFFFF ? d3d_caps.MaxPrimitiveCount : d3d_caps.MaxPrimitiveCount * 3;
		ret.max_indices = d3d_caps.MaxVertexIndex;

		ret.texture_2d_filter_caps = 0;
		if (((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_2d_filter_caps |= Sampler::TFO_Point;
		}
		if (((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_2d_filter_caps |= Sampler::TFO_Bilinear;
		}
		if (((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) != 0))
		{
			ret.texture_2d_filter_caps |= Sampler::TFO_Trilinear;
		}
		if ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0)
		{
			ret.texture_2d_filter_caps |= Sampler::TFO_Anisotropic;
		}
		ret.texture_1d_filter_caps = ret.texture_2d_filter_caps;

		ret.texture_3d_filter_caps = 0;
		if (((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_3d_filter_caps |= Sampler::TFO_Point;
		}
		if (((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_3d_filter_caps |= Sampler::TFO_Bilinear;
		}
		if (((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) != 0))
		{
			ret.texture_3d_filter_caps |= Sampler::TFO_Trilinear;
		}
		if ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0)
		{
			ret.texture_3d_filter_caps |= Sampler::TFO_Anisotropic;
		}

		ret.texture_cube_filter_caps = 0;
		if (((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_cube_filter_caps |= Sampler::TFO_Point;
		}
		if (((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_cube_filter_caps |= Sampler::TFO_Bilinear;
		}
		if (((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) != 0))
		{
			ret.texture_cube_filter_caps |= Sampler::TFO_Trilinear;
		}
		if ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0)
		{
			ret.texture_cube_filter_caps |= Sampler::TFO_Anisotropic;
		}

		if (ret.max_shader_model >= 3)
		{
			ret.hw_instancing_support = true;
		}
		else
		{
			// Check for ATI instancing support
			D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			if (D3D_OK == renderEngine.D3DObject()->CheckDeviceFormat(D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
				static_cast<D3DFORMAT>(MAKEFOURCC('I', 'N', 'S', 'T'))))
			{
				ret.hw_instancing_support = true;
			}
			else
			{
				ret.hw_instancing_support = false;
			}
		}

		return ret;
	}

	D3DFORMAT D3D9Mapping::MappingFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_L8:
			return D3DFMT_L8;

		case EF_A8:
			return D3DFMT_A8;

		case EF_AL4:
			return D3DFMT_A4L4;

		case EF_L16:
			return D3DFMT_L16;

		case EF_AL8:
			return D3DFMT_A8L8;

		case EF_R5G6B5:
			return D3DFMT_R5G6B5;

		case EF_ARGB4:
			return D3DFMT_A4R4G4B4;

		case EF_XRGB8:
			return D3DFMT_X8R8G8B8;

		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return D3DFMT_A8R8G8B8;

		case EF_A2RGB10:
			return D3DFMT_A2B10G10R10;

		case EF_GR16:
			return D3DFMT_G16R16;

		case EF_ABGR16:
			return D3DFMT_A16B16G16R16;

		case EF_R16F:
			return D3DFMT_R16F;

		case EF_GR16F:
			return D3DFMT_G16R16F;

		case EF_ABGR16F:
			return D3DFMT_A16B16G16R16F;

		case EF_R32F:
			return D3DFMT_R32F;

		case EF_GR32F:
			return D3DFMT_G32R32F;

		case EF_ABGR32F:
			return D3DFMT_A32B32G32R32F;

		case EF_DXT1:
		case EF_DXT1_SRGB:
			return D3DFMT_DXT1;

		case EF_DXT3:
		case EF_DXT3_SRGB:
			return D3DFMT_DXT3;

		case EF_DXT5:
		case EF_DXT5_SRGB:
			return D3DFMT_DXT5;

		case EF_D16:
			return D3DFMT_D16;

		case EF_D24X8:
			return D3DFMT_D24X8;

		case EF_D24S8:
			return D3DFMT_D24S8;

		case EF_D32:
			return D3DFMT_D32;

		default:
			BOOST_ASSERT(false);
			return D3DFMT_UNKNOWN;
		}
	}

	ElementFormat D3D9Mapping::MappingFormat(D3DFORMAT format)
	{
		switch (format)
		{
		case D3DFMT_L8:
			return EF_L8;

		case D3DFMT_A8:
			return EF_A8;

		case D3DFMT_A4L4:
			return EF_AL4;

		case D3DFMT_L16:
			return EF_L16;

		case D3DFMT_A8L8:
			return EF_AL8;

		case D3DFMT_R5G6B5:
			return EF_R5G6B5;

		case D3DFMT_A4R4G4B4:
			return EF_ARGB4;

		case D3DFMT_X8R8G8B8:
			return EF_XRGB8;

		case D3DFMT_A8R8G8B8:
			return EF_ARGB8;

		case D3DFMT_A2B10G10R10:
			return EF_A2RGB10;

		case D3DFMT_G16R16:
			return EF_GR16;

		case D3DFMT_A16B16G16R16:
			return EF_ABGR16;

		case D3DFMT_R16F:
			return EF_R16F;

		case D3DFMT_G16R16F:
			return EF_GR16F;

		case D3DFMT_A16B16G16R16F:
			return EF_ABGR16F;

		case D3DFMT_R32F:
			return EF_R32F;

		case D3DFMT_G32R32F:
			return EF_GR32F;

		case D3DFMT_A32B32G32R32F:
			return EF_ABGR32F;

		case D3DFMT_DXT1:
			return EF_DXT1;

		case D3DFMT_DXT3:
			return EF_DXT3;

		case D3DFMT_DXT5:
			return EF_DXT5;

		case D3DFMT_D16:
			return EF_D16;

		case D3DFMT_D24X8:
			return EF_D24X8;

		case D3DFMT_D24S8:
			return EF_D24S8;

		case D3DFMT_D32:
			return EF_D32;

		default:
			BOOST_ASSERT(false);
			return EF_Unknown;
		}
	}
}
