// D3D11ShaderObject.hpp
// KlayGE D3D11 shader对象类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11SHADEROBJECT_HPP
#define _D3D11SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11ShaderObject : public ShaderObject
	{
	public:
		D3D11ShaderObject();

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

		std::shared_ptr<std::vector<uint8_t>> const & VSCode() const
		{
			return so_template_->shader_code_[ST_VertexShader].first;
		}

		uint32_t VSSignature() const
		{
			return so_template_->vs_signature_;
		}

	private:
		struct D3D11ShaderObjectTemplate
		{
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

			ID3D11VertexShaderPtr vertex_shader_;
			ID3D11PixelShaderPtr pixel_shader_;
			ID3D11GeometryShaderPtr geometry_shader_;
			ID3D11ComputeShaderPtr compute_shader_;
			ID3D11HullShaderPtr hull_shader_;
			ID3D11DomainShaderPtr domain_shader_;
			std::array<std::pair<std::shared_ptr<std::vector<uint8_t>>, std::string>, ST_NumShaderTypes> shader_code_;
			std::array<std::shared_ptr<D3D11ShaderDesc>, ST_NumShaderTypes> shader_desc_;
			std::array<std::shared_ptr<std::vector<uint8_t>>, ST_NumShaderTypes> cbuff_indices_;

			uint32_t vs_signature_;
		};

		struct ParameterBind
		{
			RenderEffectParameter* param;
			uint32_t offset;
			std::function<void()> func;
		};

	public:
		explicit D3D11ShaderObject(std::shared_ptr<D3D11ShaderObjectTemplate> const & so_template);

	private:
		ParameterBind GetBindFunc(ShaderType type, uint32_t offset, RenderEffectParameter* param);

		std::string_view GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id);
		std::shared_ptr<std::vector<uint8_t>> CompiteToBytecode(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids);
		void AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob);
		void CreateGeometryShaderWithStreamOutput(ShaderType type, RenderEffect const & effect,
			std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob,
			std::vector<ShaderDesc::StreamOutputDecl> const & so_decl);

	private:
		std::shared_ptr<D3D11ShaderObjectTemplate> so_template_;

		std::array<std::vector<ParameterBind>, ST_NumShaderTypes> param_binds_;

		std::array<std::vector<ID3D11SamplerState*>, ST_NumShaderTypes> samplers_;
		std::array<std::vector<std::tuple<void*, uint32_t, uint32_t>>, ST_NumShaderTypes> srvsrcs_;
		std::array<std::vector<ID3D11ShaderResourceView*>, ST_NumShaderTypes> srvs_;
		std::array<std::vector<ID3D11Buffer*>, ST_NumShaderTypes> d3d11_cbuffs_;
		std::vector<void*> uavsrcs_;
		std::vector<ID3D11UnorderedAccessView*> uavs_;

		std::vector<RenderEffectConstantBuffer*> all_cbuffs_;
	};

	typedef std::shared_ptr<D3D11ShaderObject> D3D11ShaderObjectPtr;
}

#endif			// _D3D11SHADEROBJECT_HPP
