// D3D11Mapping.hpp
// KlayGE RenderEngine和D3D11本地之间的映射 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/GraphicsBuffer.hpp>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>
#include <d3dx11.h>

#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>

namespace KlayGE
{
	// 从KlayGE的ColorMask转换到D3DCOLORWRITEENABLE所使用的格式
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D11Mapping::MappingColorMask(uint32_t mask)
	{
		uint32_t ret = 0;
		if (mask & CMASK_Red)
		{
			ret |= D3D11_COLOR_WRITE_ENABLE_RED;
		}
		if (mask & CMASK_Green)
		{
			ret |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		}
		if (mask & CMASK_Blue)
		{
			ret |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		}
		if (mask & CMASK_Alpha)
		{
			ret |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
		}
		return ret;
	}

	// 从RenderEngine::CompareFunction转换到D3D11_COMPARISON_FUNC
	/////////////////////////////////////////////////////////////////////////////////
	D3D11_COMPARISON_FUNC D3D11Mapping::Mapping(CompareFunction func)
	{
		switch (func)
		{
		case CF_AlwaysFail:
			return D3D11_COMPARISON_NEVER;

		case CF_AlwaysPass:
			return D3D11_COMPARISON_ALWAYS;

		case CF_Less:
			return D3D11_COMPARISON_LESS;

		case CF_LessEqual:
			return D3D11_COMPARISON_LESS_EQUAL;

		case CF_Equal:
			return D3D11_COMPARISON_EQUAL;

		case CF_NotEqual:
			return D3D11_COMPARISON_NOT_EQUAL;

		case CF_GreaterEqual:
			return D3D11_COMPARISON_GREATER_EQUAL;

		case CF_Greater:
			return D3D11_COMPARISON_GREATER;

		default:
			BOOST_ASSERT(false);
			return D3D11_COMPARISON_NEVER;
		};
	}

	// 从RenderEngine::StencilOperation转换到D3D11_STENCIL_OP
	/////////////////////////////////////////////////////////////////////////////////
	D3D11_STENCIL_OP D3D11Mapping::Mapping(StencilOperation op)
	{
		switch (op)
		{
		case SOP_Keep:
			return D3D11_STENCIL_OP_KEEP;

		case SOP_Zero:
			return D3D11_STENCIL_OP_ZERO;

		case SOP_Replace:
			return D3D11_STENCIL_OP_REPLACE;

		case SOP_Increment:
			return D3D11_STENCIL_OP_INCR;

		case SOP_Decrement:
			return D3D11_STENCIL_OP_DECR;

		case SOP_Invert:
			return D3D11_STENCIL_OP_INVERT;

		default:
			BOOST_ASSERT(false);
			return D3D11_STENCIL_OP_KEEP;
		};
	}

	D3D11_MAP D3D11Mapping::Mapping(TextureMapAccess tma, Texture::TextureType type, uint32_t access_hint, uint32_t numMipMaps)
	{
		switch (tma)
		{
		case TMA_Read_Only:
			return D3D11_MAP_READ;

		case TMA_Write_Only:
			if (((EAH_CPU_Write | EAH_GPU_Read) == access_hint)
				|| ((EAH_CPU_Write == access_hint) && (1 == numMipMaps) && (type != Texture::TT_Cube)))
			{
				return D3D11_MAP_WRITE_DISCARD;
			}
			else
			{
				return D3D11_MAP_WRITE;
			}

		case TMA_Read_Write:
			return D3D11_MAP_READ_WRITE;

		default:
			BOOST_ASSERT(false);
			return D3D11_MAP_READ;
		};
	}

