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

namespace KlayGE
{
	struct shader_desc
	{
		shader_desc()
			: tech_pass(0xFFFFFFFF)
		{
		}

		std::string profile;
		std::string func_name;

		uint32_t tech_pass;

#ifdef KLAYGE_PLATFORM_WINDOWS
		#pragma pack(push, 1)
#endif
		struct stream_output_decl
		{
			VertexElementUsage usage;
			uint8_t usage_index;
			uint8_t start_component;
			uint8_t component_count;

			friend bool operator==(stream_output_decl const & lhs, stream_output_decl const & rhs)
			{
				return (lhs.usage == rhs.usage) && (lhs.usage_index == rhs.usage_index)
					&& (lhs.start_component == rhs.start_component) && (lhs.component_count == rhs.component_count);
			}
			friend bool operator!=(stream_output_decl const & lhs, stream_output_decl const & rhs)
			{
				return !(lhs == rhs);
			}
		};
#ifdef KLAYGE_PLATFORM_WINDOWS
		#pragma pack(pop)
#endif
		std::vector<stream_output_decl> so_decl;

		friend bool operator==(shader_desc const & lhs, shader_desc const & rhs)
		{
			return (lhs.profile == rhs.profile) && (lhs.func_name == rhs.func_name) && (lhs.so_decl == rhs.so_decl);
		}
		friend bool operator!=(shader_desc const & lhs, shader_desc const & rhs)
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
		virtual ~ShaderObject()
		{
		}

		static ShaderObjectPtr NullObject();

		virtual void SetShader(RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<ShaderObjectPtr> const & shared_so) = 0;
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

	protected:
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;
		
		bool is_validate_;
		bool has_discard_;
		bool has_tessellation_;
	};
}

#endif			// _SHADEROBJECT_HPP
