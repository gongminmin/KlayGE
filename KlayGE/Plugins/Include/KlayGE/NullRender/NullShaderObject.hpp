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

namespace KlayGE
{
	class NullShaderObject : public ShaderObject
	{
	public:
		NullShaderObject();

		bool AttachNativeShader(ShaderType type, RenderEffect const & effect,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::vector<uint8_t> const & native_shader_block) override;

		bool StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids) override;
		void StreamOut(std::ostream& os, ShaderType type) override;

		void AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids) override;
		void AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so) override;
		void LinkShaders(RenderEffect const & effect) override;
		ShaderObjectPtr Clone(RenderEffect const & effect) override;

		void Bind() override;
		void Unbind() override;

	private:
		struct NullShaderObjectTemplate
		{
			NullShaderObjectTemplate()
				: as_d3d11_(false), as_d3d12_(false), as_gl_(false), as_gles_(false)
			{
			}
			virtual ~NullShaderObjectTemplate()
			{
			}

			bool as_d3d11_;
			bool as_d3d12_;
			bool as_gl_;
			bool as_gles_;
		};

		struct D3D11ShaderObjectTemplate : public NullShaderObjectTemplate
		{
			~D3D11ShaderObjectTemplate() override
			{
			}

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 2)
#endif
			struct D3D11ShaderDesc
			{
				D3D11ShaderDesc()
					: num_samplers(0), num_srvs(0), num_uavs(0)
				{
				}

				struct ConstantBufferDesc
				{
					ConstantBufferDesc()
						: size(0)
					{
					}

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
					uint32_t size;
				};
				std::vector<ConstantBufferDesc> cb_desc;

				uint16_t num_samplers;
				uint16_t num_srvs;
				uint16_t num_uavs;

				struct BoundResourceDesc
				{
					std::string name;
					uint8_t type;
					uint8_t dimension;
					uint16_t bind_point;
				};
				std::vector<BoundResourceDesc> res_desc;
			};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

			std::array<std::pair<std::shared_ptr<std::vector<uint8_t>>, std::string>, ST_NumShaderTypes> shader_code_;
			std::array<std::shared_ptr<D3D11ShaderDesc>, ST_NumShaderTypes> shader_desc_;
			std::array<std::shared_ptr<std::vector<uint8_t>>, ST_NumShaderTypes> cbuff_indices_;

			uint32_t vs_signature_;
		};

		struct OGLShaderObjectTemplate : public NullShaderObjectTemplate
		{
			OGLShaderObjectTemplate();
			~OGLShaderObjectTemplate() override
			{
			}

			std::array<std::string, ST_NumShaderTypes> shader_func_names_;
			std::array<std::shared_ptr<std::string>, ST_NumShaderTypes> glsl_srcs_;
			std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes> pnames_;
			std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes> glsl_res_names_;
			std::vector<VertexElementUsage> vs_usages_;
			std::vector<uint8_t> vs_usage_indices_;
			std::vector<std::string> glsl_vs_attrib_names_;
			int32_t gs_input_type_, gs_output_type_, gs_max_output_vertex_;	// Only in GL
			uint32_t ds_partitioning_, ds_output_primitive_;
		};

	public:
		explicit NullShaderObject(std::shared_ptr<NullShaderObjectTemplate> const & so_template);

	private:
		void D3D11StreamOut(std::ostream& os, ShaderType type);
		void D3D11AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids);
		void D3D11AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so);
		void D3D11LinkShaders(RenderEffect const & effect);

		void OGLStreamOut(std::ostream& os, ShaderType type);
		void OGLAttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids);
		void OGLAttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so);
		void OGLLinkShaders(RenderEffect const & effect);

		void OGLESAttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so);

		std::shared_ptr<std::vector<uint8_t>> D3D11CompiteToBytecode(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids);
		void D3D11AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob);
	private:
		std::shared_ptr<NullShaderObjectTemplate> so_template_;

		std::vector<std::tuple<std::string, RenderEffectParameter*, RenderEffectParameter*, uint32_t>> gl_tex_sampler_binds_;
	};
}

#endif			// KLAYGE_PLUGINS_NULL_SHADER_OBJECT_HPP
