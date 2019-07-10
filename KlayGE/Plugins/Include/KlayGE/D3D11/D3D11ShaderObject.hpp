/**
 * @file D3D11ShaderObject.hpp
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

#ifndef KLAYGE_PLUGINS_D3D11_SHADER_OBJECT_HPP
#define KLAYGE_PLUGINS_D3D11_SHADER_OBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KFL/CXX2a/span.hpp>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

#if KLAYGE_IS_DEV_PLATFORM
struct ID3D11ShaderReflection;
#endif

namespace KlayGE
{
	struct D3D11ShaderDesc
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

	class D3D11ShaderStageObject : public ShaderStageObject
	{
	public:
		explicit D3D11ShaderStageObject(ShaderStage stage);

		void StreamIn(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res) override;
		void StreamOut(std::ostream& os) override;
		void CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
		void CreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

		std::vector<uint8_t> const& ShaderCodeBlob() const
		{
			return shader_code_;
		}

		std::string const& ShaderProfile() const
		{
			return shader_profile_;
		}

		D3D11ShaderDesc const& GetD3D11ShaderDesc() const
		{
			return shader_desc_;
		}

		std::vector<uint8_t> const& CBufferIndices() const
		{
			return cbuff_indices_;
		}

		virtual ID3D11VertexShader* HwVertexShader() const
		{
			return nullptr;
		}
		virtual ID3D11PixelShader* HwPixelShader() const
		{
			return nullptr;
		}
		virtual ID3D11GeometryShader* HwGeometryShader() const
		{
			return nullptr;
		}
		virtual ID3D11ComputeShader* HwComputeShader() const
		{
			return nullptr;
		}
		virtual ID3D11HullShader* HwHullShader() const
		{
			return nullptr;
		}
		virtual ID3D11DomainShader* HwDomainShader() const
		{
			return nullptr;
		}

	protected:
		ID3D11GeometryShaderPtr CreateGeometryShaderWithStreamOutput(RenderEffect const& effect,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids, std::span<uint8_t const> code_blob,
			std::vector<ShaderDesc::StreamOutputDecl> const& so_decl);

	private:
		std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;
		void FillCBufferIndices(RenderEffect const& effect);
		virtual void ClearHwShader() = 0;

#if KLAYGE_IS_DEV_PLATFORM
		virtual void StageSpecificReflection(ID3D11ShaderReflection* reflection)
		{
			KFL_UNUSED(reflection);
		}
#endif

	protected:
		bool is_available_;

		std::vector<uint8_t> shader_code_;
		std::string shader_profile_;
		D3D11ShaderDesc shader_desc_;
		std::vector<uint8_t> cbuff_indices_;
	};

	class D3D11VertexShaderStageObject : public D3D11ShaderStageObject
	{
	public:
		D3D11VertexShaderStageObject();

		ID3D11VertexShader* HwVertexShader() const override
		{
			return vertex_shader_.get();
		}
		ID3D11GeometryShader* HwGeometryShader() const override
		{
			return geometry_shader_.get();
		}

		uint32_t VsSignature() const
		{
			return vs_signature_;
		}

	private:
		void ClearHwShader() override;

		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
#if KLAYGE_IS_DEV_PLATFORM
		void StageSpecificReflection(ID3D11ShaderReflection* reflection) override;
#endif
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		ID3D11VertexShaderPtr vertex_shader_;
		ID3D11GeometryShaderPtr geometry_shader_;

		uint32_t vs_signature_;
	};

	class D3D11PixelShaderStageObject : public D3D11ShaderStageObject
	{
	public:
		D3D11PixelShaderStageObject();

		bool HasDiscard() const override
		{
			return has_discard_;
		}

		ID3D11PixelShader* HwPixelShader() const override
		{
			return pixel_shader_.get();
		}

	private:
		void ClearHwShader() override;
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		ID3D11PixelShaderPtr pixel_shader_;
		bool has_discard_ = true;
	};

	class D3D11GeometryShaderStageObject : public D3D11ShaderStageObject
	{
	public:
		D3D11GeometryShaderStageObject();

		ID3D11GeometryShader* HwGeometryShader() const override
		{
			return geometry_shader_.get();
		}

	private:
		void ClearHwShader() override;
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		ID3D11GeometryShaderPtr geometry_shader_;
	};

	class D3D11ComputeShaderStageObject : public D3D11ShaderStageObject
	{
	public:
		D3D11ComputeShaderStageObject();

		ID3D11ComputeShader* HwComputeShader() const override
		{
			return compute_shader_.get();
		}

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
		void ClearHwShader() override;
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
#if KLAYGE_IS_DEV_PLATFORM
		void StageSpecificReflection(ID3D11ShaderReflection* reflection) override;
#endif

	private:
		ID3D11ComputeShaderPtr compute_shader_;

		uint32_t block_size_x_, block_size_y_, block_size_z_;
	};

	class D3D11HullShaderStageObject : public D3D11ShaderStageObject
	{
	public:
		D3D11HullShaderStageObject();

		ID3D11HullShader* HwHullShader() const override
		{
			return hull_shader_.get();
		}

	private:
		void ClearHwShader() override;
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		ID3D11HullShaderPtr hull_shader_;
	};

	class D3D11DomainShaderStageObject : public D3D11ShaderStageObject
	{
	public:
		D3D11DomainShaderStageObject();

		ID3D11DomainShader* HwDomainShader() const override
		{
			return domain_shader_.get();
		}
		ID3D11GeometryShader* HwGeometryShader() const override
		{
			return geometry_shader_.get();
		}

	private:
		void ClearHwShader() override;
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		ID3D11DomainShaderPtr domain_shader_;
		ID3D11GeometryShaderPtr geometry_shader_;
	};

	class D3D11ShaderObject : public ShaderObject
	{
	public:
		D3D11ShaderObject();

		ShaderObjectPtr Clone(RenderEffect const & effect) override;

		void Bind(RenderEffect const& effect) override;
		void Unbind() override;

		std::span<uint8_t const> VsCode() const;
		uint32_t VsSignature() const;

	private:
		struct ParameterBind
		{
			RenderEffectParameter* param;
			uint32_t offset;
			std::function<void()> func;
		};

	public:
		explicit D3D11ShaderObject(std::shared_ptr<ShaderObjectTemplate> so_template);

	private:
		ParameterBind GetBindFunc(ShaderStage stage, uint32_t offset, RenderEffectParameter* param);

		void CreateHwResources(ShaderStage stage, RenderEffect const & effect) override;
		void DoLinkShaders(RenderEffect const & effect) override;

	private:
		std::array<std::vector<ParameterBind>, NumShaderStages> param_binds_;

		std::array<std::vector<ID3D11SamplerState*>, NumShaderStages> samplers_;
		std::array<std::vector<std::tuple<void*, uint32_t, uint32_t>>, NumShaderStages> srvsrcs_;
		std::array<std::vector<ID3D11ShaderResourceView*>, NumShaderStages> srvs_;
		std::vector<void*> uavsrcs_;
		std::vector<ID3D11UnorderedAccessView*> uavs_;
		std::vector<uint32_t> uav_init_counts_;
	};

	typedef std::shared_ptr<D3D11ShaderObject> D3D11ShaderObjectPtr;
}

#endif			// KLAYGE_PLUGINS_D3D11_SHADER_OBJECT_HPP
