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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>

namespace KlayGE
{
	OGLRenderLayout::OGLRenderLayout()
	{
	}

	OGLRenderLayout::~OGLRenderLayout()
	{
		for (auto const & vao : vaos_)
		{
			glDeleteVertexArrays(1, &vao.second);
		}
	}

	void OGLRenderLayout::BindVertexStreams(ShaderObjectPtr const & so, GLuint vao) const
	{
		OGLShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLShaderObject>(so);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t max_vertex_streams = re.DeviceCaps().max_vertex_streams;

		std::vector<char> used_streams(max_vertex_streams, 0);
		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			auto& stream = checked_cast<OGLGraphicsBuffer&>(*this->GetVertexStream(i));
			uint32_t const size = this->VertexSize(i);
			auto const & vertex_stream_fmt = this->VertexStreamFormat(i);

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glVertexArrayVertexBuffer(vao, i, stream.GLvbo(), this->StartVertexLocation() * size, size);
			}

			uint32_t elem_offset = 0;
			for (auto const & vs_elem : vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLintptr offset = elem_offset + this->StartVertexLocation() * size;
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;

					BOOST_ASSERT(GL_ARRAY_BUFFER == stream.GLType());
					stream.Active(true);
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						glVertexArrayAttribFormat(vao, attr, num_components, type, normalized, elem_offset);
						glVertexArrayAttribBinding(vao, attr, i);
						glEnableVertexArrayAttrib(vao, attr);
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						glVertexArrayVertexAttribOffsetEXT(vao, stream.GLvbo(), attr, num_components, type,
							normalized, size, offset);
						glEnableVertexArrayAttribEXT(vao, attr);
					}
					else
					{
						glVertexAttribPointer(attr, num_components, type, normalized, size, reinterpret_cast<GLvoid*>(offset));
						glEnableVertexAttribArray(attr);
					}

					used_streams[attr] = 1;
				}

				elem_offset += vs_elem.element_size();
			}
		}

		if (this->InstanceStream())
		{
			auto& stream = checked_cast<OGLGraphicsBuffer&>(*this->InstanceStream());

			uint32_t const instance_size = this->InstanceSize();
			BOOST_ASSERT(this->NumInstances() * instance_size <= stream.Size());

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glVertexArrayVertexBuffer(vao, this->NumVertexStreams(), stream.GLvbo(),
					this->StartInstanceLocation() * instance_size, instance_size);
				glVertexArrayBindingDivisor(vao, this->NumVertexStreams(), 1);
			}

			size_t const inst_format_size = this->InstanceStreamFormat().size();
			uint32_t elem_offset = 0;
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				VertexElement const & vs_elem = this->InstanceStreamFormat()[i];

				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;
					GLintptr offset = elem_offset + this->StartInstanceLocation() * instance_size;

					BOOST_ASSERT(GL_ARRAY_BUFFER == stream.GLType());
					stream.Active(true);
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						glVertexArrayAttribFormat(vao, attr, num_components, type, normalized, elem_offset);
						glVertexArrayAttribBinding(vao, attr, this->NumVertexStreams());
						glEnableVertexArrayAttrib(vao, attr);
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						glVertexArrayVertexAttribOffsetEXT(vao, stream.GLvbo(), attr, num_components, type,
							normalized, instance_size, offset);
						glEnableVertexArrayAttribEXT(vao, attr);
						glVertexAttribDivisor(attr, 1);
					}
					else
					{
						glVertexAttribPointer(attr, num_components, type, normalized, instance_size,
							reinterpret_cast<GLvoid*>(offset));
						glEnableVertexAttribArray(attr);
						glVertexAttribDivisor(attr, 1);
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
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glDisableVertexArrayAttrib(vao, i);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glDisableVertexArrayAttribEXT(vao, i);
				}
				else
				{
					glDisableVertexAttribArray(i);
				}
			}
		}
	}

	void OGLRenderLayout::UnbindVertexStreams(ShaderObjectPtr const & so, GLuint vao) const
	{
		OGLShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLShaderObject>(so);
		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			auto const & vertex_stream_fmt = this->VertexStreamFormat(i);

			for (auto const & vs_elem : vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						glDisableVertexArrayAttrib(vao, attr);
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						glDisableVertexArrayAttribEXT(vao, attr);
					}
					else
					{
						glDisableVertexAttribArray(attr);
					}
				}
			}
		}

		if (this->InstanceStream())
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glVertexArrayBindingDivisor(vao, this->NumVertexStreams(), 0);
			}

			size_t const inst_format_size = this->InstanceStreamFormat().size();
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				VertexElement const & vs_elem = this->InstanceStreamFormat()[i];
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						glDisableVertexArrayAttrib(vao, attr);
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						glDisableVertexArrayAttribEXT(vao, attr);
						glVertexAttribDivisor(attr, 0);
					}
					else
					{
						glDisableVertexAttribArray(attr);
						glVertexAttribDivisor(attr, 0);
					}
				}
			}
		}
	}

	void OGLRenderLayout::Active(ShaderObjectPtr const & so) const
	{
		GLuint vao;
		auto iter = vaos_.find(so);
		if (iter == vaos_.end())
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glCreateVertexArrays(1, &vao);
			}
			else
			{
				glGenVertexArrays(1, &vao);
			}
			glBindVertexArray(vao);

			vaos_.emplace(so, vao);
			this->BindVertexStreams(so, vao);
		}
		else
		{
			vao = iter->second;
			glBindVertexArray(vao);
			if (streams_dirty_)
			{
				this->BindVertexStreams(so, vao);
				streams_dirty_ = false;
			}
		}

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (this->NumVertexStreams() > 0)
		{
			auto& stream = checked_cast<OGLGraphicsBuffer&>(*this->GetVertexStream(this->NumVertexStreams() - 1));
			re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
		}
		if (this->InstanceStream())
		{
			auto& stream = checked_cast<OGLGraphicsBuffer&>(*this->InstanceStream());
			re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
		}

		if (this->UseIndices())
		{
			auto& stream = checked_cast<OGLGraphicsBuffer&>(*this->GetIndexStream());
			BOOST_ASSERT(GL_ELEMENT_ARRAY_BUFFER == stream.GLType());
			stream.Active(true);
		}
		else
		{
			re.OverrideBindBufferCache(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
}
