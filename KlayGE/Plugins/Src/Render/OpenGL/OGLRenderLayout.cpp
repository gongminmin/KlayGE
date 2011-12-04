// OGLRenderLayout.cpp
// KlayGE OpenGL渲染分布类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2009-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 完善VAO支持 (2010.8.9)
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

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>

namespace KlayGE
{
	OGLRenderLayout::OGLRenderLayout()
	{
		if (glloader_GL_VERSION_3_0() || glloader_GL_ARB_vertex_array_object())
		{
			use_vao_ = true;
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
		if (use_vao_)
		{
			typedef BOOST_TYPEOF(vaos_) VAOsType;
			BOOST_FOREACH(VAOsType::reference vao, vaos_)
			{
				glDeleteVertexArrays(1, &vao.second);
			}
		}
	}

	void OGLRenderLayout::BindVertexStreams(ShaderObjectPtr const & so) const
	{
		OGLShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLShaderObject>(so);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t max_vertex_streams = re.DeviceCaps().max_vertex_streams;

		std::vector<char> used_streams(max_vertex_streams, 0);
		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetVertexStream(i)));
			uint32_t const size = this->VertexSize(i);
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			uint8_t* elem_offset = NULL;
			typedef BOOST_TYPEOF(vertex_stream_fmt) VertexStreamFmtType;
			BOOST_FOREACH(VertexStreamFmtType::const_reference vs_elem, vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLvoid* offset = static_cast<GLvoid*>(elem_offset + this->BaseVertexLocation() * size);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;

					stream.Active();
					glVertexAttribPointer(attr, num_components, type, normalized, size, offset);
					glEnableVertexAttribArray(attr);

					used_streams[attr] = 1;
				}

				elem_offset += vs_elem.element_size();
			}
		}

		for (GLuint i = 0; i < max_vertex_streams; ++ i)
		{
			if (!used_streams[i])
			{
				glDisableVertexAttribArray(i);
			}
		}

		if (this->UseIndices())
		{
			OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetIndexStream()));
			stream.Active();
		}
	}

	void OGLRenderLayout::UnbindVertexStreams(ShaderObjectPtr const & so) const
	{
		OGLShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLShaderObject>(so);
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

	void OGLRenderLayout::Active(ShaderObjectPtr const & so) const
	{
		if (use_vao_)
		{
			GLuint vao;
			typedef BOOST_TYPEOF(vaos_) VAOsType;
			VAOsType::iterator iter = vaos_.find(so);
			if (iter == vaos_.end())
			{
				glGenVertexArrays(1, &vao);
				vaos_.insert(std::make_pair(so, vao));

				glBindVertexArray(vao);
				this->BindVertexStreams(so);
			}
			else
			{
				vao = iter->second;
				glBindVertexArray(vao);
			}
		}
		else
		{
			this->BindVertexStreams(so);
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
	}

	void OGLRenderLayout::Deactive(ShaderObjectPtr const & so) const
	{
		if (!use_vao_)
		{
			this->UnbindVertexStreams(so);
		}
	}
}
