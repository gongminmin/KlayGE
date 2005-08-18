// OGLMapping.hpp
// KlayGE RenderEngine和OpenGL本地之间的映射 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.19)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/VertexBuffer.hpp>

#include <boost/assert.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>

namespace KlayGE
{
	// 从KlayGE的Color转换到float[4]
	/////////////////////////////////////////////////////////////////////////////////
	void OGLMapping::Mapping(GLfloat* clr4, Color const & clr)
	{
		clr4[0] = clr.r();
		clr4[1] = clr.g();
		clr4[2] = clr.b();
		clr4[3] = clr.a();
	}

	// 从RenderEngine::CompareFunction转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLMapping::Mapping(RenderEngine::CompareFunction func)
	{
		switch (func)
		{
		case RenderEngine::CF_AlwaysFail:
			return GL_NEVER;

		case RenderEngine::CF_AlwaysPass:
			return GL_ALWAYS;

		case RenderEngine::CF_Less:
			return GL_LESS;

		case RenderEngine::CF_LessEqual:
			return GL_LEQUAL;

		case RenderEngine::CF_Equal:
			return GL_EQUAL;

		case RenderEngine::CF_NotEqual:
			return GL_NOTEQUAL;

		case RenderEngine::CF_GreaterEqual:
			return GL_GEQUAL;

		case RenderEngine::CF_Greater:
			return GL_GREATER;

		default:
			BOOST_ASSERT(false);
			return GL_EQUAL;
		};
	}

	// 从RenderEngine::AlphaBlendFactor转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLMapping::Mapping(RenderEngine::AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case RenderEngine::ABF_Zero:
			return GL_ZERO;

		case RenderEngine::ABF_One:
			return GL_ONE;

		case RenderEngine::ABF_Src_Alpha:
			return GL_SRC_ALPHA;

		case RenderEngine::ABF_Dst_Alpha:
			return GL_DST_ALPHA;

		case RenderEngine::ABF_Inv_Src_Alpha:
			return GL_ONE_MINUS_SRC_ALPHA;

		case RenderEngine::ABF_Inv_Dst_Alpha:
			return GL_ONE_MINUS_DST_ALPHA;

		case RenderEngine::ABF_Src_Color:
			return GL_SRC_COLOR;

		case RenderEngine::ABF_Dst_Color:
			return GL_DST_COLOR;

		case RenderEngine::ABF_Inv_Src_Color:
			return GL_ONE_MINUS_SRC_COLOR;

		case RenderEngine::ABF_Inv_Dst_Color:
			return GL_ONE_MINUS_DST_COLOR;

		case RenderEngine::ABF_Src_Alpha_Sat:
			return GL_SRC_ALPHA_SATURATE;

		default:
			BOOST_ASSERT(false);
			return GL_ZERO;
		}
	}

	// 从RenderEngine::StencilOperation转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLMapping::Mapping(RenderEngine::StencilOperation op)
	{
		switch (op)
		{
		case RenderEngine::SOP_Keep:
			return GL_KEEP;

		case RenderEngine::SOP_Zero:
			return GL_ZERO;

		case RenderEngine::SOP_Replace:
			return GL_REPLACE;

		case RenderEngine::SOP_Increment:
			return GL_INCR;

		case RenderEngine::SOP_Decrement:
			return GL_DECR;

		case RenderEngine::SOP_Invert:
			return GL_INVERT;

		default:
			BOOST_ASSERT(false);
			return GL_KEEP;
		};
	}

	GLenum OGLMapping::Mapping(RenderEngine::FillMode mode)
	{
		switch (mode)
		{
		case RenderEngine::FM_Point:
			return GL_POINT;

		case RenderEngine::FM_Line:
			return GL_LINE;

		case RenderEngine::FM_Fill:
			return GL_FILL;

		default:
			BOOST_ASSERT(false);
			return GL_FILL;
		}
	}

	GLenum OGLMapping::Mapping(RenderEngine::ShadeOptions so)
	{
		switch (so)
		{
		case RenderEngine::SO_Flat:
			return GL_FLAT;

		case RenderEngine::SO_Gouraud:
			return GL_SMOOTH;

		case RenderEngine::SO_Phong:
			return GL_SMOOTH;

		default:
			BOOST_ASSERT(false);
			return GL_FLAT;
		}
	}

	GLint OGLMapping::Mapping(Sampler::TexAddressingMode mode)
	{
		switch (mode)
		{
		case Sampler::TAM_Wrap:
			return GL_REPEAT;

		case Sampler::TAM_Mirror:
			return GL_MIRRORED_REPEAT;

		case Sampler::TAM_Clamp:
			return GL_CLAMP_TO_EDGE;

		default:
			BOOST_ASSERT(false);
			return GL_REPEAT;
		}
	}

	void OGLMapping::Mapping(GLenum& primType, uint32_t& primCount, VertexBuffer const & vb)
	{
		uint32_t const vertexCount = static_cast<uint32_t>(vb.UseIndices() ? vb.NumIndices() : vb.NumVertices());
		primType = GL_POINTS;
		primCount = vertexCount;
		switch (vb.Type())
		{
		case VertexBuffer::BT_PointList:
			primType = GL_POINTS;
			primCount = vertexCount;
			break;

		case VertexBuffer::BT_LineList:
			primType = GL_LINES;
			primCount = vertexCount / 2;
			break;

		case VertexBuffer::BT_LineStrip:
			primType = GL_LINE_STRIP;
			primCount = vertexCount - 1;
			break;

		case VertexBuffer::BT_TriangleList:
			primType = GL_TRIANGLES;
			primCount = vertexCount / 3;
			break;

		case VertexBuffer::BT_TriangleStrip:
			primType = GL_TRIANGLE_STRIP;
			primCount = vertexCount - 2;
			break;

		case VertexBuffer::BT_TriangleFan:
			primType = GL_TRIANGLE_FAN;
			primCount = vertexCount - 2;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}
