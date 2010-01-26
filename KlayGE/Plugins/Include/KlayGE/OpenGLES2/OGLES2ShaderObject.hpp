// OGLES2ShaderObject.hpp
// KlayGE OpenGL ES 2 shader对象类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2SHADEROBJECT_HPP
#define _OGLES2SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/MapVector.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/ShaderObject.hpp>

#include <boost/function.hpp>

namespace KlayGE
{
	class OGLES2ShaderObject : public ShaderObject
	{
	public:
		OGLES2ShaderObject();
		~OGLES2ShaderObject();

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

		std::string OGLES2ShaderObject::ConvertTOELSL(std::string const & glsl, ShaderType type);
		parameter_bind_t GetBindFunc(GLint location, RenderEffectParameterPtr const & param);

	private:
		boost::shared_ptr<std::vector<uint32_t> > shader_desc_ids_;
		boost::shared_ptr<std::string> shader_text_;

		GLuint glsl_program_;
		boost::shared_ptr<std::vector<std::string> > glsl_srcs_;

		parameter_binds_t param_binds_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		boost::array<std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >, ST_NumShaderTypes> samplers_;

		std::vector<std::pair<std::string, std::pair<RenderEffectParameterPtr, RenderEffectParameterPtr> > > tex_sampler_binds_;

		std::map<std::pair<VertexElementUsage, uint8_t>, GLint> attrib_locs_;
	};

	typedef boost::shared_ptr<OGLES2ShaderObject> OGLES2ShaderObjectPtr;
}

#endif			// _OGLES2SHADEROBJECT_HPP
