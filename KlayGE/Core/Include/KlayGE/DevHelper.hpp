/**
 * @file DevHelper.hpp
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

#ifndef KLAYGE_CORE_BASE_DEV_HELPER_HPP
#define KLAYGE_CORE_BASE_DEV_HELPER_HPP

#pragma once

#if KLAYGE_IS_DEV_PLATFORM

#include <KlayGE/PreDeclare.hpp>

#include <KFL/CXX17/string_view.hpp>
#include <KFL/DllLoader.hpp>
#include <KlayGE/Texture.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API DevHelper : boost::noncopyable
	{
	public:
		virtual ~DevHelper() noexcept;

		virtual RenderModelPtr ConvertModel(std::string_view input_name, std::string_view metadata_name, std::string_view output_name,
			RenderDeviceCaps const * caps) = 0;
		virtual TexturePtr ConvertTexture(std::string_view input_name, std::string_view metadata_name, std::string_view output_name,
			RenderDeviceCaps const * caps) = 0;
		virtual void GetImageInfo(std::string_view input_name, std::string_view metadata_name, RenderDeviceCaps const * caps,
			Texture::TextureType& type,
			uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
			ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch) = 0;
	};
}

#endif

#endif			// KLAYGE_CORE_BASE_DEV_HELPER_HPP
