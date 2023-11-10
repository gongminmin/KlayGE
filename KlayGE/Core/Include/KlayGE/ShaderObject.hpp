/**
 * @file ShaderObject.hpp
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

#ifndef KLAYGE_CORE_SHADER_OBJECT_HPP
#define KLAYGE_CORE_SHADER_OBJECT_HPP

#pragma once

#include <KFL/CXX23/utility.hpp>
#include <KFL/Noncopyable.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <array>
#include <memory>

namespace KlayGE
{
	class RenderEffect;
	class RenderPass;
	class RenderTechnique;

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

		struct StreamOutputDecl
		{
			VertexElementUsage usage;
			uint8_t usage_index;
			uint8_t start_component;
			uint8_t component_count;
			uint8_t slot;

			friend bool operator==(StreamOutputDecl const& lhs, StreamOutputDecl const& rhs) noexcept
			{
				return (lhs.usage == rhs.usage) && (lhs.usage_index == rhs.usage_index)
					&& (lhs.start_component == rhs.start_component) && (lhs.component_count == rhs.component_count)
					&& (lhs.slot == rhs.slot);
			}

			KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(StreamOutputDecl);
		};
		static_assert(sizeof(StreamOutputDecl) == 8);

		std::vector<StreamOutputDecl> so_decl;

		friend bool operator==(ShaderDesc const& lhs, ShaderDesc const& rhs) noexcept
		{
			return (lhs.profile == rhs.profile) && (lhs.func_name == rhs.func_name)
				&& (lhs.macros_hash == rhs.macros_hash) && (lhs.so_decl == rhs.so_decl);
		}
		friend bool operator!=(ShaderDesc const& lhs, ShaderDesc const& rhs) noexcept
		{
			return !(lhs == rhs);
		}
	};

	enum class ShaderStage : uint32_t
	{
		Vertex,
		Pixel,
		Geometry,
		Compute,
		Hull,
		Domain,

		NumStages,
	};
	uint32_t constexpr NumShaderStages = std::to_underlying(ShaderStage::NumStages);

	class KLAYGE_CORE_API ShaderStageObject
	{
		KLAYGE_NONCOPYABLE(ShaderStageObject);

	public:
		explicit ShaderStageObject(ShaderStage stage) noexcept;
		virtual ~ShaderStageObject() noexcept;

		virtual void StreamIn(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res) = 0;
		virtual void StreamOut(std::ostream& os) = 0;
		virtual void CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
			std::array<uint32_t, NumShaderStages> const& shader_desc_ids) = 0;
		virtual void CreateHwShader(
			RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids) = 0;

		bool Validate() const noexcept
		{
			return is_validate_;
		}

		// Pixel shader only
		virtual bool HasDiscard() const noexcept
		{
			return false;
		}

		// Compute shader only
		virtual uint32_t BlockSizeX() const noexcept
		{
			return 0;
		}
		virtual uint32_t BlockSizeY() const noexcept
		{
			return 0;
		}
		virtual uint32_t BlockSizeZ() const noexcept
		{
			return 0;
		}

		bool HWResourceReady() const noexcept
		{
			return hw_res_ready_;
		}

	protected:
		static std::vector<uint8_t> CompileToDXBC(ShaderStage stage, RenderEffect const& effect, RenderTechnique const& tech,
			RenderPass const& pass, std::vector<std::pair<char const*, char const*>> const& api_special_macros, char const* func_name,
			char const* shader_profile, uint32_t flags, void** reflector, bool strip);

		virtual std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const = 0;

		virtual void StageSpecificStreamIn([[maybe_unused]] ResIdentifier& res)
		{
		}
		virtual void StageSpecificStreamOut([[maybe_unused]] std::ostream& os)
		{
		}
		virtual void StageSpecificCreateHwShader(
			[[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
		{
		}

	protected:
		const ShaderStage stage_;

		bool is_validate_ = false;
		bool hw_res_ready_ = false;
	};

	using ShaderStageObjectPtr = std::shared_ptr<ShaderStageObject>;

	class ShaderObject;
	using ShaderObjectPtr = std::shared_ptr<ShaderObject>;

	class KLAYGE_CORE_API ShaderObject
	{
		KLAYGE_NONCOPYABLE(ShaderObject);

	public:
		ShaderObject();
		virtual ~ShaderObject() noexcept;

		void AttachStage(ShaderStage stage, ShaderStageObjectPtr const& shader_stage);
		ShaderStageObjectPtr const& Stage(ShaderStage stage) const noexcept;

		void LinkShaders(RenderEffect& effect);
		virtual ShaderObjectPtr Clone(RenderEffect& dst_effect) = 0;

		virtual void Bind(RenderEffect const& effect) = 0;
		virtual void Unbind() = 0;

		bool Validate() const noexcept
		{
			return immutable_->is_validate_;
		}

		bool HWResourceReady() const noexcept
		{
			return hw_res_ready_;
		}

	protected:
		struct Immutable
		{
			std::array<ShaderStageObjectPtr, NumShaderStages> shader_stages_;
			bool is_validate_;
		};

		explicit ShaderObject(std::shared_ptr<Immutable> immutable) noexcept;

	private:
		virtual void DoLinkShaders(RenderEffect& effect) = 0;

	protected:
		std::shared_ptr<Immutable> const immutable_;

		bool shader_stages_dirty_ = true;

		bool hw_res_ready_ = false;
	};
}

#endif			// KLAYGE_CORE_SHADER_OBJECT_HPP