	D3D11_BLEND D3D11Mapping::Mapping(AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case ABF_Zero:
			return D3D11_BLEND_ZERO;

		case ABF_One:
			return D3D11_BLEND_ONE;

		case ABF_Src_Alpha:
			return D3D11_BLEND_SRC_ALPHA;

		case ABF_Dst_Alpha:
			return D3D11_BLEND_DEST_ALPHA;

		case ABF_Inv_Src_Alpha:
			return D3D11_BLEND_INV_SRC_ALPHA;

		case ABF_Inv_Dst_Alpha:
			return D3D11_BLEND_INV_DEST_ALPHA;

		case ABF_Src_Color:
			return D3D11_BLEND_SRC_COLOR;

		case ABF_Dst_Color:
			return D3D11_BLEND_DEST_COLOR;

		case ABF_Inv_Src_Color:
			return D3D11_BLEND_INV_SRC_COLOR;

		case ABF_Inv_Dst_Color:
			return D3D11_BLEND_INV_DEST_COLOR;

		case ABF_Src_Alpha_Sat:
			return D3D11_BLEND_SRC_ALPHA_SAT;

		default:
			BOOST_ASSERT(false);
			return D3D11_BLEND_ZERO;
		}
	}

	D3D11_CULL_MODE D3D11Mapping::Mapping(CullMode mode)
	{
		switch (mode)
		{
		case CM_None:
			return D3D11_CULL_NONE;

		case CM_Front:
			return D3D11_CULL_FRONT;

		case CM_Back:
			return D3D11_CULL_BACK;

		default:
			BOOST_ASSERT(false);
			return D3D11_CULL_NONE;
		}
	}

	D3D11_FILL_MODE D3D11Mapping::Mapping(PolygonMode mode)
	{
		switch (mode)
		{
		case PM_Point:
			return D3D11_FILL_WIREFRAME;

		case PM_Line:
			return D3D11_FILL_WIREFRAME;

		case PM_Fill:
			return D3D11_FILL_SOLID;

		default:
			BOOST_ASSERT(false);
			return D3D11_FILL_SOLID;
		}
	}

	D3D11_BLEND_OP D3D11Mapping::Mapping(BlendOperation bo)
	{
		switch (bo)
		{
		case BOP_Add:
			return D3D11_BLEND_OP_ADD;

		case BOP_Sub:
			return D3D11_BLEND_OP_SUBTRACT;

		case BOP_Rev_Sub:
			return D3D11_BLEND_OP_REV_SUBTRACT;

		case BOP_Min:
			return D3D11_BLEND_OP_MIN;

		case BOP_Max:
			return D3D11_BLEND_OP_MAX;

		default:
			BOOST_ASSERT(false);
			return D3D11_BLEND_OP_ADD;
		}
	}

	D3D11_TEXTURE_ADDRESS_MODE D3D11Mapping::Mapping(TexAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;

		case TAM_Wrap:
			return D3D11_TEXTURE_ADDRESS_WRAP;

		case TAM_Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;

		case TAM_Border:
			return D3D11_TEXTURE_ADDRESS_BORDER;

		default:
			BOOST_ASSERT(false);
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		}
	}

	D3D11_FILTER D3D11Mapping::Mapping(TexFilterOp filter)
	{
		switch (filter)
		{
		case TFO_Min_Mag_Mip_Point:
			return D3D11_FILTER_MIN_MAG_MIP_POINT;

		case TFO_Min_Mag_Point_Mip_Linear:
			return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;

		case TFO_Min_Point_Mag_Linear_Mip_Point:
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

		case TFO_Min_Point_Mag_Mip_Linear:
			return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;

		case TFO_Min_Mag_Linear_Mip_Point:
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		case TFO_Min_Mag_Mip_Linear:
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

		case TFO_Anisotropic:
			return D3D11_FILTER_ANISOTROPIC;

		case TFO_Cmp_Min_Mag_Mip_Point:
			return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

		case TFO_Cmp_Min_Mag_Point_Mip_Linear:
			return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;

		case TFO_Cmp_Min_Point_Mag_Linear_Mip_Point:
			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;

		case TFO_Cmp_Min_Point_Mag_Mip_Linear:
			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;

		case TFO_Cmp_Min_Mag_Linear_Mip_Point:
			return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

		case TFO_Cmp_Min_Mag_Mip_Linear:
			return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

		case TFO_Cmp_Anisotropic:
			return D3D11_FILTER_COMPARISON_ANISOTROPIC;

		default:
			BOOST_ASSERT(false);
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
	}

	D3D11_DEPTH_WRITE_MASK D3D11Mapping::Mapping(bool depth_write_mask)
	{
		if (depth_write_mask)
		{
			return D3D11_DEPTH_WRITE_MASK_ALL;
		}
		else
		{
			return D3D11_DEPTH_WRITE_MASK_ZERO;
		}
	}

	D3D11_PRIMITIVE_TOPOLOGY D3D11Mapping::Mapping(RenderLayout::topology_type tt)
	{
		switch (tt)
		{
		case RenderLayout::TT_PointList:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

		case RenderLayout::TT_LineList:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		case RenderLayout::TT_LineStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

		case RenderLayout::TT_TriangleList:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		case RenderLayout::TT_TriangleStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		default:
			BOOST_ASSERT(false);
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		}
	}

	void D3D11Mapping::Mapping(std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, size_t stream, vertex_elements_type const & vet, RenderLayout::stream_type type, uint32_t freq)
	{
		elements.resize(vet.size());

		uint16_t elem_offset = 0;
		for (uint32_t i = 0; i < elements.size(); ++ i)
		{
			vertex_element const & vs_elem = vet[i];

			D3D11_INPUT_ELEMENT_DESC& element = elements[i];
			element.SemanticIndex = vs_elem.usage_index;
			element.Format = D3D11Mapping::MappingFormat(vs_elem.format);
			element.InputSlot = static_cast<WORD>(stream);
			element.AlignedByteOffset = elem_offset;
			if (RenderLayout::ST_Geometry == type)
			{
				element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				element.InstanceDataStepRate = 0;
			}
			else
			{
				BOOST_ASSERT(RenderLayout::ST_Instance == type);

				element.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				element.InstanceDataStepRate = freq;
			}

			switch (vs_elem.usage)
			{
			// Vertex xyzs
			case VEU_Position:
				element.SemanticName = "POSITION";
				break;

			// Normal
			case VEU_Normal:
				element.SemanticName = "NORMAL";
				break;

			// Vertex colors
			case VEU_Diffuse:
				element.SemanticName = "COLOR";
				break;

			// Vertex speculars
			case VEU_Specular:
				element.SemanticName = "COLOR";
				break;

			// Blend Weights
			case VEU_BlendWeight:
				element.SemanticName = "BLENDWEIGHT";
				break;

			// Blend Indices
			case VEU_BlendIndex:
				element.SemanticName = "BLENDINDICES";
				if (EF_ABGR8 == vs_elem.format)
				{
					element.Format = DXGI_FORMAT_R8G8B8A8_UINT;
				}
				break;

			// Do texture coords
			case VEU_TextureCoord:
				element.SemanticName = "TEXCOORD";
				break;

			case VEU_Tangent:
				element.SemanticName = "TANGENT";
				break;

			case VEU_Binormal:
				element.SemanticName = "BINORMAL";
				break;
			}

			elem_offset = static_cast<uint16_t>(elem_offset + vs_elem.element_size());
		}
	}

	DXGI_FORMAT D3D11Mapping::MappingFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_A8:
			return DXGI_FORMAT_A8_UNORM;

		case EF_R8:
			return DXGI_FORMAT_R8_UNORM;

		case EF_SIGNED_R8:
			return DXGI_FORMAT_R8_SNORM;

		case EF_GR8:
			return DXGI_FORMAT_R8G8_UNORM;

		case EF_SIGNED_GR8:
			return DXGI_FORMAT_R8G8_SNORM;

		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case EF_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case EF_SIGNED_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

		case EF_A2BGR10:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case EF_R8UI:
			return DXGI_FORMAT_R8_UINT;

		case EF_R8I:
			return DXGI_FORMAT_R8_SINT;

		case EF_GR8UI:
			return DXGI_FORMAT_R8G8_UINT;

		case EF_GR8I:
			return DXGI_FORMAT_R8G8_SINT;

		case EF_ABGR8UI:
			return DXGI_FORMAT_R8G8B8A8_UINT;

		case EF_ABGR8I:
			return DXGI_FORMAT_R8G8B8A8_SINT;

		case EF_A2BGR10UI:
			return DXGI_FORMAT_R10G10B10A2_UINT;

		case EF_R16:
			return DXGI_FORMAT_R16_UNORM;

		case EF_SIGNED_R16:
			return DXGI_FORMAT_R16_SNORM;

		case EF_GR16:
			return DXGI_FORMAT_R16G16_UNORM;

		case EF_SIGNED_GR16:
			return DXGI_FORMAT_R16G16_SNORM;

		case EF_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case EF_SIGNED_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case EF_R16UI:
			return DXGI_FORMAT_R16_UINT;

		case EF_R16I:
			return DXGI_FORMAT_R16_SINT;

		case EF_GR16UI:
			return DXGI_FORMAT_R16G16_UINT;

		case EF_GR16I:
			return DXGI_FORMAT_R16G16_SINT;

		case EF_ABGR16UI:
			return DXGI_FORMAT_R16G16B16A16_UINT;

		case EF_ABGR16I:
			return DXGI_FORMAT_R16G16B16A16_SINT;

		case EF_R32UI:
			return DXGI_FORMAT_R32_UINT;

		case EF_R32I:
			return DXGI_FORMAT_R32_SINT;

		case EF_GR32UI:
			return DXGI_FORMAT_R32G32_UINT;

		case EF_GR32I:
			return DXGI_FORMAT_R32G32_SINT;

		case EF_BGR32UI:
			return DXGI_FORMAT_R32G32B32_UINT;

		case EF_BGR32I:
			return DXGI_FORMAT_R32G32B32_SINT;

		case EF_ABGR32UI:
			return DXGI_FORMAT_R32G32B32A32_UINT;

		case EF_ABGR32I:
			return DXGI_FORMAT_R32G32B32A32_SINT;

		case EF_R16F:
			return DXGI_FORMAT_R16_FLOAT;

		case EF_GR16F:
			return DXGI_FORMAT_R16G16_FLOAT;

		case EF_B10G11R11F:
			return DXGI_FORMAT_R11G11B10_FLOAT;

		case EF_ABGR16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case EF_R32F:
			return DXGI_FORMAT_R32_FLOAT;

		case EF_GR32F:
			return DXGI_FORMAT_R32G32_FLOAT;

		case EF_BGR32F:
			return DXGI_FORMAT_R32G32B32_FLOAT;

		case EF_ABGR32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case EF_BC1:
			return DXGI_FORMAT_BC1_UNORM;

		case EF_BC2:
			return DXGI_FORMAT_BC2_UNORM;

		case EF_BC3:
			return DXGI_FORMAT_BC3_UNORM;

		case EF_BC4:
			return DXGI_FORMAT_BC4_UNORM;

		case EF_SIGNED_BC4:
			return DXGI_FORMAT_BC4_SNORM;

		case EF_BC5:
			return DXGI_FORMAT_BC5_UNORM;

		case EF_SIGNED_BC5:
			return DXGI_FORMAT_BC5_SNORM;

		case EF_BC6:
			return DXGI_FORMAT_BC6H_UF16;

		case EF_SIGNED_BC6:
			return DXGI_FORMAT_BC6H_SF16;

		case EF_BC7:
			return DXGI_FORMAT_BC7_UNORM;

		case EF_D16:
			return DXGI_FORMAT_D16_UNORM;

		case EF_D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case EF_D32F:
			return DXGI_FORMAT_D32_FLOAT;

		case EF_ABGR8_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case EF_BC1_SRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case EF_BC2_SRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case EF_BC3_SRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case EF_BC7_SRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

		default:
			THR(boost::system::posix_error::not_supported);
		}
	}

	ElementFormat D3D11Mapping::MappingFormat(DXGI_FORMAT format)
	{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4063)
