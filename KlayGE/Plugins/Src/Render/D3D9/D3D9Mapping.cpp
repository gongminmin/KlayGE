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
#include <KlayGE/VertexBuffer.hpp>

#define NOMINMAX
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <boost/assert.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>

namespace KlayGE
{
	// 从KlayGE的Matrix4转换到D3DMATRIX
	/////////////////////////////////////////////////////////////////////////////////
	D3DMATRIX D3D9Mapping::Mapping(Matrix4 const & mat)
	{
		D3DMATRIX d3dMat;
		std::copy(mat.begin(), mat.end(), &d3dMat._11);

		return d3dMat;
	}

	// 从D3DMATRIX转换到KlayGE的Matrix4
	/////////////////////////////////////////////////////////////////////////////////
	Matrix4 D3D9Mapping::Mapping(D3DMATRIX const & mat)
	{
		return Matrix4(&mat.m[0][0]);
	}

	// 从KlayGE的Color转换到D3DCOLORVALUE
	/////////////////////////////////////////////////////////////////////////////////
	D3DVECTOR D3D9Mapping::Mapping(Vector3 const & vec)
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

	uint32_t D3D9Mapping::Mapping(RenderEngine::FillMode mode)
	{
		switch (mode)
		{
		case RenderEngine::FM_Point:
			return D3DFILL_POINT;

		case RenderEngine::FM_Line:
			return D3DFILL_WIREFRAME;

		case RenderEngine::FM_Fill:
			return D3DFILL_SOLID;

		default:
			BOOST_ASSERT(false);
			return D3DFILL_POINT;
		}
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::ShadeOptions so)
	{
		switch (so)
		{
		case RenderEngine::SO_Flat:
			return D3DSHADE_FLAT;

		case RenderEngine::SO_Gouraud:
			return D3DSHADE_GOURAUD;

		case RenderEngine::SO_Phong:
			return D3DSHADE_PHONG;

		default:
			BOOST_ASSERT(false);
			return D3DSHADE_FLAT;
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

	void D3D9Mapping::Mapping(D3DPRIMITIVETYPE& primType, uint32_t& primCount, VertexBuffer const & vb)
	{
		uint32_t const vertexCount(static_cast<uint32_t>(vb.UseIndices() ? vb.NumIndices() : vb.NumVertices()));
		switch (vb.Type())
		{
		case VertexBuffer::BT_PointList:
			primType = D3DPT_POINTLIST;
			primCount = vertexCount;
			break;

		case VertexBuffer::BT_LineList:
			primType = D3DPT_LINELIST;
			primCount = vertexCount / 2;
			break;

		case VertexBuffer::BT_LineStrip:
			primType = D3DPT_LINESTRIP;
			primCount = vertexCount - 1;
			break;

		case VertexBuffer::BT_TriangleList:
			primType = D3DPT_TRIANGLELIST;
			primCount = vertexCount / 3;
			break;

		case VertexBuffer::BT_TriangleStrip:
			primType = D3DPT_TRIANGLESTRIP;
			primCount = vertexCount - 2;
			break;

		case VertexBuffer::BT_TriangleFan:
			primType = D3DPT_TRIANGLEFAN;
			primCount = vertexCount - 2;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void D3D9Mapping::Mapping(std::vector<D3DVERTEXELEMENT9>& elements, size_t stream, VertexStream const & vs)
	{
		elements.resize(vs.NumElements());

		uint16_t elem_offset = 0;
		for (uint32_t i = 0; i < vs.NumElements(); ++ i)
		{
			vertex_element const & vs_elem = vs.Element(i);

			D3DVERTEXELEMENT9& element = elements[i];
			element.Type		= D3DDECLTYPE_FLOAT1 - 1 + vs_elem.num_components;
			element.Offset		= elem_offset;
			element.Method		= D3DDECLMETHOD_DEFAULT;
			element.Stream		= static_cast<WORD>(stream);
			element.UsageIndex	= vs_elem.usage_index;

			switch (vs_elem.usage)
			{
			// Vertex xyzs
			case VEU_Position:
				element.Usage		= D3DDECLUSAGE_POSITION;
				break;

			// Normal
			case VEU_Normal:
				element.Usage		= D3DDECLUSAGE_NORMAL;
				break;

			// Vertex colors
			case VEU_Diffuse:
				element.Usage		= D3DDECLUSAGE_COLOR;
				break;

			// Vertex speculars
			case VEU_Specular:
				element.Usage		= D3DDECLUSAGE_COLOR;
				break;
			
			// Blend Weights
			case VEU_BlendWeight:
				element.Usage		= D3DDECLUSAGE_BLENDWEIGHT;
				break;

			// Blend Indices
			case VEU_BlendIndex:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_BLENDINDICES;
				break;

			// Do texture coords
			case VEU_TextureCoord:
				element.Usage		= D3DDECLUSAGE_TEXCOORD;
				break;

			case VEU_Tangent:
				element.Usage		= D3DDECLUSAGE_TANGENT;
				break;

			case VEU_Binormal:
				element.Usage		= D3DDECLUSAGE_BINORMAL;
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

		ret.min_point_size = 0;
		ret.max_point_size = d3d_caps.MaxPointSize;

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
				// Notify the driver that instancing support is expected
				renderEngine.D3DDevice()->SetRenderState(D3DRS_POINTSIZE, MAKEFOURCC('I', 'N', 'S', 'T'));
			}
			else
			{
				ret.hw_instancing_support = false;
			}
		}

		return ret;
	}
}
