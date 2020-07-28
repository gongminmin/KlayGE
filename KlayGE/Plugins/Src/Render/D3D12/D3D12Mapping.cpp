/**
 * @file D3D12Mapping.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/GraphicsBuffer.hpp>

#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>

namespace KlayGE
{
	// 从KlayGE的ColorMask转换到D3DCOLORWRITEENABLE所使用的格式
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D12Mapping::MappingColorMask(uint32_t mask)
	{
		uint32_t ret = 0;
		if (mask & CMASK_Red)
		{
			ret |= D3D12_COLOR_WRITE_ENABLE_RED;
		}
		if (mask & CMASK_Green)
		{
			ret |= D3D12_COLOR_WRITE_ENABLE_GREEN;
		}
		if (mask & CMASK_Blue)
		{
			ret |= D3D12_COLOR_WRITE_ENABLE_BLUE;
		}
		if (mask & CMASK_Alpha)
		{
			ret |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}
		return ret;
	}

	D3D12_COMPARISON_FUNC D3D12Mapping::Mapping(CompareFunction func)
	{
		switch (func)
		{
		case CF_AlwaysFail:
			return D3D12_COMPARISON_FUNC_NEVER;

		case CF_AlwaysPass:
			return D3D12_COMPARISON_FUNC_ALWAYS;

		case CF_Less:
			return D3D12_COMPARISON_FUNC_LESS;

		case CF_LessEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;

		case CF_Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;

		case CF_NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;

		case CF_GreaterEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;

		case CF_Greater:
			return D3D12_COMPARISON_FUNC_GREATER;

		default:
			KFL_UNREACHABLE("Invalid compare function");
		};
	}

	D3D12_STENCIL_OP D3D12Mapping::Mapping(StencilOperation op)
	{
		switch (op)
		{
		case SOP_Keep:
			return D3D12_STENCIL_OP_KEEP;

		case SOP_Zero:
			return D3D12_STENCIL_OP_ZERO;

		case SOP_Replace:
			return D3D12_STENCIL_OP_REPLACE;

		case SOP_Incr:
			return D3D12_STENCIL_OP_INCR_SAT;

		case SOP_Decr:
			return D3D12_STENCIL_OP_DECR_SAT;

		case SOP_Invert:
			return D3D12_STENCIL_OP_INVERT;

		case SOP_Incr_Wrap:
			return D3D12_STENCIL_OP_INCR;

		case SOP_Decr_Wrap:
			return D3D12_STENCIL_OP_DECR;

		default:
			KFL_UNREACHABLE("Invalid stencil operation");
		};
	}

	D3D12_BLEND D3D12Mapping::Mapping(AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case ABF_Zero:
			return D3D12_BLEND_ZERO;

		case ABF_One:
			return D3D12_BLEND_ONE;

		case ABF_Src_Alpha:
			return D3D12_BLEND_SRC_ALPHA;

		case ABF_Dst_Alpha:
			return D3D12_BLEND_DEST_ALPHA;

		case ABF_Inv_Src_Alpha:
			return D3D12_BLEND_INV_SRC_ALPHA;

		case ABF_Inv_Dst_Alpha:
			return D3D12_BLEND_INV_DEST_ALPHA;

		case ABF_Src_Color:
			return D3D12_BLEND_SRC_COLOR;

		case ABF_Dst_Color:
			return D3D12_BLEND_DEST_COLOR;

		case ABF_Inv_Src_Color:
			return D3D12_BLEND_INV_SRC_COLOR;

		case ABF_Inv_Dst_Color:
			return D3D12_BLEND_INV_DEST_COLOR;

		case ABF_Src_Alpha_Sat:
			return D3D12_BLEND_SRC_ALPHA_SAT;

		case ABF_Blend_Factor:
			return D3D12_BLEND_BLEND_FACTOR;

		case ABF_Inv_Blend_Factor:
			return D3D12_BLEND_INV_BLEND_FACTOR;

		case ABF_Src1_Alpha:
			return D3D12_BLEND_SRC1_ALPHA;

		case ABF_Inv_Src1_Alpha:
			return D3D12_BLEND_INV_SRC1_ALPHA;

		case ABF_Src1_Color:
			return D3D12_BLEND_SRC1_COLOR;

		case ABF_Inv_Src1_Color:
			return D3D12_BLEND_INV_SRC1_COLOR;

		default:
			KFL_UNREACHABLE("Invalid alpha blend factor");
		}
	}

	D3D12_CULL_MODE D3D12Mapping::Mapping(CullMode mode)
	{
		switch (mode)
		{
		case CM_None:
			return D3D12_CULL_MODE_NONE;

		case CM_Front:
			return D3D12_CULL_MODE_FRONT;

		case CM_Back:
			return D3D12_CULL_MODE_BACK;

		default:
			KFL_UNREACHABLE("Invalid cull mode");
		}
	}

	D3D12_FILL_MODE D3D12Mapping::Mapping(PolygonMode mode)
	{
		switch (mode)
		{
		case PM_Point:
			return D3D12_FILL_MODE_WIREFRAME;

		case PM_Line:
			return D3D12_FILL_MODE_WIREFRAME;

		case PM_Fill:
			return D3D12_FILL_MODE_SOLID;

		default:
			KFL_UNREACHABLE("Invalid polygon mode");
		}
	}

	D3D12_BLEND_OP D3D12Mapping::Mapping(BlendOperation bo)
	{
		switch (bo)
		{
		case BOP_Add:
			return D3D12_BLEND_OP_ADD;

		case BOP_Sub:
			return D3D12_BLEND_OP_SUBTRACT;

		case BOP_Rev_Sub:
			return D3D12_BLEND_OP_REV_SUBTRACT;

		case BOP_Min:
			return D3D12_BLEND_OP_MIN;

		case BOP_Max:
			return D3D12_BLEND_OP_MAX;

		default:
			KFL_UNREACHABLE("Invalid blend operation");
		}
	}

	D3D12_TEXTURE_ADDRESS_MODE D3D12Mapping::Mapping(TexAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_Clamp:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

		case TAM_Wrap:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;

		case TAM_Mirror:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;

		case TAM_Border:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;

		default:
			KFL_UNREACHABLE("Invalid texture addressing mode");
		}
	}

	D3D12_FILTER D3D12Mapping::Mapping(TexFilterOp filter)
	{
		switch (filter)
		{
		case TFO_Min_Mag_Mip_Point:
			return D3D12_FILTER_MIN_MAG_MIP_POINT;

		case TFO_Min_Mag_Point_Mip_Linear:
			return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;

		case TFO_Min_Point_Mag_Linear_Mip_Point:
			return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

		case TFO_Min_Point_Mag_Mip_Linear:
			return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;

		case TFO_Min_Mag_Linear_Mip_Point:
			return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		case TFO_Min_Mag_Mip_Linear:
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;

		case TFO_Anisotropic:
			return D3D12_FILTER_ANISOTROPIC;

		case TFO_Cmp_Min_Mag_Mip_Point:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

		case TFO_Cmp_Min_Mag_Point_Mip_Linear:
			return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;

		case TFO_Cmp_Min_Point_Mag_Linear_Mip_Point:
			return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;

		case TFO_Cmp_Min_Point_Mag_Mip_Linear:
			return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;

		case TFO_Cmp_Min_Mag_Linear_Mip_Point:
			return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

		case TFO_Cmp_Min_Mag_Mip_Linear:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

		case TFO_Cmp_Anisotropic:
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;

		default:
			KFL_UNREACHABLE("Invalid texture filter operation");
		}
	}

	D3D12_DEPTH_WRITE_MASK D3D12Mapping::Mapping(bool depth_write_mask)
	{
		if (depth_write_mask)
		{
			return D3D12_DEPTH_WRITE_MASK_ALL;
		}
		else
		{
			return D3D12_DEPTH_WRITE_MASK_ZERO;
		}
	}

	D3D12_LOGIC_OP D3D12Mapping::Mapping(LogicOperation lo)
	{
		switch (lo)
		{
		case LOP_Clear:
			return D3D12_LOGIC_OP_CLEAR;

		case LOP_Set:
			return D3D12_LOGIC_OP_SET;

		case LOP_Copy:
			return D3D12_LOGIC_OP_COPY;

		case LOP_CopyInverted:
			return D3D12_LOGIC_OP_COPY_INVERTED;

		case LOP_Noop:
			return D3D12_LOGIC_OP_NOOP;

		case LOP_Invert:
			return D3D12_LOGIC_OP_INVERT;

		case LOP_And:
			return D3D12_LOGIC_OP_AND;

		case LOP_NAnd:
			return D3D12_LOGIC_OP_NAND;

		case LOP_Or:
			return D3D12_LOGIC_OP_OR;

		case LOP_NOR:
			return D3D12_LOGIC_OP_NOR;

		case LOP_XOR:
			return D3D12_LOGIC_OP_XOR;

		case LOP_Equiv:
			return D3D12_LOGIC_OP_EQUIV;

		case LOP_AndReverse:
			return D3D12_LOGIC_OP_AND_REVERSE;

		case LOP_AndInverted:
			return D3D12_LOGIC_OP_AND_INVERTED;

		case LOP_OrReverse:
			return D3D12_LOGIC_OP_OR_REVERSE;

		case LOP_OrInverted:
			return D3D12_LOGIC_OP_OR_INVERTED;

		default:
			KFL_UNREACHABLE("Invalid logic operation");
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY D3D12Mapping::Mapping(RenderLayout::topology_type tt)
	{
		switch (tt)
		{
		case RenderLayout::TT_PointList:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

		case RenderLayout::TT_LineList:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

		case RenderLayout::TT_LineStrip:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

		case RenderLayout::TT_TriangleList:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		case RenderLayout::TT_TriangleStrip:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		case RenderLayout::TT_LineList_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

		case RenderLayout::TT_LineStrip_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

		case RenderLayout::TT_TriangleList_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;

		case RenderLayout::TT_TriangleStrip_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;

		case RenderLayout::TT_1_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_2_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_3_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_4_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_5_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_6_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_7_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_8_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_9_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_10_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_11_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_12_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_13_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_14_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_15_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_16_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_17_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_18_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_19_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_20_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_21_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_22_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_23_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_24_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_25_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_26_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_27_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_28_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_29_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_30_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_31_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;

		case RenderLayout::TT_32_Ctrl_Pt_PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;

		default:
			KFL_UNREACHABLE("Invalid topology type");
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Mapping::MappingPriTopoType(RenderLayout::topology_type tt)
	{
		switch (tt)
		{
		case RenderLayout::TT_PointList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		case RenderLayout::TT_LineList:
		case RenderLayout::TT_LineStrip:
		case RenderLayout::TT_LineList_Adj:
		case RenderLayout::TT_LineStrip_Adj:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

		case RenderLayout::TT_TriangleList:
		case RenderLayout::TT_TriangleStrip:
		case RenderLayout::TT_TriangleList_Adj:
		case RenderLayout::TT_TriangleStrip_Adj:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		case RenderLayout::TT_1_Ctrl_Pt_PatchList:
		case RenderLayout::TT_2_Ctrl_Pt_PatchList:
		case RenderLayout::TT_3_Ctrl_Pt_PatchList:
		case RenderLayout::TT_4_Ctrl_Pt_PatchList:
		case RenderLayout::TT_5_Ctrl_Pt_PatchList:
		case RenderLayout::TT_6_Ctrl_Pt_PatchList:
		case RenderLayout::TT_7_Ctrl_Pt_PatchList:
		case RenderLayout::TT_8_Ctrl_Pt_PatchList:
		case RenderLayout::TT_9_Ctrl_Pt_PatchList:
		case RenderLayout::TT_10_Ctrl_Pt_PatchList:
		case RenderLayout::TT_11_Ctrl_Pt_PatchList:
		case RenderLayout::TT_12_Ctrl_Pt_PatchList:
		case RenderLayout::TT_13_Ctrl_Pt_PatchList:
		case RenderLayout::TT_14_Ctrl_Pt_PatchList:
		case RenderLayout::TT_15_Ctrl_Pt_PatchList:
		case RenderLayout::TT_16_Ctrl_Pt_PatchList:
		case RenderLayout::TT_17_Ctrl_Pt_PatchList:
		case RenderLayout::TT_18_Ctrl_Pt_PatchList:
		case RenderLayout::TT_19_Ctrl_Pt_PatchList:
		case RenderLayout::TT_20_Ctrl_Pt_PatchList:
		case RenderLayout::TT_21_Ctrl_Pt_PatchList:
		case RenderLayout::TT_22_Ctrl_Pt_PatchList:
		case RenderLayout::TT_23_Ctrl_Pt_PatchList:
		case RenderLayout::TT_24_Ctrl_Pt_PatchList:
		case RenderLayout::TT_25_Ctrl_Pt_PatchList:
		case RenderLayout::TT_26_Ctrl_Pt_PatchList:
		case RenderLayout::TT_27_Ctrl_Pt_PatchList:
		case RenderLayout::TT_28_Ctrl_Pt_PatchList:
		case RenderLayout::TT_29_Ctrl_Pt_PatchList:
		case RenderLayout::TT_30_Ctrl_Pt_PatchList:
		case RenderLayout::TT_31_Ctrl_Pt_PatchList:
		case RenderLayout::TT_32_Ctrl_Pt_PatchList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

		default:
			KFL_UNREACHABLE("Invalid topology type");
		}
	}

	void D3D12Mapping::Mapping(std::vector<D3D12_INPUT_ELEMENT_DESC>& elements, size_t stream, std::span<VertexElement const> vet,
		RenderLayout::stream_type type, uint32_t freq)
	{
		elements.resize(vet.size());

		uint16_t elem_offset = 0;
		for (uint32_t i = 0; i < elements.size(); ++ i)
		{
			VertexElement const & vs_elem = vet[i];

			D3D12_INPUT_ELEMENT_DESC& element = elements[i];
			element.SemanticIndex = vs_elem.usage_index;
			element.Format = D3D12Mapping::MappingFormat(vs_elem.format);
			element.InputSlot = static_cast<WORD>(stream);
			element.AlignedByteOffset = elem_offset;
			if (RenderLayout::ST_Geometry == type)
			{
				element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				element.InstanceDataStepRate = 0;
			}
			else
			{
				BOOST_ASSERT(RenderLayout::ST_Instance == type);

				element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
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

	D3D12_SO_DECLARATION_ENTRY D3D12Mapping::Mapping(ShaderDesc::StreamOutputDecl const & decl)
	{
		D3D12_SO_DECLARATION_ENTRY ret;

		ret.Stream = 0;
		ret.SemanticIndex = decl.usage_index;
		ret.StartComponent = decl.start_component;
		ret.ComponentCount = decl.component_count;
		ret.OutputSlot = decl.slot;
		switch (decl.usage)
		{
		// Vertex xyzs
		case VEU_Position:
			ret.SemanticName = "SV_Position";
			break;

		// Normal
		case VEU_Normal:
			ret.SemanticName = "NORMAL";
			break;

		// Vertex colors
		case VEU_Diffuse:
			ret.SemanticName = "COLOR";
			break;

		// Vertex speculars
		case VEU_Specular:
			ret.SemanticName = "COLOR";
			break;

		// Blend Weights
		case VEU_BlendWeight:
			ret.SemanticName = "BLENDWEIGHT";
			break;

		// Blend Indices
		case VEU_BlendIndex:
			ret.SemanticName = "BLENDINDICES";
			break;

		// Do texture coords
		case VEU_TextureCoord:
			ret.SemanticName = "TEXCOORD";
			break;

		case VEU_Tangent:
			ret.SemanticName = "TANGENT";
			break;

		case VEU_Binormal:
			ret.SemanticName = "BINORMAL";
			break;
		}

		return ret;
	}

	DXGI_FORMAT D3D12Mapping::MappingFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_A8:
			return DXGI_FORMAT_A8_UNORM;

		case EF_R5G6B5:
			return DXGI_FORMAT_B5G6R5_UNORM;

		case EF_A1RGB5:
			return DXGI_FORMAT_B5G5R5A1_UNORM;

		case EF_ARGB4:
			return DXGI_FORMAT_B4G4R4A4_UNORM;

		case EF_R8:
			return DXGI_FORMAT_R8_UNORM;

		case EF_SIGNED_R8:
			return DXGI_FORMAT_R8_SNORM;

		case EF_GR8:
			return DXGI_FORMAT_R8G8_UNORM;

		case EF_SIGNED_GR8:
			return DXGI_FORMAT_R8G8_SNORM;

		case EF_ARGB8:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case EF_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case EF_SIGNED_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

		case EF_A2BGR10:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case EF_SIGNED_A2BGR10:
			return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

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

		case EF_X24G8:
			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

		case EF_D32F:
			return DXGI_FORMAT_D32_FLOAT;
			
		case EF_ARGB8_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

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
			KFL_UNREACHABLE("Invalid format");
		}
	}

	ElementFormat D3D12Mapping::MappingFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_A8_UNORM:
			return EF_A8;

		case DXGI_FORMAT_B5G6R5_UNORM:
			return EF_R5G6B5;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return EF_A1RGB5;

		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return EF_ARGB4;

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

		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return EF_SIGNED_A2BGR10;

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

		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return EF_X24G8;

		case DXGI_FORMAT_D32_FLOAT:
			return EF_D32F;

		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return EF_ARGB8_SRGB;

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
			KFL_UNREACHABLE("Invalid format");
		}
	}
}
