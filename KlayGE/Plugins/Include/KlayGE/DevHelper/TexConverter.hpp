/**
 * @file TexConverter.hpp
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

#ifndef KLAYGE_PLUGINS_TEX_CONVERTER_HPP
#define KLAYGE_PLUGINS_TEX_CONVERTER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>

#include <string_view>
#include <vector>

#include <KlayGE/DevHelper/DevHelper.hpp>
#include <KlayGE/DevHelper/TexMetadata.hpp>

namespace KlayGE
{
	class KLAYGE_DEV_HELPER_API TexConverter final
	{
	public:
		TexturePtr Load(std::string_view input_name, TexMetadata const & metadata);

		static void GetImageInfo(std::string_view input_name, TexMetadata const& metadata, Texture::TextureType& type, uint32_t& width,
			uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ElementFormat& format, uint32_t& row_pitch,
			uint32_t& slice_pitch);

		static bool IsSupported(std::string_view input_name);
	};
}

#endif		// KLAYGE_PLUGINS_TEX_CONVERTER_HPP
