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
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class OGLShaderObject : public ShaderObject
	{
	public:
		OGLShaderObject();
		~OGLShaderObject();

		bool AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<uint8_t> const & native_shader_block);
		
		virtual bool StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
			std::vector<uint32_t> const & shader_desc_ids) override;
		virtual void StreamOut(std::ostream& os, ShaderType type) override;

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
			std::function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		parameter_bind_t GetBindFunc(GLint location, RenderEffectParameterPtr const & param);
		void AttachGLSL(uint32_t type);
		void LinkGLSL();
		void AttachUBOs(RenderEffect const & effect);
		void PrintGLSLError(ShaderType type, char const * info);
		void PrintGLSLErrorAtLine(std::string const & glsl, int err_line);

	private:
		GLuint glsl_program_;
		GLenum glsl_bin_format_;
		std::shared_ptr<std::vector<uint8_t>> glsl_bin_program_;
		std::shared_ptr<std::array<std::string, ST_NumShaderTypes>> shader_func_names_;
		std::shared_ptr<std::array<std::shared_ptr<std::string>, ST_NumShaderTypes>> glsl_srcs_;
		std::shared_ptr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>> pnames_;
		std::shared_ptr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>> glsl_res_names_;
		std::shared_ptr<std::vector<VertexElementUsage>> vs_usages_;
		std::shared_ptr<std::vector<uint8_t>> vs_usage_indices_;
		std::shared_ptr<std::vector<std::string>> glsl_vs_attrib_names_;
		GLint gs_input_type_, gs_output_type_, gs_max_output_vertex_;
		uint32_t ds_partitioning_, ds_output_primitive_;

		parameter_binds_t param_binds_;

		std::vector<std::pair<TexturePtr, SamplerStateObjectPtr>> samplers_;
		std::vector<GLuint> gl_bind_targets_;
		std::vector<GLuint> gl_bind_textures_;
		std::vector<GLuint> gl_bind_cbuffs_;

		std::vector<std::tuple<std::string, RenderEffectParameterPtr, RenderEffectParameterPtr, uint32_t>> tex_sampler_binds_;

		std::map<std::pair<VertexElementUsage, uint8_t>, GLint> attrib_locs_;

		std::vector<RenderEffectConstantBufferPtr> all_cbuffs_;
	};

	typedef std::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// _OGLSHADEROBJECT_HPP
