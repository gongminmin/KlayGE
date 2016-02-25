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

#ifndef _D3D12SHADEROBJECT_HPP
#define _D3D12SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>

namespace KlayGE
{
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 2)
#endif
	struct D3D12ShaderParameterHandle
	{
		uint32_t shader_type;

		D3D_SHADER_VARIABLE_TYPE param_type;

		uint32_t cbuff;

		uint32_t offset;
		uint32_t elements;
		uint8_t rows;
		uint8_t columns;
	};

	struct D3D12ShaderDesc
	{
		D3D12ShaderDesc()
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

	class D3D12ShaderObject : public ShaderObject
	{
	public:
		D3D12ShaderObject();

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

		std::shared_ptr<std::vector<uint8_t>> const & ShaderBlob(ShaderType type) const
		{
			return shader_code_[type].first;
		}

		uint32_t VSSignature() const
		{
			return vs_signature_;
		}


		std::vector<D3D12_SAMPLER_DESC> const & Samplers(ShaderType type) const
		{
			return samplers_[type];
		}

		std::vector<std::tuple<ID3D12Resource*, uint32_t, uint32_t>> const & SRVSrcs(ShaderType type) const
		{
			return srvsrcs_[type];
		}

		std::vector<D3D12ShaderResourceViewSimulation*> const & SRVs(ShaderType type) const
		{
			return srvs_[type];
		}

		std::vector<std::pair<ID3D12Resource*, ID3D12Resource*>> const & UAVSrcs(ShaderType type) const
		{
			return uavsrcs_[type];
		}

		std::vector<D3D12UnorderedAccessViewSimulation*> const & UAVs(ShaderType type) const
		{
			return uavs_[type];
		}

		std::vector<GraphicsBuffer*> const & CBuffers(ShaderType type) const
		{
			return d3d_cbuffs_[type];
		}

		std::vector<D3D12_SO_DECLARATION_ENTRY> const & SODecl() const
		{
			return so_decl_;
		}
		
		uint32_t RasterizedStream() const
		{
			return rasterized_stream_;
		}

		ID3D12RootSignaturePtr const & RootSignature() const
		{
			return root_signature_;
		}
		ID3D12DescriptorHeapPtr const & SamplerHeap() const
		{
			return sampler_heap_;
		}

	private:
		struct parameter_bind_t
		{
			RenderEffectParameterPtr param;
			D3D12ShaderParameterHandle p_handle;
			std::function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		parameter_bind_t GetBindFunc(D3D12ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param);

		std::string GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id);
		std::shared_ptr<std::vector<uint8_t>> CompiteToBytecode(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids);
		void AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
			std::vector<uint32_t> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob);

		void CreateRootSignature();

	private:
		std::array<parameter_binds_t, ST_NumShaderTypes> param_binds_;

		std::array<std::pair<std::shared_ptr<std::vector<uint8_t>>, std::string>, ST_NumShaderTypes> shader_code_;
		std::array<std::shared_ptr<D3D12ShaderDesc>, ST_NumShaderTypes> shader_desc_;

		std::array<std::vector<D3D12_SAMPLER_DESC>, ST_NumShaderTypes> samplers_;
		std::array<std::vector<std::tuple<ID3D12Resource*, uint32_t, uint32_t>>, ST_NumShaderTypes> srvsrcs_;
		std::array<std::vector<D3D12ShaderResourceViewSimulation*>, ST_NumShaderTypes> srvs_;
		std::array<std::vector<std::pair<ID3D12Resource*, ID3D12Resource*>>, ST_NumShaderTypes> uavsrcs_;
		std::array<std::vector<D3D12UnorderedAccessViewSimulation*>, ST_NumShaderTypes> uavs_;
		std::array<std::shared_ptr<std::vector<uint8_t>>, ST_NumShaderTypes> cbuff_indices_;
		std::array<std::vector<GraphicsBuffer*>, ST_NumShaderTypes> d3d_cbuffs_;
		bool vs_so_;
		bool ds_so_;
		std::vector<D3D12_SO_DECLARATION_ENTRY> so_decl_;
		uint32_t rasterized_stream_;

		std::vector<RenderEffectConstantBufferPtr> all_cbuffs_;

		uint32_t vs_signature_;

		ID3D12RootSignaturePtr root_signature_;
		ID3D12DescriptorHeapPtr sampler_heap_;
	};

	typedef std::shared_ptr<D3D12ShaderObject> D3D12ShaderObjectPtr;
}

#endif			// _D3D12SHADEROBJECT_HPP
