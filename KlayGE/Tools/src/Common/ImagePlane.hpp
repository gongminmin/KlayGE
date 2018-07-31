/**
 * @file ImagePlane.hpp
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

#ifndef KLAYGE_TOOLS_IMAGE_CONV_IMAGE_PLANE_HPP
#define KLAYGE_TOOLS_IMAGE_CONV_IMAGE_PLANE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KlayGE/ElementFormat.hpp>

#include <vector>

namespace KlayGE
{
	class TexMetadata;

	class ImagePlane
	{
	public:
		bool Load(std::string_view name, TexMetadata const & metadata);
		void FormatConversion(ElementFormat format);
		ImagePlane ResizeTo(uint32_t width, uint32_t height, bool linear);

		std::vector<uint8_t> const & UncompressedData() const
		{
			return uncompressed_data_;
		}
		std::vector<uint8_t> const & CompressedData() const
		{
			return compressed_data_;
		}
		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}
		uint32_t UncompressedRowPitch() const
		{
			return uncompressed_row_pitch_;
		}
		uint32_t CompressedRowPitch() const
		{
			return compressed_row_pitch_;
		}
		uint32_t UncompressedSlicePitch() const
		{
			return uncompressed_slice_pitch_;
		}
		uint32_t CompressedSlicePitch() const
		{
			return compressed_slice_pitch_;
		}
		ElementFormat UncompressedFormat() const
		{
			return uncompressed_format_;
		}
		ElementFormat CompressedFormat() const
		{
			return compressed_format_;
		}

	private:
		std::vector<uint8_t> uncompressed_data_;
		std::vector<uint8_t> compressed_data_;
		uint32_t width_;
		uint32_t height_;
		uint32_t uncompressed_row_pitch_;
		uint32_t compressed_row_pitch_;
		uint32_t uncompressed_slice_pitch_;
		uint32_t compressed_slice_pitch_;
		ElementFormat uncompressed_format_;
		ElementFormat compressed_format_ = EF_Unknown;
	};
}

#endif		// KLAYGE_TOOLS_IMAGE_CONV_IMAGE_PLANE_HPP
