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
			for (auto const & vao : vaos_)
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
			for (auto const & vs_elem : vertex_stream_fmt)
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

		if (this->InstanceStream() && (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_instanced_arrays()))
		{
			OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->InstanceStream()));

			uint32_t const instance_size = this->InstanceSize();
			BOOST_ASSERT(this->NumInstances() * instance_size <= stream.Size());

			size_t const inst_format_size = this->InstanceStreamFormat().size();
			uint8_t* elem_offset = nullptr;
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = this->InstanceStreamFormat()[i];

				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLESMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;
					GLvoid* offset = static_cast<GLvoid*>(elem_offset + this->StartInstanceLocation() * instance_size);

					BOOST_ASSERT(GL_ARRAY_BUFFER == stream.GLType());
					stream.Active(use_vao_);
					glVertexAttribPointer(attr, num_components, type, normalized, instance_size, offset);
					glEnableVertexAttribArray(attr);

					if (glloader_GLES_VERSION_3_0())
					{
						glVertexAttribDivisor(attr, 1);
					}
					else
					{
						glVertexAttribDivisorEXT(attr, 1);
					}

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
		if (!(ogl_re.HackForMali() || ogl_re.HackForAdreno()) && this->UseIndices())
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

			for (auto const & vs_elem : vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					glDisableVertexAttribArray(attr);
				}
			}
		}

		if (this->InstanceStream() && (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_instanced_arrays()))
		{
			size_t const inst_format_size = this->InstanceStreamFormat().size();
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = this->InstanceStreamFormat()[i];
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					glDisableVertexAttribArray(attr);
					if (glloader_GLES_VERSION_3_0())
					{
						glVertexAttribDivisor(attr, 0);
					}
					else
					{
						glVertexAttribDivisorEXT(attr, 0);
					}
				}
			}
		}
	}

	void OGLESRenderLayout::Active(ShaderObjectPtr const & so) const
	{
		if (use_vao_)
		{
			GLuint vao;
			auto iter = vaos_.find(so);
			if (iter == vaos_.end())
			{
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);

				vaos_.emplace(so, vao);
				this->BindVertexStreams(so);
			}
			else
			{
				vao = iter->second;
				glBindVertexArray(vao);
				if (streams_dirty_)
				{
					this->BindVertexStreams(so);
					streams_dirty_ = false;
				}
			}

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (this->NumVertexStreams() > 0)
			{
				OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetVertexStream(this->NumVertexStreams() - 1)));
				re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
			}
			if (this->InstanceStream() && (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_instanced_arrays()))
			{
				OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->InstanceStream()));
				re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
			}

			if (this->UseIndices())
			{
				if (re.HackForMali() || re.HackForAdreno())
				{
					OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetIndexStream()));
					BOOST_ASSERT(GL_ELEMENT_ARRAY_BUFFER == stream.GLType());
					stream.Active(use_vao_);
				}
				else
				{
					OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(this->GetIndexStream()));
					BOOST_ASSERT(GL_ELEMENT_ARRAY_BUFFER == stream.GLType());
					re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
				}
			}
			else
			{
				re.OverrideBindBufferCache(GL_ELEMENT_ARRAY_BUFFER, 0);
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
