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

#ifndef KLAYGE_TOOLS_TOOL_COMMON_TEX_CONVERTER_HPP
#define KLAYGE_TOOLS_TOOL_COMMON_TEX_CONVERTER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KlayGE/ElementFormat.hpp>

#include <vector>

#include <KlayGE/ToolCommon.hpp>
#include <KlayGE/TexMetadata.hpp>

namespace KlayGE
{
	class ImagePlane;

	class KLAYGE_TOOL_API TexConverter
	{
	public:
		bool Convert(std::string_view input_name, TexMetadata const & metadata,
			Texture::TextureType& output_type, uint32_t& output_width, uint32_t& output_height, uint32_t& output_depth,
			uint32_t& output_num_mipmaps, uint32_t& output_array_size,
			ElementFormat& output_format, std::vector<ElementInitData>& output_init_data, std::vector<uint8_t>& output_data_block);

	private:
		bool Load();
		void Save(Texture::TextureType& output_type, uint32_t& output_width, uint32_t& output_height, uint32_t& output_depth,
			uint32_t& output_num_mipmaps, uint32_t& output_array_size,
			ElementFormat& output_format, std::vector<ElementInitData>& output_init_data, std::vector<uint8_t>& output_data_block);

	private:
		std::string input_name_;

		TexMetadata metadata_;

		std::vector<std::vector<std::shared_ptr<ImagePlane>>> planes_;
		uint32_t width_;
		uint32_t height_;
		uint32_t array_size_;
		uint32_t num_mipmaps_;
		ElementFormat format_;
	};
}

#endif		// KLAYGE_TOOLS_TOOL_COMMON_TEX_CONVERTER_HPP
