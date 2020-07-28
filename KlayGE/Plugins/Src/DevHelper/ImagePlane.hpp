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

#include <KlayGE/DevHelper/DevHelper.hpp>

namespace KlayGE
{
	class TexMetadata;

	class ImagePlane final
	{
	public:
		bool Load(std::string_view name, TexMetadata const & metadata);
		void RgbToLum();
		void AlphaToLum();
		void BumpToNormal(float scale, float amplitude);
		void NormalToHeight(float min_z);
		void PrepareNormalCompression(ElementFormat normal_compression_format);
		void FormatConversion(ElementFormat format);
		ImagePlane ResizeTo(uint32_t width, uint32_t height, bool linear);

		uint32_t Width() const
		{
			return uncompressed_tex_->Width(0);
		}
		uint32_t Height() const
		{
			return uncompressed_tex_->Height(0);
		}
		TexturePtr const & UncompressedTex() const
		{
			return uncompressed_tex_;
		}
		TexturePtr const & CompressedTex() const
		{
			return compressed_tex_;
		}

	private:
		float RgbToLum(Color const & clr);

	private:
		TexturePtr uncompressed_tex_;
		TexturePtr compressed_tex_;
	};
}

#endif		// KLAYGE_TOOLS_IMAGE_CONV_IMAGE_PLANE_HPP
