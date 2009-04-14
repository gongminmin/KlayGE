// OGLRenderLayout.cpp
// KlayGE OpenGL渲染分布类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 使用glVertexAttribPointer (2009.3.28)
//
// 3.8.0
// 初次建立 (2009.2.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>

namespace KlayGE
{
	OGLRenderLayout::OGLRenderLayout()
		: dirty_vao_(true), vao_(0)
	{
		if (glloader_GL_VERSION_3_0() || glloader_GL_ARB_vertex_array_object())
		{
			use_vao_ = true;
			glGenVertexArrays(1, &vao_);
		}
		else
		{
			use_vao_ = false;
		}
		if ((!glloader_GL_VERSION_3_1()) && glloader_GL_NV_primitive_restart())
		{
			use_nv_pri_restart_ = true;
		}
		else
		{
			use_nv_pri_restart_ = false;
		}
	}

	OGLRenderLayout::~OGLRenderLayout()
	{
		if (use_vao_ && (vao_ != 0))
		{
			glDeleteVertexArrays(1, &vao_);
		}
	}

	void OGLRenderLayout::Active() const
	{
		if (use_vao_)
		{
			glBindVertexArray(vao_);
		}
		else
		{
			glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		}

		if (use_nv_pri_restart_)
		{
			ElementFormat format = this->IndexStreamFormat();
			if (format != EF_Unknown)
			{
				if (EF_R16UI == format)
				{
					glPrimitiveRestartIndexNV(0xFFFF);
				}
				else
				{
					BOOST_ASSERT(EF_R32UI == format);
					glPrimitiveRestartIndexNV(0xFFFFFFFF);
				}
				glEnableClientState(GL_PRIMITIVE_RESTART_NV);
			}
		}

		if (dirty_vao_ || !use_vao_)
		{
			// From http://www.gamedev.net/community/forums/topic.asp?topic_id=501785
			// POSITION, ATTR0				Input Vertex, Generic Attribute 0
			// BLENDWEIGHT, ATTR1			Input vertex weight, Generic Attribute 1
			// NORMAL, ATTR2				Input normal, Generic Attribute 2
			// COLOR0, DIFFUSE, ATTR3		Input primary color, Generic Attribute 3
			// COLOR1, SPECULAR, ATTR4		Input secondary color, Generic Attribute 4
			// TESSFACTOR, FOGCOORD,ATTR5	Input fog coordinate, Generic Attribute 5
			// PSIZE, ATTR6					Input point size, Generic Attribute 6
			// BLENDINDICES, ATTR7			Generic Attribute 7
			// TEXCOORD0-TEXCOORD7,
			// 		ATTR8-ATTR15			Input texture coordinates (texcoord0-texcoord7), Generic Attributes 8-15
			// TANGENT, ATTR14				Generic Attribute 14
			// BINORMAL, ATTR15				Generic Attribute 15
			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetVertexStream(i)));
				uint32_t const size = this->VertexSize(i);
				vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

				uint8_t* elem_offset = NULL;
				BOOST_FOREACH(BOOST_TYPEOF(vertex_stream_fmt)::const_reference vs_elem, vertex_stream_fmt)
				{
					uint32_t attr;
					switch (vs_elem.usage)
					{
					case VEU_Position:
						attr = 0;
						break;

					case VEU_Normal:
						attr = 2;
						break;

					case VEU_Diffuse:
						attr = 3;
						break;

					case VEU_Specular:
						attr = 4;
						break;

					case VEU_BlendWeight:
						attr = 1;
						break;

					case VEU_BlendIndex:
						attr = 7;
						break;

					case VEU_TextureCoord:
						attr = 8 + vs_elem.usage_index;
						break;

					case VEU_Tangent:
						attr = 14;
						break;

					case VEU_Binormal:
						attr = 15;
						break;

					default:
						attr = 0;
						BOOST_ASSERT(false);
						break;
					}

					GLvoid* offset = static_cast<GLvoid*>(elem_offset);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum const type = IsFloatFormat(vs_elem.format) ? GL_FLOAT : GL_UNSIGNED_BYTE;

					glEnableVertexAttribArray(attr);
					stream.Active();
					glVertexAttribPointer(attr, num_components, type, GL_FALSE, size, offset);

					elem_offset += vs_elem.element_size();
				}
			}

			dirty_vao_ = false;
		}
	}

	void OGLRenderLayout::Deactive() const
	{
		if (!use_vao_)
		{
			glPopClientAttrib();
		}
	}
}
