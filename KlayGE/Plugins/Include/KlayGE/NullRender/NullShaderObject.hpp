/**
 * @file NullShaderObject.hpp
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

#ifndef KLAYGE_PLUGINS_NULL_SHADER_OBJECT_HPP
#define KLAYGE_PLUGINS_NULL_SHADER_OBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>

struct ID3D11ShaderReflection;
typedef unsigned int GLenum;
typedef int GLint;

namespace DXBC2GLSL
{
	class DXBC2GLSL;
}

namespace KlayGE
{
	struct D3DShaderDesc
	{
		struct ConstantBufferDesc
		{
			struct VariableDesc
			{
				std::string name;
				uint32_t start_offset;
				uint8_t type;
				uint8_t rows;
				uint8_t columns;
				uint16_t elements;
			};
			std::vector<VariableDesc> var_desc;

			std::string name;
			size_t name_hash;
			uint32_t size = 0;
		};
		std::vector<ConstantBufferDesc> cb_desc;

		uint16_t num_samplers = 0;
		uint16_t num_srvs = 0;
		uint16_t num_uavs = 0;

		struct BoundResourceDesc
		{
			std::string name;
			uint8_t type;
			uint8_t dimension;
			uint16_t bind_point;
		};
		std::vector<BoundResourceDesc> res_desc;
	};

	class D3DShaderStageObject : public ShaderStageObject
	{
	public:
		D3DShaderStageObject(ShaderStage stage, bool as_d3d12);

		void StreamIn(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res) override;
		void StreamOut(std::ostream& os) override;
		void CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
		void CreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

		std::vector<uint8_t> const& ShaderCodeBlob() const
		{
			return shader_code_;
		}

		std::string const& ShaderProfile() const
		{
			return shader_profile_;
		}

		D3DShaderDesc const& GetD3DShaderDesc() const
		{
			return shader_desc_;
		}

		std::vector<uint8_t> const& CBufferIndices() const
		{
			return cbuff_indices_;
		}

	private:
		std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;
		void FillCBufferIndices(RenderEffect const& effect);

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		virtual void StageSpecificReflection(ID3D11ShaderReflection* reflection)
		{
			KFL_UNUSED(reflection);
		}
#endif

	protected:
		const bool as_d3d12_;

		bool is_available_;

		std::vector<uint8_t> shader_code_;
		std::string shader_profile_;
		D3DShaderDesc shader_desc_;
		std::vector<uint8_t> cbuff_indices_;
	};

	class D3D11VertexShaderStageObject final : public D3DShaderStageObject
	{
	public:
		D3D11VertexShaderStageObject();

		uint32_t VsSignature() const
		{
			return vs_signature_;
		}

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void StageSpecificReflection(ID3D11ShaderReflection* reflection) override;
#endif

	private:
		uint32_t vs_signature_;
	};

	class D3D12VertexShaderStageObject final : public D3DShaderStageObject
	{
	public:
		D3D12VertexShaderStageObject();
	};

	class D3DPixelShaderStageObject final : public D3DShaderStageObject
	{
	public:
		explicit D3DPixelShaderStageObject(bool as_d3d12);
	};

	class D3DGeometryShaderStageObject final : public D3DShaderStageObject
	{
	public:
		explicit D3DGeometryShaderStageObject(bool as_d3d12);
	};

	class D3DComputeShaderStageObject final : public D3DShaderStageObject
	{
	public:
		explicit D3DComputeShaderStageObject(bool as_d3d12);

		uint32_t BlockSizeX() const override
		{
			return block_size_x_;
		}
		uint32_t BlockSizeY() const override
		{
			return block_size_y_;
		}
		uint32_t BlockSizeZ() const override
		{
			return block_size_z_;
		}

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void StageSpecificReflection(ID3D11ShaderReflection* reflection) override;
#endif

	private:
		uint32_t block_size_x_, block_size_y_, block_size_z_;
	};

	class D3DHullShaderStageObject final : public D3DShaderStageObject
	{
	public:
		explicit D3DHullShaderStageObject(bool as_d3d12);
	};

	class D3DDomainShaderStageObject final : public D3DShaderStageObject
	{
	public:
		explicit D3DDomainShaderStageObject(bool as_d3d12);
	};


	class OGLShaderStageObject : public ShaderStageObject
	{
	public:
		OGLShaderStageObject(ShaderStage stage, GLenum gl_shader_type, bool as_gles);

		void StreamIn(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res) override;
		void StreamOut(std::ostream& os) override;
		void CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
		void CreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

		std::string const& GlslSource() const
		{
			return glsl_src_;
		}

		std::string const& ShaderFuncName() const
		{
			return shader_func_name_;
		}

		std::vector<std::string> const& PNames() const
		{
			return pnames_;
		}

		std::vector<std::string> const& GlslResNames() const
		{
			return glsl_res_names_;
		}

		std::vector<std::pair<std::string, std::string>> const& TexSamplerPairs() const
		{
			return tex_sampler_pairs_;
		}

		virtual uint32_t DsPartitioning() const
		{
			return 0;
		}
		virtual uint32_t DsOutputPrimitive() const
		{
			return 0;
		}

	private:
		std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;

		virtual void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl)
		{
			KFL_UNUSED(dxbc2glsl);
		}

	protected:
		const GLenum gl_shader_type_;
		const bool as_gles_;

		bool is_available_;

		std::string shader_func_name_;
		std::string glsl_src_;
		std::vector<std::string> pnames_;
		std::vector<std::string> glsl_res_names_;

		std::vector<std::pair<std::string, std::string>> tex_sampler_pairs_;
	};

	class OGLVertexShaderStageObject final : public OGLShaderStageObject
	{
	public:
		explicit OGLVertexShaderStageObject(bool as_gles);

		std::vector<VertexElementUsage> const& Usages() const
		{
			return usages_;
		}
		std::vector<uint8_t> const& UsageIndices() const
		{
			return usage_indices_;
		}
		std::vector<std::string> const& GlslAttribNames() const
		{
			return glsl_attrib_names_;
		}

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
		void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl) override;

	private:
		std::vector<VertexElementUsage> usages_;
		std::vector<uint8_t> usage_indices_;
		std::vector<std::string> glsl_attrib_names_;
	};

	class OGLPixelShaderStageObject final : public OGLShaderStageObject
	{
	public:
		explicit OGLPixelShaderStageObject(bool as_gles);

		bool HasDiscard() const override
		{
			return has_discard_;
		}

	private:
		bool has_discard_ = true;
	};

	class OGLGeometryShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLGeometryShaderStageObject();

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
		void StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl) override;

	private:
		GLint gs_input_type_ = 0;
		GLint gs_output_type_ = 0;
		GLint gs_max_output_vertex_ = 0;
	};

	class OGLESGeometryShaderStageObject final : public OGLShaderStageObject
	{
	public:
		OGLESGeometryShaderStageObject();
	};

	class OGLComputeShaderStageObject final : public OGLShaderStageObject
	{
	public:
		explicit OGLComputeShaderStageObject(bool as_gles);
	};

	class OGLHullShaderStageObject final : public OGLShaderStageObject
	{
	public:
		explicit OGLHullShaderStageObject(bool as_gles);

		uint32_t DsPartitioning() const override
		{
			return ds_partitioning_;
		}
		uint32_t DsOutputPrimitive() const override
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
		explicit OGLDomainShaderStageObject(bool as_gles);

		void DsParameters(uint32_t partitioning, uint32_t output_primitive);

		uint32_t DsPartitioning() const override
		{
			return ds_partitioning_;
		}
		uint32_t DsOutputPrimitive() const override
		{
			return ds_output_primitive_;
		}

	private:
		uint32_t ds_partitioning_ = 0;
		uint32_t ds_output_primitive_ = 0;
	};


	class NullShaderObject final : public ShaderObject
	{
	public:
		NullShaderObject();

		ShaderObjectPtr Clone(RenderEffect const & effect) override;

		void Bind(RenderEffect const& effect) override;
		void Unbind() override;

	private:
		struct NullShaderObjectTemplate
		{
			bool as_d3d11_ = false;
			bool as_d3d12_ = false;
			bool as_gl_ = false;
			bool as_gles_ = false;
		};

	public:
		NullShaderObject(
			std::shared_ptr<ShaderObjectTemplate> so_template, std::shared_ptr<NullShaderObjectTemplate> null_so_template);

	private:
		void CreateHwResources(ShaderStage stage, RenderEffect const & effect) override;
		void DoLinkShaders(RenderEffect const & effect) override;

		void OGLAppendTexSamplerBinds(
			ShaderStage stage, RenderEffect const& effect, std::vector<std::pair<std::string, std::string>> const& tex_sampler_pairs);

	private:
		const std::shared_ptr<NullShaderObjectTemplate> null_so_template_;

		std::vector<std::tuple<std::string, RenderEffectParameter*, RenderEffectParameter*, uint32_t>> gl_tex_sampler_binds_;
	};
}

#endif			// KLAYGE_PLUGINS_NULL_SHADER_OBJECT_HPP
