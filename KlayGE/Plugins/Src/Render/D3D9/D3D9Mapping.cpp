// D3D9Mapping.hpp
// KlayGE RenderEngine和D3D9本地之间的映射 实现文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 初次建立 (2005.3.20)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/VertexBuffer.hpp>

#include <cassert>

#include <d3dx9.h>
#include <d3dx9.h>
#include <dxerr9.h>

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
			assert(false);
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
			return D3DSTENCILOP_INCRSAT;

		case RenderEngine::SOP_Decrement:
			return D3DSTENCILOP_DECRSAT;

		case RenderEngine::SOP_Invert:
			return D3DSTENCILOP_INVERT;

		default:
			assert(false);
			return D3DSTENCILOP_KEEP;
		};
	}

	// 从RenderEngine::TexFiltering转换到D3D的MagFilter标志
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D9Mapping::MappingToMagFilter(D3DCAPS9 const & caps, Texture::TexFilterOp tf)
	{
		// NOTE: Fall through if device doesn't support requested type
		if ((Texture::TFO_Anisotropic == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC))
		{
			return D3DTEXF_ANISOTROPIC;
		}
		else
		{
			tf = Texture::TFO_Trilinear;
		}

		if ((Texture::TFO_Trilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = Texture::TFO_Bilinear;
		}

		if ((Texture::TFO_Bilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}

		return D3DTEXF_POINT;
	}

	// 从RenderEngine::TexFiltering转换到D3D的MinFilter标志
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D9Mapping::MappingToMinFilter(D3DCAPS9 const & caps, Texture::TexFilterOp tf)
	{
		// NOTE: Fall through if device doesn't support requested type
		if ((Texture::TFO_Anisotropic == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC))
		{
			return D3DTEXF_ANISOTROPIC;
		}
		else
		{
			tf = Texture::TFO_Trilinear;
		}

		if ((Texture::TFO_Trilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = Texture::TFO_Bilinear;
		}

		if ((Texture::TFO_Bilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}

		return D3DTEXF_POINT;
	}

	// 从RenderEngine::TexFiltering转换到D3D的MipFilter标志
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D9Mapping::MappingToMipFilter(D3DCAPS9 const & caps, Texture::TexFilterOp tf)
	{
		// NOTE: Fall through if device doesn't support requested type
		if ((Texture::TFO_Anisotropic == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = Texture::TFO_Trilinear;
		}

		if ((Texture::TFO_Trilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = Texture::TFO_Bilinear;
		}

		if ((Texture::TFO_Bilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
		{
			return D3DTEXF_POINT;
		}

		return D3DTEXF_NONE;
	}

	D3DLIGHTTYPE D3D9Mapping::Mapping(Light::LightTypes type)
	{
		switch (type)
		{
		case Light::LT_Point:
			return D3DLIGHT_POINT;

		case Light::LT_Directional:
			return D3DLIGHT_DIRECTIONAL;

		case Light::LT_Spot:
			return D3DLIGHT_SPOT;

		default:
			assert(false);
			return D3DLIGHT_POINT;
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
			assert(false);
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
			assert(false);
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
			assert(false);
			return D3DSHADE_FLAT;
		}
	}

	uint32_t D3D9Mapping::Mapping(RenderEngine::FogMode mode)
	{
		switch (mode)
		{
		case RenderEngine::Fog_Linear:
			return D3DFOG_LINEAR;

		case RenderEngine::Fog_Exp:
			return D3DFOG_EXP;

		case RenderEngine::Fog_Exp2:
			return D3DFOG_EXP2;

		default:
			assert(false);
			return D3DFOG_LINEAR;
		}
	}

	uint32_t D3D9Mapping::Mapping(Texture::TexAddressingMode mode)
	{
		switch (mode)
		{
		case Texture::TAM_Clamp:
			return D3DTADDRESS_CLAMP;

		case Texture::TAM_Wrap:
			return D3DTADDRESS_WRAP;

		case Texture::TAM_Mirror:
			return D3DTADDRESS_MIRROR;

		default:
			assert(false);
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
			assert(false);
			break;
		}
	}

	void D3D9Mapping::Mapping(D3DVERTEXELEMENT9& element, size_t stream, VertexStream const & vs)
	{
		element.Offset = 0;
		element.Method = D3DDECLMETHOD_DEFAULT;
		element.Stream = static_cast<WORD>(stream);

		VertexStreamType type(vs.Type());

		switch (type)
		{
		// Vertex xyzs
		case VST_Positions:
			element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<BYTE>(vs.ElementsPerVertex());
			element.Usage		= D3DDECLUSAGE_POSITION;
			element.UsageIndex	= 0;
			break;

		// Normal
		case VST_Normals:
			element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<BYTE>(vs.ElementsPerVertex());
			element.Usage		= D3DDECLUSAGE_NORMAL;
			element.UsageIndex	= 0;
			break;

		// Vertex colors
		case VST_Diffuses:
			element.Type		= D3DDECLTYPE_D3DCOLOR;
			element.Usage		= D3DDECLUSAGE_COLOR;
			element.UsageIndex	= 0;
			break;

		// Vertex speculars
		case VST_Speculars:
			element.Type		= D3DDECLTYPE_D3DCOLOR;
			element.Usage		= D3DDECLUSAGE_COLOR;
			element.UsageIndex	= 1;
			break;
		
		// Blend Weights
		case VST_BlendWeights:
			element.Type		= D3DDECLTYPE_FLOAT4;
			element.Usage		= D3DDECLUSAGE_BLENDWEIGHT;
			element.UsageIndex	= 0;
			break;

		// Blend Indices
		case VST_BlendIndices:
			element.Type		= D3DDECLTYPE_D3DCOLOR;
			element.Usage		= D3DDECLUSAGE_BLENDINDICES;
			element.UsageIndex	= 0;
			break;

		// Do texture coords
		case VST_TextureCoords0:
		case VST_TextureCoords1:
		case VST_TextureCoords2:
		case VST_TextureCoords3:
		case VST_TextureCoords4:
		case VST_TextureCoords5:
		case VST_TextureCoords6:
		case VST_TextureCoords7:
			element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<BYTE>(vs.ElementsPerVertex());
			element.Usage		= D3DDECLUSAGE_TEXCOORD;
			element.UsageIndex	= static_cast<BYTE>(type - VST_TextureCoords0);
			break;
		}
	}
}
