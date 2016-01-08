// ShaderObject.hpp
// KlayGE shader对象类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 支持Gemoetry Shader (2009.2.5)
//
// 3.7.0
// 改为直接传入RenderEffect (2008.7.4)
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SHADEROBJECT_HPP
#define _SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <array>

namespace KlayGE
{
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

	class KLAYGE_CORE_API ShaderObject
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
		ShaderObject();
		virtual ~ShaderObject()
		{
		}

		virtual bool AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<uint8_t> const & native_shader_block) = 0;

		virtual bool StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
			std::vector<uint32_t> const & shader_desc_ids) = 0;
		virtual void StreamOut(std::ostream& os, ShaderType type) = 0;

		virtual void AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids) = 0;
		virtual void AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so) = 0;
		virtual void LinkShaders(RenderEffect const & effect) = 0;
		virtual ShaderObjectPtr Clone(RenderEffect const & effect) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		bool ShaderValidate(ShaderType type) const
		{
			return is_shader_validate_[type];
		}
		bool Validate() const
		{
			return is_validate_;
		}

		bool HasDiscard() const
		{
			return has_discard_;
		}
		bool HasTessellation() const
		{
			return has_tessellation_;
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
		std::array<bool, ST_NumShaderTypes> is_shader_validate_;
		
		bool is_validate_;
		bool has_discard_;
		bool has_tessellation_;
		uint32_t cs_block_size_x_, cs_block_size_y_, cs_block_size_z_;
	};
}

#endif			// _SHADEROBJECT_HPP
