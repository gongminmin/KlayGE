/**
 * @file NullRenderEngine.cpp
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

#include <KlayGE/KlayGE.hpp>

#include "NullTexture.hpp"

namespace KlayGE
{
	NullTexture::NullTexture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
		: Texture(type, sample_count, sample_quality, access_hint)
	{
	}

	NullTexture::~NullTexture()
	{
	}

	std::wstring const & NullTexture::Name() const
	{
		static const std::wstring name(L"Null Texture");
		return name;
	}

	uint32_t NullTexture::Width([[maybe_unused]] uint32_t level) const
	{
		return 0;
	}

	uint32_t NullTexture::Height([[maybe_unused]] uint32_t level) const
	{
		return 0;
	}

	uint32_t NullTexture::Depth([[maybe_unused]] uint32_t level) const
	{
		return 0;
	}

	void NullTexture::CopyToTexture([[maybe_unused]] Texture& target, [[maybe_unused]] TextureFilter filter)
	{
	}

	void NullTexture::CopyToSubTexture1D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_width,
		[[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] TextureFilter filter)
	{
	}

	void NullTexture::CopyToSubTexture2D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
		[[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height, [[maybe_unused]] uint32_t src_array_index,
		[[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
	{
	}

	void NullTexture::CopyToSubTexture3D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
		[[maybe_unused]] uint32_t dst_z_offset, [[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height,
		[[maybe_unused]] uint32_t dst_depth, [[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] uint32_t src_level,
		[[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset, [[maybe_unused]] uint32_t src_z_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] uint32_t src_depth,
		[[maybe_unused]] TextureFilter filter)
	{
	}

	void NullTexture::CopyToSubTextureCube([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] CubeFaces dst_face, [[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset,
		[[maybe_unused]] uint32_t dst_y_offset, [[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height,
		[[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] CubeFaces src_face, [[maybe_unused]] uint32_t src_level,
		[[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset, [[maybe_unused]] uint32_t src_width,
		[[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
	{
	}

	void NullTexture::Map1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t width, void*& data)
	{
		data = nullptr;
	}

	void NullTexture::Map2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t width,
		[[maybe_unused]] uint32_t height, void*& data, uint32_t& row_pitch)
	{
		data = nullptr;
		row_pitch = 0;
	}

	void NullTexture::Map3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t z_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth, void*& data,
		uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		data = nullptr;
		row_pitch = 0;
		slice_pitch = 0;
	}

	void NullTexture::MapCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] TextureMapAccess tma, [[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, void*& data, uint32_t& row_pitch)
	{
		data = nullptr;
		row_pitch = 0;
	}

	void NullTexture::Unmap1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
	}

	void NullTexture::Unmap2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
	}

	void NullTexture::Unmap3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
	}

	void NullTexture::UnmapCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face, [[maybe_unused]] uint32_t level)
	{
	}

	void NullTexture::CreateHWResource(
		[[maybe_unused]] std::span<ElementInitData const> init_data, [[maybe_unused]] float4 const* clear_value_hint)
	{
	}

	void NullTexture::DeleteHWResource()
	{
	}

	bool NullTexture::HWResourceReady() const
	{
		return true;
	}

	void NullTexture::UpdateSubresource1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t width, [[maybe_unused]] void const* data)
	{
	}

	void NullTexture::UpdateSubresource2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t width,
		[[maybe_unused]] uint32_t height, [[maybe_unused]] void const* data, [[maybe_unused]] uint32_t row_pitch)
	{
	}

	void NullTexture::UpdateSubresource3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t z_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth,
		[[maybe_unused]] void const* data, [[maybe_unused]] uint32_t row_pitch, [[maybe_unused]] uint32_t slice_pitch)
	{
	}

	void NullTexture::UpdateSubresourceCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face,
		[[maybe_unused]] uint32_t level, [[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] void const* data,
		[[maybe_unused]] uint32_t row_pitch)
	{
	}
}
