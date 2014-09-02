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
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>
#include <KlayGE/OpenGLES/OGLESRenderLayout.hpp>

namespace KlayGE
{
	OGLESRenderLayout::OGLESRenderLayout()
	{
		if (glloader_GLES_VERSION_3_0())
		{
			use_vao_ = true;
		}
		else
		{
			use_vao_ = false;
		}
	}

	OGLESRenderLayout::~OGLESRenderLayout()
	{
		if (use_vao_)
		{
			typedef KLAYGE_DECLTYPE(vaos_) VAOsType;
			KLAYGE_FOREACH(VAOsType::reference vao, vaos_)
			{
				glDeleteVertexArrays(1, &vao.second);
			}
		}
	}

	void OGLESRenderLayout::BindVertexStreams(ShaderObjectPtr const & so) const
	{
		OGLESShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLESShaderObject>(so);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t max_vertex_streams = re.DeviceCaps().max_vertex_streams;

		std::vector<char> used_streams(max_vertex_streams, 0);
		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetVertexStream(i)));
			uint32_t const size = this->VertexSize(i);
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			uint8_t* elem_offset = nullptr;
			typedef KLAYGE_DECLTYPE(vertex_stream_fmt) VertexStreamFmtType;
			KLAYGE_FOREACH(VertexStreamFmtType::const_reference vs_elem, vertex_stream_fmt)
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

					BOOST_ASSERT(GL_ARRAY_BUFFER == stream.GLType());
					stream.Active(use_vao_);
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

		OGLESRenderEngine& ogl_re = *checked_cast<OGLESRenderEngine*>(&re);
		if (!(ogl_re.HackForPVR() || ogl_re.HackForMali() || ogl_re.HackForAdreno()) && this->UseIndices())
		{
			OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetIndexStream()));
			BOOST_ASSERT(GL_ELEMENT_ARRAY_BUFFER == stream.GLType());
			stream.Active(use_vao_);
		}
	}

	void OGLESRenderLayout::UnbindVertexStreams(ShaderObjectPtr const & so) const
	{
		OGLESShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLESShaderObject>(so);

		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			typedef KLAYGE_DECLTYPE(vertex_stream_fmt) VertexStreamFmtType;
			KLAYGE_FOREACH(VertexStreamFmtType::const_reference vs_elem, vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					glDisableVertexAttribArray(attr);
				}
			}
		}
	}

	void OGLESRenderLayout::Active(ShaderObjectPtr const & so) const
	{
		if (use_vao_)
		{
			GLuint vao;
			typedef KLAYGE_DECLTYPE(vaos_) VAOsType;
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

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if ((re.HackForPVR() || re.HackForMali() || re.HackForAdreno()) && this->UseIndices())
			{
				OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetIndexStream()));
				BOOST_ASSERT(GL_ELEMENT_ARRAY_BUFFER == stream.GLType());
				stream.Active(use_vao_);
			}
		}
		else
		{
			this->BindVertexStreams(so);
		}
	}

	void OGLESRenderLayout::Deactive(ShaderObjectPtr const & so) const
	{
		if (!use_vao_)
		{
			this->UnbindVertexStreams(so);
		}
	}
}
