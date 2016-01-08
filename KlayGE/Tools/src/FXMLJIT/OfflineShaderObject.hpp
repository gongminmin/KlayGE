/**
* @file OfflineShaderObject.hpp
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

#ifndef _OFFLINESHADEROBJECT_HPP
#define _OFFLINESHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>

namespace KlayGE
{
	namespace Offline
	{
		class RenderEffect;
		class RenderTechnique;
		class RenderPass;
		class ShaderObject;
		typedef std::shared_ptr<ShaderObject> ShaderObjectPtr;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
		struct OfflineRenderDeviceCaps
		{
			std::string platform;
			uint8_t major_version;
			uint8_t minor_version;

			bool requires_flipping;
			uint32_t native_shader_fourcc;
			uint32_t native_shader_version;

			ShaderModel max_shader_model;

			uint32_t max_texture_depth;
			uint32_t max_texture_array_length;
			uint8_t max_pixel_texture_units;
			uint8_t max_simultaneous_rts;

			bool standard_derivatives_support : 1;
			bool shader_texture_lod_support : 1;
			bool fp_color_support : 1;
			bool pack_to_rgba_required : 1;

			bool gs_support : 1;
			bool cs_support : 1;
			bool hs_support : 1;
			bool ds_support : 1;

			bool bc4_support : 1;
			bool bc5_support : 1;
			bool frag_depth_support : 1;
			bool ubo_support : 1;
		};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

		struct ShaderDesc
		{
			ShaderDesc()
				: macros_hash(0), tech_pass_type(0xFFFFFFFF)
			{
			}

			std::string profile;
			std::string func_name;
			uint64_t macros_hash;

			uint32_t tech_pass_type;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
			struct StreamOutputDecl
			{
				VertexElementUsage usage;
				uint8_t usage_index;
				uint8_t start_component;
				uint8_t component_count;

				friend bool operator==(StreamOutputDecl const & lhs, StreamOutputDecl const & rhs)
				{
					return (lhs.usage == rhs.usage) && (lhs.usage_index == rhs.usage_index)
						&& (lhs.start_component == rhs.start_component) && (lhs.component_count == rhs.component_count);
				}
				friend bool operator!=(StreamOutputDecl const & lhs, StreamOutputDecl const & rhs)
				{
					return !(lhs == rhs);
				}
			};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif
			std::vector<StreamOutputDecl> so_decl;

			friend bool operator==(ShaderDesc const & lhs, ShaderDesc const & rhs)
			{
				return (lhs.profile == rhs.profile) && (lhs.func_name == rhs.func_name)
					&& (lhs.macros_hash == rhs.macros_hash) && (lhs.so_decl == rhs.so_decl);
			}
			friend bool operator!=(ShaderDesc const & lhs, ShaderDesc const & rhs)
			{
				return !(lhs == rhs);
			}
		};

		class ShaderObject
		{
		public:
			enum ShaderType
			{
				ST_VertexShader,
				ST_PixelShader,
				ST_GeometryShader,
				ST_ComputeShader,
				ST_HullShader,
				ST_DomainShader,

				ST_NumShaderTypes
			};

		public:
			explicit ShaderObject(OfflineRenderDeviceCaps const & caps);
			virtual ~ShaderObject()
			{
			}

			virtual void StreamOut(std::ostream& os, ShaderType type) = 0;

			virtual void AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids) = 0;
			virtual void AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so) = 0;
			virtual void LinkShaders(RenderEffect const & effect) = 0;

			bool ShaderValidate(ShaderType type) const
			{
				return is_shader_validate_[type];
			}
			bool Validate() const
			{
				return is_validate_;
			}

			uint32_t CSBlockSizeX() const
			{
				return cs_block_size_x_;
			}
			uint32_t CSBlockSizeY() const
			{
				return cs_block_size_y_;
			}
			uint32_t CSBlockSizeZ() const
			{
				return cs_block_size_z_;
			}

		protected:
			std::vector<uint8_t> CompileToDXBC(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass,
				std::vector<std::pair<char const *, char const *>> const & api_special_macros,
				char const * func_name, char const * shader_profile, uint32_t flags);
			void ReflectDXBC(std::vector<uint8_t> const & code, void** reflector);
			std::vector<uint8_t> StripDXBC(std::vector<uint8_t> const & code, uint32_t strip_flags);

		protected:
			OfflineRenderDeviceCaps caps_;

			std::array<bool, ST_NumShaderTypes> is_shader_validate_;

			bool is_validate_;
			uint32_t cs_block_size_x_, cs_block_size_y_, cs_block_size_z_;
		};
	}
}

#endif			// _OFFLINESHADEROBJECT_HPP