#endif
		switch (format)
		{
		case DXGI_FORMAT_A8_UNORM:
			return EF_A8;

		case DXGI_FORMAT_R8_UNORM:
			return EF_R8;

		case DXGI_FORMAT_R8_SNORM:
			return EF_SIGNED_R8;

		case DXGI_FORMAT_R8G8_UNORM:
			return EF_GR8;

		case DXGI_FORMAT_R8G8_SNORM:
			return EF_SIGNED_GR8;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return EF_ARGB8;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return EF_ABGR8;

		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return EF_SIGNED_ABGR8;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return EF_A2BGR10;

		case DXGI_FORMAT_R8_UINT:
			return EF_R8UI;

		case DXGI_FORMAT_R8_SINT:
			return EF_R8I;

		case DXGI_FORMAT_R8G8_UINT:
			return EF_GR8UI;

		case DXGI_FORMAT_R8G8_SINT:
			return EF_GR8I;

		case DXGI_FORMAT_R8G8B8A8_UINT:
			return EF_ABGR8UI;

		case DXGI_FORMAT_R8G8B8A8_SINT:
			return EF_ABGR8I;

		case DXGI_FORMAT_R10G10B10A2_UINT:
			return EF_A2BGR10UI;

		case DXGI_FORMAT_R16_UNORM:
			return EF_R16;

		case DXGI_FORMAT_R16_SNORM:
			return EF_SIGNED_R16;

		case DXGI_FORMAT_R16G16_UNORM:
			return EF_GR16;

		case DXGI_FORMAT_R16G16_SNORM:
			return EF_SIGNED_GR16;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return EF_ABGR16;

		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return EF_SIGNED_ABGR16;

		case DXGI_FORMAT_R16_UINT:
			return EF_R16UI;

		case DXGI_FORMAT_R16_SINT:
			return EF_R16I;

		case DXGI_FORMAT_R16G16_UINT:
			return EF_GR16UI;

		case DXGI_FORMAT_R16G16_SINT:
			return EF_GR16I;

		case DXGI_FORMAT_R16G16B16A16_UINT:
			return EF_ABGR16UI;

		case DXGI_FORMAT_R16G16B16A16_SINT:
			return EF_ABGR16I;

		case DXGI_FORMAT_R32_UINT:
			return EF_R32UI;

		case DXGI_FORMAT_R32_SINT:
			return EF_R32I;

		case DXGI_FORMAT_R32G32_UINT:
			return EF_GR32UI;

		case DXGI_FORMAT_R32G32_SINT:
			return EF_GR32I;

		case DXGI_FORMAT_R32G32B32_UINT:
			return EF_BGR32UI;

		case DXGI_FORMAT_R32G32B32_SINT:
			return EF_BGR32I;

		case DXGI_FORMAT_R32G32B32A32_UINT:
			return EF_ABGR32UI;

		case DXGI_FORMAT_R32G32B32A32_SINT:
			return EF_ABGR32I;

		case DXGI_FORMAT_R16_FLOAT:
			return EF_R16F;

		case DXGI_FORMAT_R16G16_FLOAT:
			return EF_GR16F;

		case DXGI_FORMAT_R11G11B10_FLOAT:
			return EF_B10G11R11F;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return EF_ABGR16F;

		case DXGI_FORMAT_R32_FLOAT:
			return EF_R32F;

		case DXGI_FORMAT_R32G32_FLOAT:
			return EF_GR32F;

		case DXGI_FORMAT_R32G32B32_FLOAT:
			return EF_BGR32F;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return EF_ABGR32F;

		case DXGI_FORMAT_BC1_UNORM:
			return EF_BC1;

		case DXGI_FORMAT_BC2_UNORM:
			return EF_BC2;

		case DXGI_FORMAT_BC3_UNORM:
			return EF_BC3;

		case DXGI_FORMAT_BC4_UNORM:
			return EF_BC4;

		case DXGI_FORMAT_BC4_SNORM:
			return EF_SIGNED_BC4;

		case DXGI_FORMAT_BC5_UNORM:
			return EF_BC5;

		case DXGI_FORMAT_BC5_SNORM:
			return EF_SIGNED_BC5;

		case DXGI_FORMAT_BC6H_UF16:
			return EF_BC6;

		case DXGI_FORMAT_BC6H_SF16:
			return EF_SIGNED_BC6;

		case DXGI_FORMAT_BC7_UNORM:
			return EF_BC7;

		case DXGI_FORMAT_D16_UNORM:
			return EF_D16;

		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return EF_D24S8;

		case DXGI_FORMAT_D32_FLOAT:
			return EF_D32F;

		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return EF_ABGR8_SRGB;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return EF_BC1_SRGB;

		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return EF_BC2_SRGB;

		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return EF_BC3_SRGB;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return EF_BC7_SRGB;

		default:
			THR(boost::system::posix_error::not_supported);
		}
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}
}
