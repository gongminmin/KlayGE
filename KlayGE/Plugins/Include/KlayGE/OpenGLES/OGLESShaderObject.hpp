// OGLESShaderObject.hpp
// KlayGE OpenGL ES shader对象类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESSHADEROBJECT_HPP
#define _OGLESSHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class OGLESShaderObject : public ShaderObject
	{
	public:
		OGLESShaderObject();
		~OGLESShaderObject();

		std::string GenShaderText(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass);

		bool AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<uint8_t> const & native_shader_block);
		void ExtractNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint8_t>& native_shader_block);

		void AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids);
		void AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so);
		void LinkShaders(RenderEffect const & effect);
		ShaderObjectPtr Clone(RenderEffect const & effect);

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
			function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		std::string ConvertToESSL(std::string const & glsl, ShaderType type);
		parameter_bind_t GetBindFunc(GLint location, RenderEffectParameterPtr const & param);
		void CompileToNative(std::string const & shader_text, int type, std::string const & func_name);
		void AttachGLSL(uint32_t type);
		void LinkGLSL();

	private:
		GLuint glsl_program_;
		shared_ptr<array<std::string, ST_NumShaderTypes> > shader_func_names_;
		shared_ptr<array<shared_ptr<std::string>, ST_NumShaderTypes> > glsl_srcs_;
		shared_ptr<array<shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> > pnames_;
		shared_ptr<array<shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> > glsl_res_names_;
		shared_ptr<std::vector<VertexElementUsage> > vs_usages_;
		shared_ptr<std::vector<uint8_t> > vs_usage_indices_;
		shared_ptr<std::vector<std::string> > glsl_vs_attrib_names_;

		parameter_binds_t param_binds_;

		std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> > samplers_;

		std::vector<tuple<std::string, RenderEffectParameterPtr, RenderEffectParameterPtr, uint32_t> > tex_sampler_binds_;

		std::map<std::pair<VertexElementUsage, uint8_t>, GLint> attrib_locs_;
	};

	typedef shared_ptr<OGLESShaderObject> OGLESShaderObjectPtr;
}

#endif			// _OGLESSHADEROBJECT_HPP
