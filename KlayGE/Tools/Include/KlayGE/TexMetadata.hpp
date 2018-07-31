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

#ifndef KLAYGE_TOOLS_IMAGE_CONV_TEX_METADATA_HPP
#define KLAYGE_TOOLS_IMAGE_CONV_TEX_METADATA_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Texture.hpp>

#include <vector>

#include <KlayGE/ToolCommon.hpp>

namespace KlayGE
{
	enum TextureSlot
	{
		TS_Albedo,
		TS_Metalness,
		TS_Glossiness,
		TS_Emissive,
		TS_Normal,
		TS_Height,

		// Offline only
		TS_Bump,	// Will be converted to normal map

		TS_NumTextureSlots
	};

	class KLAYGE_TOOL_API TexMetadata
	{
	public:
		TexMetadata();
		TexMetadata(std::string_view name);

		void Load(std::string_view name);

		Texture::TextureType Type() const
		{
			return type_;
		}

		TextureSlot Slot() const
		{
			return slot_;
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

		uint8_t ChannelMapping(uint32_t channel) const
		{
			return channel_mapping_[channel];
		}

		bool MipmapEnabled() const
		{
			return mipmap_.enabled;
		}
		bool AutoGenMipmap() const
		{
			return mipmap_.auto_gen;
		}
		uint32_t NumMipmaps() const
		{
			return mipmap_.num_levels;
		}
		bool LinearMipmap() const
		{
			return mipmap_.linear;
		}

		uint32_t ArraySize() const;
		std::string_view PlaneFileName(uint32_t array_index, uint32_t mip) const;

	private:
		Texture::TextureType type_ = Texture::TT_2D;

		TextureSlot slot_ = TS_Albedo;

		ElementFormat prefered_format_ = EF_Unknown;
		bool force_srgb_ = false;
		uint8_t channel_mapping_[4] = { 0, 1, 2, 3 };

		struct Mipmap
		{
			bool enabled = false;
			bool auto_gen = true;
			uint32_t num_levels = 0;
			bool linear = true;
		};
		Mipmap mipmap_;

		// Array    Mipmap
		std::vector<std::vector<std::string>> plane_file_names_;
	};
}

#endif		// KLAYGE_TOOLS_IMAGE_CONV_TEX_METADATA_HPP
