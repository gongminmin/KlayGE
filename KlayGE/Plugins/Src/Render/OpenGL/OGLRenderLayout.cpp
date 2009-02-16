// OGLRenderLayout.cpp
// KlayGE OpenGL渲染分布类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://klayge.sourceforge.net
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

			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetVertexStream(i)));
				uint32_t const size = this->VertexSize(i);
				vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

				uint8_t* elem_offset = NULL;
				BOOST_FOREACH(BOOST_TYPEOF(vertex_stream_fmt)::const_reference vs_elem, vertex_stream_fmt)
				{
					GLvoid* offset = static_cast<GLvoid*>(elem_offset);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum const type = IsFloatFormat(vs_elem.format) ? GL_FLOAT : GL_UNSIGNED_BYTE;

					if (VEU_TextureCoord == vs_elem.usage)
					{
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						if (glloader_GL_EXT_direct_state_access())
						{
							stream.Active();
							glMultiTexCoordPointerEXT(GL_TEXTURE0 + vs_elem.usage_index, num_components, type, size, offset);
						}
						else
						{
							glClientActiveTexture(GL_TEXTURE0 + vs_elem.usage_index);
							stream.Active();
							glTexCoordPointer(num_components, type, size, offset);
						}
						break;
					}

					elem_offset += vs_elem.element_size();
				}
			}
		}

		if (dirty_vao_ || !use_vao_)
		{
			for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetVertexStream(i)));
				uint32_t const size = this->VertexSize(i);
				vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

				uint8_t* elem_offset = NULL;
				BOOST_FOREACH(BOOST_TYPEOF(vertex_stream_fmt)::const_reference vs_elem, vertex_stream_fmt)
				{
					GLvoid* offset = static_cast<GLvoid*>(elem_offset);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum const type = IsFloatFormat(vs_elem.format) ? GL_FLOAT : GL_UNSIGNED_BYTE;

					switch (vs_elem.usage)
					{
					case VEU_Position:
						glEnableClientState(GL_VERTEX_ARRAY);
						stream.Active();
						glVertexPointer(num_components, type, size, offset);
						break;

					case VEU_Normal:
						glEnableClientState(GL_NORMAL_ARRAY);
						stream.Active();
						glNormalPointer(type, size, offset);
						break;

					case VEU_Diffuse:
						glEnableClientState(GL_COLOR_ARRAY);
						stream.Active();
						glColorPointer(num_components, type, size, offset);
						break;

					case VEU_Specular:
						glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
						stream.Active();
						glSecondaryColorPointer(num_components, type, size, offset);
						break;

					case VEU_BlendWeight:
						glEnableVertexAttribArray(1);
						stream.Active();
						glVertexAttribPointer(1, num_components, type, GL_FALSE, size, offset);
						break;

					case VEU_BlendIndex:
						glEnableVertexAttribArray(7);
						stream.Active();
						glVertexAttribPointer(7, num_components, type, GL_FALSE, size, offset);
						break;

					case VEU_TextureCoord:
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						if (glloader_GL_EXT_direct_state_access())
						{
							stream.Active();
							glMultiTexCoordPointerEXT(GL_TEXTURE0 + vs_elem.usage_index, num_components, type, size, offset);
						}
						else
						{
							glClientActiveTexture(GL_TEXTURE0 + vs_elem.usage_index);
							stream.Active();
							glTexCoordPointer(num_components, type, size, offset);
						}
						break;

					case VEU_Tangent:
						glEnableVertexAttribArray(14);
						stream.Active();
						glVertexAttribPointer(14, num_components, type, GL_FALSE, size, offset);
						break;

					case VEU_Binormal:
						glEnableVertexAttribArray(15);
						stream.Active();
						glVertexAttribPointer(15, num_components, type, GL_FALSE, size, offset);
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}

					elem_offset += vs_elem.element_size();
				}
			}

			dirty_vao_ = false;
		}
	}
}
