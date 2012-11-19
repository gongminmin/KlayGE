// OGLESRenderLayout.cpp
// KlayGE OpenGL ES 2渲染分布类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
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

#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>
#include <KlayGE/OpenGLES/OGLESRenderLayout.hpp>

namespace KlayGE
{
	OGLESRenderLayout::OGLESRenderLayout()
	{
	}

	OGLESRenderLayout::~OGLESRenderLayout()
	{
	}

	void OGLESRenderLayout::Active(ShaderObjectPtr const & so) const
	{
		OGLES2ShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLESShaderObject>(so);

		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetVertexStream(i)));
			uint32_t const size = this->VertexSize(i);
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			uint8_t* elem_offset = nullptr;
			typedef BOOST_TYPEOF(vertex_stream_fmt) VertexStreamFmtType;
			BOOST_FOREACH(VertexStreamFmtType::const_reference vs_elem, vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLvoid* offset = static_cast<GLvoid*>(elem_offset + this->StartVertexLocation() * size);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLESMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;

					glEnableVertexAttribArray(attr);
					stream.Active();
					glVertexAttribPointer(attr, num_components, type, normalized, size, offset);
				}

				elem_offset += vs_elem.element_size();
			}
		}
	}

	void OGLESRenderLayout::Deactive(ShaderObjectPtr const & so) const
	{
		OGLES2ShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLESShaderObject>(so);

		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			typedef BOOST_TYPEOF(vertex_stream_fmt) VertexStreamFmtType;
			BOOST_FOREACH(VertexStreamFmtType::const_reference vs_elem, vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					glDisableVertexAttribArray(attr);
				}
			}
		}
	}
}
