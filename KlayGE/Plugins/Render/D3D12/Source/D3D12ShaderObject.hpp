/**
 * @file D3D12ShaderObject.hpp
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

#ifndef KLAYGE_PLUGINS_D3D12_SHADER_OBJECT_HPP
#define KLAYGE_PLUGINS_D3D12_SHADER_OBJECT_HPP

#pragma once

#include <KFL/CXX23/utility.hpp>
#include <KlayGE/ShaderObject.hpp>

#include "D3D12Util.hpp"

#if KLAYGE_IS_DEV_PLATFORM
struct ID3D12ShaderReflection;
#endif

namespace KlayGE
{
	struct D3D12ShaderDesc
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

	class D3D12ShaderStageObject : public ShaderStageObject
	{
	public:
		explicit D3D12ShaderStageObject(ShaderStage stage);

		void StreamIn(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res) override;
		void StreamOut(std::ostream& os) override;
		void CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
		void CreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

		std::span<uint8_t const> ShaderCodeBlob() const;

		std::string const& ShaderProfile() const noexcept
		{
			return shader_profile_;
		}

		D3D12ShaderDesc const& GetD3D12ShaderDesc() const noexcept
		{
			return shader_desc_;
		}

		std::vector<uint8_t> const& CBufferIndices() const noexcept
		{
			return cbuff_indices_;
		}

		virtual bool HasStreamOutput() const noexcept
		{
			return false;
		}

		virtual void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept;
		virtual void UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const noexcept;

	private:
		std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;
		void FillCBufferIndices(RenderEffect const& effect);

#if KLAYGE_IS_DEV_PLATFORM
		virtual void StageSpecificReflection([[maybe_unused]] ID3D12ShaderReflection* reflection)
		{
		}
#endif

	protected:
		bool is_available_;

		std::vector<uint8_t> shader_code_;
		std::string shader_profile_;
		D3D12ShaderDesc shader_desc_;
		std::vector<uint8_t> cbuff_indices_;
	};

	class D3D12VertexShaderStageObject final : public D3D12ShaderStageObject
	{
	public:
		D3D12VertexShaderStageObject();

		bool HasStreamOutput() const noexcept override
		{
			return !so_decl_.empty();
		}

		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept override;
		using D3D12ShaderStageObject::UpdatePsoDesc;

	private:
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		std::vector<D3D12_SO_DECLARATION_ENTRY> so_decl_;
		uint32_t rasterized_stream_ = 0;
	};

	class D3D12PixelShaderStageObject final : public D3D12ShaderStageObject
	{
	public:
		D3D12PixelShaderStageObject();

		bool HasDiscard() const noexcept override
		{
			return has_discard_;
		}

		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept override;
		using D3D12ShaderStageObject::UpdatePsoDesc;

	private:
		bool has_discard_ = true;
	};

	class D3D12GeometryShaderStageObject final : public D3D12ShaderStageObject
	{
	public:
		D3D12GeometryShaderStageObject();

		bool HasStreamOutput() const noexcept override
		{
			return !so_decl_.empty();
		}

		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept override;
		using D3D12ShaderStageObject::UpdatePsoDesc;

	private:
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		std::vector<D3D12_SO_DECLARATION_ENTRY> so_decl_;
		uint32_t rasterized_stream_ = 0;
	};

	class D3D12ComputeShaderStageObject final : public D3D12ShaderStageObject
	{
	public:
		D3D12ComputeShaderStageObject();

		uint32_t BlockSizeX() const noexcept override
		{
			return block_size_x_;
		}
		uint32_t BlockSizeY() const noexcept override
		{
			return block_size_y_;
		}
		uint32_t BlockSizeZ() const noexcept override
		{
			return block_size_z_;
		}

		void UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const noexcept override;
		using D3D12ShaderStageObject::UpdatePsoDesc;

	private:
		void StageSpecificStreamIn(ResIdentifier& res) override;
		void StageSpecificStreamOut(std::ostream& os) override;
#if KLAYGE_IS_DEV_PLATFORM
		void StageSpecificReflection(ID3D12ShaderReflection* reflection) override;
#endif
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		uint32_t block_size_x_, block_size_y_, block_size_z_;
	};

	class D3D12HullShaderStageObject final : public D3D12ShaderStageObject
	{
	public:
		D3D12HullShaderStageObject();

		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept override;
		using D3D12ShaderStageObject::UpdatePsoDesc;

	private:
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;
	};

	class D3D12DomainShaderStageObject final : public D3D12ShaderStageObject
	{
	public:
		D3D12DomainShaderStageObject();

		bool HasStreamOutput() const noexcept override
		{
			return !so_decl_.empty();
		}

		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept override;
		using D3D12ShaderStageObject::UpdatePsoDesc;

	private:
		void StageSpecificCreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) override;

	private:
		std::vector<D3D12_SO_DECLARATION_ENTRY> so_decl_;
		uint32_t rasterized_stream_ = 0;
	};

	class D3D12ShaderObject final : public ShaderObject
	{
	public:
		D3D12ShaderObject();
		~D3D12ShaderObject() override;

		ShaderObjectPtr Clone(RenderEffect& dst_effect) override;

		void Bind(RenderEffect const& effect) override;
		void Unbind() override;

		uint32_t NumSrvs(ShaderStage stage) const noexcept
		{
			return d3d_immutable_->num_srvs_[std::to_underlying(stage)];
		}
		uint32_t NumUavs(ShaderStage stage) const noexcept
		{
			return d3d_immutable_->num_uavs_[std::to_underlying(stage)];
		}
		uint32_t NumSamplers(ShaderStage stage) const noexcept
		{
			return d3d_immutable_->num_samplers_[std::to_underlying(stage)];
		}

		uint32_t NumCBuffers(ShaderStage stage) const noexcept;
		D3D12_GPU_VIRTUAL_ADDRESS CBufferGpuVAddr(RenderEffect const& effect, ShaderStage stage, uint32_t index) const noexcept;

		ID3D12RootSignature* RootSignature() const noexcept
		{
			return d3d_immutable_->root_signature_.get();
		}
		D3D12GpuDescriptorBlock const& SrvUavDescBlock() const noexcept
		{
			return srv_uav_desc_block_;
		}
		D3D12GpuDescriptorBlock const& SamplerDescBlock() const noexcept
		{
			return d3d_immutable_->sampler_desc_block_;
		}

		void* GetD3D12ShaderObjectTemplate() noexcept
		{
			return d3d_immutable_.get();
		}
		void const* GetD3D12ShaderObjectTemplate() const noexcept
		{
			return d3d_immutable_.get();
		}

		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept;
		void UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const noexcept;

	private:
		struct D3D12Immutable
		{
			~D3D12Immutable();

			ID3D12RootSignaturePtr root_signature_;
			D3D12GpuDescriptorBlock sampler_desc_block_;

			std::array<uint32_t, NumShaderStages> num_srvs_{};
			std::array<uint32_t, NumShaderStages> num_uavs_{};
			std::array<uint32_t, NumShaderStages> num_samplers_{};

			uint32_t num_total_srvs_;
		};

		struct ParameterBind
		{
			RenderEffectParameter const* param;
			uint32_t offset;
			std::function<void()> update;
		};

	public:
		D3D12ShaderObject(std::shared_ptr<Immutable> immutable, std::shared_ptr<D3D12Immutable> d3d_immutable) noexcept;

	private:
		ParameterBind GetBindFunc(uint32_t srv_stage_base, uint32_t uav_stage_base, uint32_t offset, RenderEffectParameter const& param);

		void DoLinkShaders(RenderEffect& effect) override;

	private:
		const std::shared_ptr<D3D12Immutable> d3d_immutable_;

		std::array<std::vector<ParameterBind>, NumShaderStages> param_binds_;
		std::vector<std::tuple<D3D12Resource*, uint32_t, uint32_t>> srv_uav_srcs_;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srv_uav_handles_;
		D3D12GpuDescriptorBlock srv_uav_desc_block_;
	};

	typedef std::shared_ptr<D3D12ShaderObject> D3D12ShaderObjectPtr;
}

#endif			// KLAYGE_PLUGINS_D3D12_SHADER_OBJECT_HPP
