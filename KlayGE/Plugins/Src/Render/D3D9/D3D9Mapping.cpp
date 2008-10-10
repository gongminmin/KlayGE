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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/GraphicsBuffer.hpp>

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
		return *reinterpret_cast<D3DVECTOR const *>(&vec);
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
		return *reinterpret_cast<D3DCOLORVALUE const *>(&clr);
	}

	// 从KlayGE的ColorMask转换到D3DCOLORWRITEENABLE所使用的格式
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D9Mapping::MappingColorMask(uint32_t mask)
	{
		uint32_t ret = 0;
		if (mask & CMASK_Red)
		{
			ret |= D3DCOLORWRITEENABLE_RED;
		}
		if (mask & CMASK_Green)
		{
			ret |= D3DCOLORWRITEENABLE_GREEN;
		}
		if (mask & CMASK_Blue)
		{
			ret |= D3DCOLORWRITEENABLE_BLUE;
		}
		if (mask & CMASK_Alpha)
		{
			ret |= D3DCOLORWRITEENABLE_ALPHA;
		}
		return ret;
	}

	// 从RenderEngine::CompareFunction转换到D3DCMPFUNC
	/////////////////////////////////////////////////////////////////////////////////
	D3DCMPFUNC D3D9Mapping::Mapping(CompareFunction func)
	{
		switch (func)
		{
		case CF_AlwaysFail:
			return D3DCMP_NEVER;

		case CF_AlwaysPass:
			return D3DCMP_ALWAYS;

		case CF_Less:
			return D3DCMP_LESS;

		case CF_LessEqual:
			return D3DCMP_LESSEQUAL;

		case CF_Equal:
			return D3DCMP_EQUAL;

		case CF_NotEqual:
			return D3DCMP_NOTEQUAL;

		case CF_GreaterEqual:
			return D3DCMP_GREATEREQUAL;

		case CF_Greater:
			return D3DCMP_GREATER;

		default:
			BOOST_ASSERT(false);
			return D3DCMP_NEVER;
		};
	}

	// 从RenderEngine::StencilOperation转换到D3DSTENCILOP
	/////////////////////////////////////////////////////////////////////////////////
	D3DSTENCILOP D3D9Mapping::Mapping(StencilOperation op)
	{
		switch (op)
		{
		case SOP_Keep:
			return D3DSTENCILOP_KEEP;

		case SOP_Zero:
			return D3DSTENCILOP_ZERO;

		case SOP_Replace:
			return D3DSTENCILOP_REPLACE;

		case SOP_Increment:
			return D3DSTENCILOP_INCR;

		case SOP_Decrement:
			return D3DSTENCILOP_DECR;

		case SOP_Invert:
			return D3DSTENCILOP_INVERT;

		default:
			BOOST_ASSERT(false);
			return D3DSTENCILOP_KEEP;
		};
	}

	uint32_t D3D9Mapping::Mapping(AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case ABF_Zero:
			return D3DBLEND_ZERO;

		case ABF_One:
			return D3DBLEND_ONE;

		case ABF_Src_Alpha:
			return D3DBLEND_SRCALPHA;

		case ABF_Dst_Alpha:
			return D3DBLEND_DESTALPHA;

		case ABF_Inv_Src_Alpha:
			return D3DBLEND_INVSRCALPHA;

		case ABF_Inv_Dst_Alpha:
			return D3DBLEND_INVDESTALPHA;

		case ABF_Src_Color:
			return D3DBLEND_SRCCOLOR;

		case ABF_Dst_Color:
			return D3DBLEND_DESTCOLOR;

		case ABF_Inv_Src_Color:
			return D3DBLEND_INVSRCCOLOR;

		case ABF_Inv_Dst_Color:
			return D3DBLEND_INVDESTCOLOR;

		case ABF_Src_Alpha_Sat:
			return D3DBLEND_SRCALPHASAT;

		default:
			BOOST_ASSERT(false);
			return D3DBLEND_ZERO;
		}
	}

	uint32_t D3D9Mapping::Mapping(CullMode mode, bool front_face_ccw)
	{
		switch (mode)
		{
		case CM_None:
			return D3DCULL_NONE;

		case CM_Front:
			if (front_face_ccw)
			{
				return D3DCULL_CCW;
			}
			else
			{
				return D3DCULL_CW;
			}

		case CM_Back:
			if (front_face_ccw)
			{
				return D3DCULL_CW;
			}
			else
			{
				return D3DCULL_CCW;
			}

		default:
			BOOST_ASSERT(false);
			return D3DCULL_NONE;
		}
	}

	uint32_t D3D9Mapping::Mapping(PolygonMode mode)
	{
		switch (mode)
		{
		case PM_Point:
			return D3DFILL_POINT;

		case PM_Line:
			return D3DFILL_WIREFRAME;

		case PM_Fill:
			return D3DFILL_SOLID;

		default:
			BOOST_ASSERT(false);
			return D3DFILL_POINT;
		}
	}

	uint32_t D3D9Mapping::Mapping(ShadeMode mode)
	{
		switch (mode)
		{
		case SM_Flat:
			return D3DSHADE_FLAT;

		case SM_Gouraud:
			return D3DSHADE_GOURAUD;

		default:
			BOOST_ASSERT(false);
			return D3DSHADE_FLAT;
		}
	}

	uint32_t D3D9Mapping::Mapping(BlendOperation bo)
	{
		switch (bo)
		{
		case BOP_Add:
			return D3DBLENDOP_ADD;

		case BOP_Sub:
			return D3DBLENDOP_SUBTRACT;

		case BOP_Rev_Sub:
			return D3DBLENDOP_REVSUBTRACT;

		case BOP_Min:
			return D3DBLENDOP_MIN;

		case BOP_Max:
			return D3DBLENDOP_MAX;

		default:
			BOOST_ASSERT(false);
			return D3DBLENDOP_ADD;
		}
	}

	uint32_t D3D9Mapping::Mapping(TexAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_Clamp:
			return D3DTADDRESS_CLAMP;

		case TAM_Wrap:
			return D3DTADDRESS_WRAP;

		case TAM_Mirror:
			return D3DTADDRESS_MIRROR;

		case TAM_Border:
			return D3DTADDRESS_BORDER;

		default:
			BOOST_ASSERT(false);
			return D3DTADDRESS_CLAMP;
		}
	}

	void D3D9Mapping::Mapping(D3DPRIMITIVETYPE& primType, uint32_t& primCount, RenderLayout const & rl)
	{
		uint32_t const vertexCount(static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices()));
		switch (rl.TopologyType())
		{
		case RenderLayout::TT_PointList:
			primType = D3DPT_POINTLIST;
			primCount = vertexCount;
			break;

		case RenderLayout::TT_LineList:
			primType = D3DPT_LINELIST;
			primCount = vertexCount / 2;
			break;

		case RenderLayout::TT_LineStrip:
			primType = D3DPT_LINESTRIP;
			primCount = vertexCount - 1;
			break;

		case RenderLayout::TT_TriangleList:
			primType = D3DPT_TRIANGLELIST;
			primCount = vertexCount / 3;
			break;

		case RenderLayout::TT_TriangleStrip:
			primType = D3DPT_TRIANGLESTRIP;
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
			if (IsFloatFormat(vs_elem.format))
			{
				element.Type = D3DDECLTYPE_FLOAT1 - 1 + static_cast<uint8_t>(NumComponents(vs_elem.format));
			}
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
				if (EF_ABGR8 == vs_elem.format)
				{
					element.Type = D3DDECLTYPE_UBYTE4N;
				}
				if (EF_ARGB8 == vs_elem.format)
				{
					element.Type = D3DDECLTYPE_D3DCOLOR;
				}
				break;

			// Vertex speculars
			case VEU_Specular:
				element.Usage = D3DDECLUSAGE_COLOR;
				if (EF_ABGR8 == vs_elem.format)
				{
					element.Type = D3DDECLTYPE_UBYTE4N;
				}
				if (EF_ARGB8 == vs_elem.format)
				{
					element.Type = D3DDECLTYPE_D3DCOLOR;
				}
				break;

			// Blend Weights
			case VEU_BlendWeight:
				element.Usage = D3DDECLUSAGE_BLENDWEIGHT;
				break;

			// Blend Indices
			case VEU_BlendIndex:
				element.Usage = D3DDECLUSAGE_BLENDINDICES;
				element.Type = D3DDECLTYPE_UBYTE4;
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
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		RenderDeviceCaps ret;

		ret.max_shader_model		= static_cast<uint8_t>(std::min((d3d_caps.VertexShaderVersion & 0xFF00) >> 8,
													(d3d_caps.PixelShaderVersion & 0xFF00) >> 8));

		ret.max_texture_width		= d3d_caps.MaxTextureWidth;
		ret.max_texture_height		= d3d_caps.MaxTextureHeight;
		ret.max_texture_depth		= d3d_caps.MaxVolumeExtent;
		ret.max_texture_cube_size	= d3d_caps.MaxTextureWidth;
		ret.max_texture_array_length = 0;
		ret.max_texture_units		= 16;
		ret.max_texture_anisotropy	= static_cast<uint8_t>(d3d_caps.MaxAnisotropy);

		if (S_OK == re.D3DObject()->CheckDeviceFormat(d3d_caps.AdapterOrdinal,
				d3d_caps.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE,
				D3DFMT_A32B32G32R32F))
		{
			ret.max_vertex_texture_units = 4;
		}
		else
		{
			ret.max_vertex_texture_units = 0;
		}

		ret.max_simultaneous_rts	= static_cast<uint8_t>(d3d_caps.NumSimultaneousRTs);

		ret.max_vertices = d3d_caps.MaxPrimitiveCount > 0xFFFF ? d3d_caps.MaxPrimitiveCount : d3d_caps.MaxPrimitiveCount * 3;
		ret.max_indices = d3d_caps.MaxVertexIndex;

		ret.texture_2d_filter_caps = 0;
		if (((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_2d_filter_caps |= TFO_Point;
		}
		if (((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_2d_filter_caps |= TFO_Bilinear;
		}
		if (((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) != 0))
		{
			ret.texture_2d_filter_caps |= TFO_Trilinear;
		}
		if ((d3d_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0)
		{
			ret.texture_2d_filter_caps |= TFO_Anisotropic;
		}
		ret.texture_1d_filter_caps = ret.texture_2d_filter_caps;

		ret.texture_3d_filter_caps = 0;
		if (((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_3d_filter_caps |= TFO_Point;
		}
		if (((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_3d_filter_caps |= TFO_Bilinear;
		}
		if (((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) != 0))
		{
			ret.texture_3d_filter_caps |= TFO_Trilinear;
		}
		if ((d3d_caps.VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0)
		{
			ret.texture_3d_filter_caps |= TFO_Anisotropic;
		}

		ret.texture_cube_filter_caps = 0;
		if (((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_cube_filter_caps |= TFO_Point;
		}
		if (((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) != 0))
		{
			ret.texture_cube_filter_caps |= TFO_Bilinear;
		}
		if (((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) != 0)
			&& ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) != 0))
		{
			ret.texture_cube_filter_caps |= TFO_Trilinear;
		}
		if ((d3d_caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0)
		{
			ret.texture_cube_filter_caps |= TFO_Anisotropic;
		}

		if (ret.max_shader_model >= 3)
		{
			ret.hw_instancing_support = true;
		}
		else
		{
			// Check for ATI instancing support
			if (D3D_OK == re.D3DObject()->CheckDeviceFormat(d3d_caps.AdapterOrdinal,
				d3d_caps.DeviceType, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
				static_cast<D3DFORMAT>(MakeFourCC<'I', 'N', 'S', 'T'>::value)))
			{
				ret.hw_instancing_support = true;
			}
			else
			{
				ret.hw_instancing_support = false;
			}
		}

		ret.stream_output_support = false;

		// NVIDIA's Transparency Multisampling
		if (S_OK == re.D3DObject()->CheckDeviceFormat(d3d_caps.AdapterOrdinal,
			d3d_caps.DeviceType, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
			static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value)))
		{
			ret.alpha_to_coverage_support = true;
		}
		else
		{
			ret.alpha_to_coverage_support = false;
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

		case EF_SIGNED_GR8:
			return D3DFMT_V8U8;

		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return D3DFMT_A8R8G8B8;

		case EF_SIGNED_ABGR8:
			return D3DFMT_Q8W8V8U8;

		case EF_A2BGR10:
			return D3DFMT_A2B10G10R10;

		case EF_SIGNED_A2BGR10:
			return D3DFMT_A2W10V10U10;

		case EF_GR16:
			return D3DFMT_G16R16;

		case EF_SIGNED_GR16:
			return D3DFMT_V16U16;

		case EF_ABGR16:
			return D3DFMT_A16B16G16R16;

		case EF_SIGNED_ABGR16:
			return D3DFMT_Q16W16V16U16;

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

		case EF_BC1:
		case EF_BC1_SRGB:
			return D3DFMT_DXT1;

		case EF_BC2:
		case EF_BC2_SRGB:
			return D3DFMT_DXT3;

		case EF_BC3:
		case EF_BC3_SRGB:
			return D3DFMT_DXT5;

		case EF_BC4:
		case EF_BC4_SRGB:
			return static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'I', '1'>::value);

		case EF_BC5:
		case EF_BC5_SRGB:
			return static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'I', '2'>::value);

		case EF_D16:
			return D3DFMT_D16;

		case EF_D24S8:
			return D3DFMT_D24S8;

		default:
			THR(boost::system::posix_error::not_supported);
		}
	}

	ElementFormat D3D9Mapping::MappingFormat(D3DFORMAT format)
	{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4063)
#endif
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

		case D3DFMT_X4R4G4B4:
		case D3DFMT_A4R4G4B4:
			return EF_ARGB4;

		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
			return EF_ARGB8;

		case D3DFMT_V8U8:
			return EF_SIGNED_GR8;

		case D3DFMT_Q8W8V8U8:
			return EF_SIGNED_ABGR8;

		case D3DFMT_A2B10G10R10:
			return EF_A2BGR10;

		case D3DFMT_A2W10V10U10:
			return EF_SIGNED_A2BGR10;

		case D3DFMT_G16R16:
			return EF_GR16;

		case D3DFMT_V16U16:
			return EF_SIGNED_GR16;

		case D3DFMT_A16B16G16R16:
			return EF_ABGR16;

		case D3DFMT_Q16W16V16U16:
			return EF_SIGNED_ABGR16;

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
			return EF_BC1;

		case D3DFMT_DXT3:
			return EF_BC2;

		case D3DFMT_DXT5:
			return EF_BC3;

		case MakeFourCC<'A', 'T', 'I', '1'>::value:
			return EF_BC4;

		case MakeFourCC<'A', 'T', 'I', '2'>::value:
			return EF_BC5;

		case D3DFMT_D16:
			return EF_D16;

		case D3DFMT_D24X8:
		case D3DFMT_D24S8:
			return EF_D24S8;

		default:
			THR(boost::system::posix_error::not_supported);
		}
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}
}
