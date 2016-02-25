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
#include <KFL/COMPtr.hpp>
#include <KFL/ThrowErr.hpp>
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
			for (auto const & vao : vaos_)
			{
				glDeleteVertexArrays(1, &vao.second);
			}
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
			OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetVertexStream(i)));
			uint32_t const size = this->VertexSize(i);
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
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
					stream.Active(use_vao_);
					if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
					{
						glVertexArrayAttribFormat(vao, attr, num_components, type, normalized, elem_offset);
						glVertexArrayAttribBinding(vao, attr, i);
						glEnableVertexArrayAttrib(vao, attr);
					}
					else if (use_vao_ && glloader_GL_EXT_direct_state_access())
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

		if (this->InstanceStream() && (glloader_GL_VERSION_3_3() || glloader_GL_ARB_instanced_arrays()))
		{
			OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->InstanceStream()));

			uint32_t const instance_size = this->InstanceSize();
			BOOST_ASSERT(this->NumInstances() * instance_size <= stream.Size());

			if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
			{
				glVertexArrayVertexBuffer(vao, this->NumVertexStreams(), stream.GLvbo(),
					this->StartInstanceLocation() * instance_size, instance_size);
				glVertexArrayBindingDivisor(vao, this->NumVertexStreams(), 1);
			}

			size_t const inst_format_size = this->InstanceStreamFormat().size();
			uint32_t elem_offset = 0;
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = this->InstanceStreamFormat()[i];

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
					stream.Active(use_vao_);
					if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
					{
						glVertexArrayAttribFormat(vao, attr, num_components, type, normalized, elem_offset);
						glVertexArrayAttribBinding(vao, attr, this->NumVertexStreams());
						glEnableVertexArrayAttrib(vao, attr);
					}
					else if (use_vao_ && glloader_GL_EXT_direct_state_access())
					{
						glVertexArrayVertexAttribOffsetEXT(vao, stream.GLvbo(), attr, num_components, type,
							normalized, instance_size, offset);
						glEnableVertexArrayAttribEXT(vao, attr);

						if (glloader_GL_VERSION_3_3())
						{
							glVertexAttribDivisor(attr, 1);
						}
						else
						{
							glVertexAttribDivisorARB(attr, 1);
						}
					}
					else
					{
						glVertexAttribPointer(attr, num_components, type, normalized, instance_size,
							reinterpret_cast<GLvoid*>(offset));
						glEnableVertexAttribArray(attr);

						if (glloader_GL_VERSION_3_3())
						{
							glVertexAttribDivisor(attr, 1);
						}
						else
						{
							glVertexAttribDivisorARB(attr, 1);
						}
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
				if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
				{
					glDisableVertexArrayAttrib(vao, i);
				}
				else if (use_vao_ && glloader_GL_EXT_direct_state_access())
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
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			for (auto const & vs_elem : vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
					{
						glDisableVertexArrayAttrib(vao, attr);
					}
					else if (use_vao_ && glloader_GL_EXT_direct_state_access())
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

		if (this->InstanceStream() && (glloader_GL_VERSION_3_3() || glloader_GL_ARB_instanced_arrays()))
		{
			if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
			{
				glVertexArrayBindingDivisor(vao, this->NumVertexStreams(), 0);
			}

			size_t const inst_format_size = this->InstanceStreamFormat().size();
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = this->InstanceStreamFormat()[i];
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					if (use_vao_ && (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access()))
					{
						glDisableVertexArrayAttrib(vao, attr);
					}
					else if (use_vao_ && glloader_GL_EXT_direct_state_access())
					{
						glDisableVertexArrayAttribEXT(vao, attr);

						if (glloader_GL_VERSION_3_3())
						{
							glVertexAttribDivisor(attr, 0);
						}
						else
						{
							glVertexAttribDivisorARB(attr, 0);
						}
					}
					else
					{
						glDisableVertexAttribArray(attr);

						if (glloader_GL_VERSION_3_3())
						{
							glVertexAttribDivisor(attr, 0);
						}
						else
						{
							glVertexAttribDivisorARB(attr, 0);
						}
					}
				}
			}
		}
	}

	void OGLRenderLayout::Active(ShaderObjectPtr const & so) const
	{
		if (use_vao_)
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

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (this->NumVertexStreams() > 0)
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetVertexStream(this->NumVertexStreams() - 1)));
				re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
			}
			if (this->InstanceStream() && (glloader_GL_VERSION_3_3() || glloader_GL_ARB_instanced_arrays()))
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->InstanceStream()));
				re.OverrideBindBufferCache(stream.GLType(), stream.GLvbo());
			}

			if (this->UseIndices())
			{
				OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(this->GetIndexStream()));
				BOOST_ASSERT(GL_ELEMENT_ARRAY_BUFFER == stream.GLType());
				stream.Active(use_vao_);
			}
			else
			{
				re.OverrideBindBufferCache(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
		}
		else
		{
			this->BindVertexStreams(so, 0);
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
			this->UnbindVertexStreams(so, 0);
		}
	}
}
