/**
* @file OfflineD3D11ShaderObject.hpp
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

#ifndef _OFFLINED3D11SHADEROBJECT_HPP
#define _OFFLINED3D11SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include "OfflineShaderObject.hpp"

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <D3D11Shader.h>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	namespace Offline
	{
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 2)
#endif
		struct D3D11ShaderParameterHandle
		{
			uint32_t shader_type;

			D3D_SHADER_VARIABLE_TYPE param_type;

			uint32_t cbuff;

			uint32_t offset;
			uint32_t elements;
			uint8_t rows;
			uint8_t columns;
		};

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

		class D3D11ShaderObject : public ShaderObject
		{
		public:
			explicit D3D11ShaderObject(OfflineRenderDeviceCaps const & caps);

			std::string GenShaderText(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass) const;

			virtual void StreamOut(std::ostream& os, ShaderType type) KLAYGE_OVERRIDE;

			void AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids);
			void AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so);
			void LinkShaders(RenderEffect const & effect);

			shared_ptr<std::vector<uint8_t> > const & VSCode() const
			{
				return shader_code_[ST_VertexShader].first;
			}

			uint32_t VSSignature() const
			{
				return vs_signature_;
			}

		private:
			struct parameter_bind_t
			{
				RenderEffectParameterPtr param;
				D3D11ShaderParameterHandle p_handle;
			};
			typedef std::vector<parameter_bind_t> parameter_binds_t;

			std::string GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id);
			shared_ptr<std::vector<uint8_t> > CompiteToBytecode(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids);
			void AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
				std::vector<uint32_t> const & shader_desc_ids, shared_ptr<std::vector<uint8_t> > const & code_blob);

		private:
			array<std::pair<shared_ptr<std::vector<uint8_t> >, std::string>, ST_NumShaderTypes> shader_code_;
			array<D3D11ShaderDesc, ST_NumShaderTypes> shader_desc_;

			array<std::vector<uint8_t>, ST_NumShaderTypes> cbuff_indices_;

			uint32_t vs_signature_;

			std::string vs_profile_, ps_profile_, gs_profile_, cs_profile_, hs_profile_, ds_profile_;
		};

		typedef shared_ptr<D3D11ShaderObject> D3D11ShaderObjectPtr;
	}
}

#endif			// _D3D11SHADEROBJECT_HPP
