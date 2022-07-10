/**
 * @file OGLShaderObject.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef KLAYGE_PLUGINS_OGL_SHADER_OBJECT_HPP
#define KLAYGE_PLUGINS_OGL_SHADER_OBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/ShaderObject.hpp>

namespace DXBC2GLSL
{
	class DXBC2GLSL;
}

namespace KlayGE
{
	struct TextureBind
	{
		ShaderResourceViewPtr buff_srv;

		ShaderResourceViewPtr tex_srv;
		SamplerStateObjectPtr sampler;
	};

	class OGLShaderStageObject : public ShaderStageObject
	{
	public:
		explicit OGLShaderStageObject(ShaderStage stage);
		~OGLShaderStageObject() override;
		
		void StreamIn(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res) override;
		void StreamOut(std::ostream& os) override;
		void CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
		void CreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
		
		std::string const& GlslSource() const noexcept
		{
			return glsl_src_;
		}

		std::string const& ShaderFuncName() const noexcept
		{
			return shader_func_name_;
		}

		std::vector<std::string> const& PNames() const noexcept
		{
			return pnames_;
		}

		std::vector<std::string> const& GlslResNames() const noexcept
		{
			return glsl_res_names_;
		}

		std::vector<std::pair<std::string, std::string>> const& TexSamplerPairs() const noexcept
		{
			return tex_sampler_pairs_;
		}

		GLuint GlShader() const noexcept
		{
			return gl_shader_;
		}

		virtual std::span<std::string const> GlslTfbVaryings() const
		{
			return std::span<std::string const>();
		}
		virtual bool TfbSeparateAttribs() const noexcept
		{
			return false;
		}

		virtual uint32_t DsPartitioning() const noexcept
		{
			return 0;
		}
		virtual uint32_t DsOutputPrimitive() const noexcept
		{
			return 0;
		}

	protected:
		void RetrieveTfbVaryings(ShaderDesc const & sd, std::vector<std::string>& tfb_varyings, bool& tfb_separate_attribs);

	private:
		std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;

		virtual void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl)
		{
			KFL_UNUSED(dxbc2glsl);
		}
		void StageSpecificCreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override
		{
			KFL_UNUSED(effect);
			KFL_UNUSED(shader_desc_ids);
		}

	protected:
		bool is_available_;

		std::string shader_func_name_;
		std::string glsl_src_;
		std::vector<std::string> pnames_;
		std::vector<std::string> glsl_res_names_;

		std::vector<std::pair<std::string, std::string>> tex_sampler_pairs_;

		GLuint gl_shader_ = 0;
	};

	class OGLVertexShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLVertexShaderStageObject();

		std::vector<VertexElementUsage> const& Usages() const noexcept
		{
			return usages_;
		}
		std::vector<uint8_t> const& UsageIndices() const noexcept
		{
			return usage_indices_;
		}
		std::vector<std::string> const& GlslAttribNames() const noexcept
		{
			return glsl_attrib_names_;
		}

		std::span<std::string const> GlslTfbVaryings() const override
		{
			return MakeSpan(glsl_tfb_varyings_);
		}
		bool TfbSeparateAttribs() const noexcept override
		{
			return tfb_separate_attribs_;
		}

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
		void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl) override;
		void StageSpecificCreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		std::vector<VertexElementUsage> usages_;
		std::vector<uint8_t> usage_indices_;
		std::vector<std::string> glsl_attrib_names_;

		std::vector<std::string> glsl_tfb_varyings_;
		bool tfb_separate_attribs_;
	};

	class OGLPixelShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLPixelShaderStageObject();
	};

	class OGLGeometryShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLGeometryShaderStageObject();

		std::span<std::string const> GlslTfbVaryings() const override
		{
			return MakeSpan(glsl_tfb_varyings_);
		}
		bool TfbSeparateAttribs() const noexcept override
		{
			return tfb_separate_attribs_;
		}

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
		void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl) override;
		void StageSpecificCreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		GLint gs_input_type_ = 0;
		GLint gs_output_type_ = 0;
		GLint gs_max_output_vertex_ = 0;

		std::vector<std::string> glsl_tfb_varyings_;
		bool tfb_separate_attribs_;
	};

	class OGLComputeShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLComputeShaderStageObject();
	};

	class OGLHullShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLHullShaderStageObject();

		uint32_t DsPartitioning() const noexcept override
		{
			return ds_partitioning_;
		}
		uint32_t DsOutputPrimitive() const noexcept override
		{
			return ds_output_primitive_;
		}

	private:
		void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl) override;

	private:
		uint32_t ds_partitioning_ = 0;
		uint32_t ds_output_primitive_ = 0;
	};

	class OGLDomainShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLDomainShaderStageObject();

		std::span<std::string const> GlslTfbVaryings() const override
		{
			return MakeSpan(glsl_tfb_varyings_);
		}
		bool TfbSeparateAttribs() const noexcept override
		{
			return tfb_separate_attribs_;
		}

		void DsParameters(uint32_t partitioning, uint32_t output_primitive);

		uint32_t DsPartitioning() const noexcept override
		{
			return ds_partitioning_;
		}
		uint32_t DsOutputPrimitive() const noexcept override
		{
			return ds_output_primitive_;
		}

	private:
		void StageSpecificCreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		uint32_t ds_partitioning_ = 0;
		uint32_t ds_output_primitive_ = 0;

		std::vector<std::string> glsl_tfb_varyings_;
		bool tfb_separate_attribs_;
	};

	class OGLShaderObject final : public ShaderObject
	{
	public:
		OGLShaderObject();
		~OGLShaderObject() override;

		ShaderObjectPtr Clone(RenderEffect& dst_effect) override;

		void Bind(RenderEffect const& effect) override;
		void Unbind() override;

		GLint GetAttribLocation(VertexElementUsage usage, uint8_t usage_index);

		GLuint GLSLProgram() const noexcept
		{
			return glsl_program_;
		}

	private:
		struct OGLImmutable
		{
			GLenum glsl_bin_format_;
			std::vector<uint8_t> glsl_bin_program_;
		};

		struct ParameterBind
		{
			std::string combined_sampler_name;
			RenderEffectParameter const* param;
			int location;
			int tex_sampler_bind_index;
			std::function<void()> func;
		};

	public:
		OGLShaderObject(std::shared_ptr<Immutable> immutable, std::shared_ptr<OGLImmutable> gl_immutable) noexcept;

	private:
		void DoLinkShaders(RenderEffect& effect) override;

		void LinkGLSL();
		void AttachUBOs(RenderEffect& effect);

	private:
		const std::shared_ptr<OGLImmutable> gl_immutable_;

		GLuint glsl_program_;

		std::vector<ParameterBind> param_binds_;

		std::vector<TextureBind> textures_;
		std::vector<GLuint> gl_bind_targets_;
		std::vector<GLuint> gl_bind_textures_;
		std::vector<GLuint> gl_bind_samplers_;

		std::vector<std::tuple<std::string, RenderEffectParameter*, RenderEffectParameter*, uint32_t>> tex_sampler_binds_;

		std::map<std::pair<VertexElementUsage, uint8_t>, GLint> attrib_locs_;

		std::vector<uint32_t> all_cbuff_indices_;
	};

	typedef std::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// KLAYGE_PLUGINS_OGL_SHADER_OBJECT_HPP
