/**
 * @file TexMetadata.hpp
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

#ifndef KLAYGE_PLUGINS_TEX_METADATA_HPP
#define KLAYGE_PLUGINS_TEX_METADATA_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/Texture.hpp>

#include <string>
#include <vector>

#include <KlayGE/DevHelper/DevHelper.hpp>

namespace KlayGE
{
	class KLAYGE_DEV_HELPER_API TexMetadata final
	{
	public:
		TexMetadata();
		explicit TexMetadata(std::string_view name);
		TexMetadata(std::string_view name, bool assign_default_values);

		void Load(std::string_view name);
		void Load(std::string_view name, bool assign_default_values);
		void Save(std::string const & name) const;
		void DeviceDependentAdjustment(RenderDeviceCaps const & caps);

		Texture::TextureType TextureType() const
		{
			return type_;
		}

		RenderMaterial::TextureSlot Slot() const
		{
			return slot_;
		}
		void Slot(RenderMaterial::TextureSlot slot)
		{
			slot_ = slot;
		}

		ElementFormat PreferedFormat() const
		{
			return prefered_format_;
		}
		void PreferedFormat(ElementFormat format)
		{
			prefered_format_ = format;
		}

		bool ForceSRGB() const
		{
			return force_srgb_;
		}
		void ForceSRGB(bool srgb)
		{
			force_srgb_ = srgb;
		}

		int8_t ChannelMapping(uint32_t channel) const
		{
			return channel_mapping_[channel];
		}
		void ChannelMapping(uint32_t channel, int8_t mapping)
		{
			channel_mapping_[channel] = mapping;
		}

		bool RgbToLum() const
		{
			return rgb_to_lum_;
		}
		void RgbToLum(bool rgb_to_lum)
		{
			rgb_to_lum_ = rgb_to_lum;
		}

		bool MipmapEnabled() const
		{
			return mipmap_.enabled;
		}
		void MipmapEnabled(bool mip)
		{
			mipmap_.enabled = mip;
		}
		bool AutoGenMipmap() const
		{
			return mipmap_.auto_gen;
		}
		void AutoGenMipmap(bool auto_gen)
		{
			mipmap_.auto_gen = auto_gen;
		}
		uint32_t NumMipmaps() const
		{
			return mipmap_.num_levels;
		}
		void NumMipmaps(uint32_t levels)
		{
			mipmap_.num_levels = levels;
		}
		bool LinearMipmap() const
		{
			return mipmap_.linear;
		}
		void LinearMipmap(bool linear)
		{
			mipmap_.linear = linear;
		}

		bool BumpToNormal() const
		{
			return bump_.to_normal;
		}
		void BumpToNormal(bool to_normal)
		{
			bump_.to_normal = to_normal;
		}
		float BumpScale() const
		{
			return bump_.scale;
		}
		void BumpScale(float scale)
		{
			bump_.scale = scale;
		}
		bool BumpToOcclusion() const
		{
			return bump_.to_occlusion;
		}
		void BumpToOcclusion(bool to_occlusion)
		{
			bump_.to_occlusion = to_occlusion;
		}
		float OcclusionAmplitude() const
		{
			return bump_.occlusion_amplitude;
		}
		void OcclusionAmplitude(float amplitude)
		{
			bump_.occlusion_amplitude = amplitude;
		}

		bool NormalToHeight() const
		{
			return bump_.from_normal;
		}
		void NormalToHeight(bool from_normal)
		{
			bump_.from_normal = from_normal;
		}
		float HeightMinZ() const
		{
			return bump_.min_z;
		}
		void HeightMinZ(float min_z)
		{
			bump_.min_z = min_z;
		}

		uint32_t ArraySize() const;
		void ArraySize(uint32_t size);
		std::string_view PlaneFileName(uint32_t array_index, uint32_t mip) const;
		void PlaneFileName(uint32_t array_index, uint32_t mip, std::string_view name);

	private:
		Texture::TextureType type_ = Texture::TT_2D;

		RenderMaterial::TextureSlot slot_ = RenderMaterial::TS_Albedo;

		ElementFormat prefered_format_ = EF_Unknown;
		bool force_srgb_ = false;
		int8_t channel_mapping_[4] = { 0, 1, 2, 3 };
		bool rgb_to_lum_ = false;

		struct Mipmap
		{
			bool enabled = false;
			bool auto_gen = true;
			uint32_t num_levels = 0;
			bool linear = true;
		};
		Mipmap mipmap_;

		struct Bump
		{
			bool to_normal = false;
			float scale = 1.0f;
			bool to_occlusion = false;
			float occlusion_amplitude = 1.0f;

			bool from_normal = false;
			float min_z = 1e-6f;
		};
		Bump bump_;

		// Array    Mipmap
		std::vector<std::vector<std::string>> plane_file_names_;
	};
}

#endif		// KLAYGE_PLUGINS_TEX_METADATA_HPP
