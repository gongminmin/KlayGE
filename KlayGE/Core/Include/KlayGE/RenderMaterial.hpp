/**
 * @file RenderMaterial.hpp
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

#ifndef _RENDERMATERIAL_HPP
#define _RENDERMATERIAL_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <array>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderMaterial final : boost::noncopyable
	{
	public:
		enum TextureSlot
		{
			TS_Albedo,
			TS_MetalnessGlossiness,
			TS_Emissive,
			TS_Normal,
			TS_Height,
			TS_Occlusion,

			TS_NumTextureSlots
		};

		enum class SurfaceDetailMode : uint32_t
		{
			ParallaxMapping = 0,
			ParallaxOcclusionMapping,
			FlatTessellation,
			SmoothTessellation
		};

	public:
		RenderMaterial();

		RenderMaterialPtr Clone() const;

		void Name(std::string_view name)
		{
			name_ = std::string(name);
		}
		std::string const& Name() const
		{
			return name_;
		}

		void Albedo(float4 const& value);
		float4 const& Albedo() const;

		void Metalness(float value);
		float Metalness() const;
		void Glossiness(float value);
		float Glossiness() const;
		void Emissive(float3 const& value);
		float3 const& Emissive() const;

		void Transparent(bool value)
		{
			transparent_ = value;
		}
		bool Transparent() const
		{
			return transparent_;
		}
		void AlphaTestThreshold(float value);
		float AlphaTestThreshold() const;
		void Sss(bool value)
		{
			sss_ = value;
		}
		bool Sss() const
		{
			return sss_;
		}
		void TwoSided(bool value)
		{
			two_sided_ = value;
		}
		bool TwoSided() const
		{
			return two_sided_;
		}

		void NormalScale(float value);
		float NormalScale() const;
		void OcclusionStrength(float value);
		float OcclusionStrength() const;

		void TextureName(TextureSlot slot, std::string_view name);
		std::string const& TextureName(TextureSlot slot) const
		{
			return textures_[slot].first;
		}
		void Texture(TextureSlot slot, ShaderResourceViewPtr srv);
		ShaderResourceViewPtr const& Texture(TextureSlot slot) const
		{
			return textures_[slot].second;
		}

		void DetailMode(SurfaceDetailMode value)
		{
			detail_mode_ = value;
		}
		SurfaceDetailMode DetailMode() const
		{
			return detail_mode_;
		}
		void HeightOffset(float value);
		float HeightOffset() const;
		void HeightScale(float value);
		float HeightScale() const;
		void EdgeTessHint(float value);
		float EdgeTessHint() const;
		void InsideTessHint(float value);
		float InsideTessHint() const;
		void MinTessFactor(float value);
		float MinTessFactor() const;
		void MaxTessFactor(float value);
		float MaxTessFactor() const;

		void Active(RenderEffect& effect);

		void LoadTextureSlots();

	private:
		std::string name_;

		bool is_sw_mode_ = false;
		RenderEffectConstantBufferPtr cbuffer_;
		std::vector<uint32_t> sw_cbuffer_;
		RenderEffectParameter* albedo_tex_param_ = nullptr;
		RenderEffectParameter* metalness_glossiness_tex_param_ = nullptr;
		RenderEffectParameter* emissive_tex_param_ = nullptr;
		RenderEffectParameter* normal_tex_param_ = nullptr;
		RenderEffectParameter* height_tex_param_ = nullptr;
		RenderEffectParameter* occlusion_tex_param_ = nullptr;

		bool transparent_ = false;
		bool sss_ = false;
		bool two_sided_ = false;
		SurfaceDetailMode detail_mode_ = SurfaceDetailMode::ParallaxMapping;
		std::array<std::pair<std::string, ShaderResourceViewPtr>, TS_NumTextureSlots> textures_;
	};

	float const MAX_SHININESS = 8192;
	float const INV_LOG_MAX_SHININESS = 1 / log(MAX_SHININESS);

	inline float Shininess2Glossiness(float shininess)
	{
		return log(shininess) * INV_LOG_MAX_SHININESS;
	}

	inline float Glossiness2Shininess(float glossiness)
	{
		return pow(MAX_SHININESS, glossiness);
	}

	KLAYGE_CORE_API RenderMaterialPtr SyncLoadRenderMaterial(std::string_view mtlml_name);
	KLAYGE_CORE_API RenderMaterialPtr ASyncLoadRenderMaterial(std::string_view mtlml_name);
	KLAYGE_CORE_API void SaveRenderMaterial(RenderMaterialPtr const & mtl, std::string const & mtlml_name);
}

#endif		//_RENDERMATERIAL_HPP
