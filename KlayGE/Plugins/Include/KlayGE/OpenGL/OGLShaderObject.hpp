// OGLShaderObject.hpp
// KlayGE OpenGL shader对象类 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// Cg载入后编译成GLSL使用 (2009.4.26)
//
// 3.7.0
// 改为直接传入RenderEffect (2008.7.4)
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLSHADEROBJECT_HPP
#define _OGLSHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/MapVector.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/ShaderObject.hpp>

#include <boost/function.hpp>

namespace KlayGE
{
	class OGLShaderObject : public ShaderObject
	{
	public:
		OGLShaderObject();
		~OGLShaderObject();

		std::string GenShaderText(RenderEffect const & effect);

		void SetShader(RenderEffect& effect, boost::shared_ptr<std::vector<uint32_t> > const & shader_desc_ids,
			uint32_t tech_index, uint32_t pass_index);
		ShaderObjectPtr Clone(RenderEffect& effect);

		void Bind();
		void Unbind();

		GLint GetAttribLocation(VertexElementUsage usage, uint8_t usage_index);

		GLuint GLSLProgram() const
		{
			return glsl_program_;
		}

	private:
		struct parameter_bind_t
		{
			std::string combined_sampler_name;
			RenderEffectParameterPtr param;
			int location;
			int shader_type;
			int tex_sampler_bind_index;
			boost::function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		parameter_bind_t GetBindFunc(GLint location, RenderEffectParameterPtr const & param);

	private:
		boost::shared_ptr<std::vector<uint32_t> > shader_desc_ids_;
		boost::shared_ptr<std::string> shader_text_;

		GLuint glsl_program_;
		boost::shared_ptr<std::vector<GLint> > glsl_bin_formats_;
		boost::shared_ptr<std::vector<uint8_t> > glsl_bin_program_;
		boost::shared_ptr<std::vector<std::string> > glsl_srcs_;

		parameter_binds_t param_binds_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> > samplers_;

		std::vector<std::pair<std::string, std::pair<RenderEffectParameterPtr, RenderEffectParameterPtr> > > tex_sampler_binds_;

		std::map<std::pair<VertexElementUsage, uint8_t>, GLint> attrib_locs_;
	};

	typedef boost::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// _OGLSHADEROBJECT_HPP
