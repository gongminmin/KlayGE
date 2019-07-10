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

#include <KlayGE/NullRender/NullTexture.hpp>

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

	uint32_t NullTexture::Width(uint32_t level) const
	{
		KFL_UNUSED(level);
		return 0;
	}

	uint32_t NullTexture::Height(uint32_t level) const
	{
		KFL_UNUSED(level);
		return 0;
	}

	uint32_t NullTexture::Depth(uint32_t level) const
	{
		KFL_UNUSED(level);
		return 0;
	}

	void NullTexture::CopyToTexture(Texture& target)
	{
		KFL_UNUSED(target);
	}

	void NullTexture::CopyToSubTexture1D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_width);
	}

	void NullTexture::CopyToSubTexture2D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_y_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(dst_height);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_y_offset);
		KFL_UNUSED(src_width);
		KFL_UNUSED(src_height);
	}

	void NullTexture::CopyToSubTexture3D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
		uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_y_offset);
		KFL_UNUSED(dst_z_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(dst_height);
		KFL_UNUSED(dst_depth);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_y_offset);
		KFL_UNUSED(src_z_offset);
		KFL_UNUSED(src_width);
		KFL_UNUSED(src_height);
		KFL_UNUSED(src_depth);
	}

	void NullTexture::CopyToSubTextureCube(Texture& target,
		uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_face);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_y_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(dst_height);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_face);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_y_offset);
		KFL_UNUSED(src_width);
		KFL_UNUSED(src_height);
	}

	void NullTexture::BuildMipSubLevels()
	{
	}

	void NullTexture::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t width,
		void*& data)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(width);

		data = nullptr;
	}

	void NullTexture::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);

		data = nullptr;
		row_pitch = 0;
	}

	void NullTexture::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(z_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);

		data = nullptr;
		row_pitch = 0;
		slice_pitch = 0;
	}

	void NullTexture::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);

		data = nullptr;
		row_pitch = 0;
	}

	void NullTexture::Unmap1D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
	}

	void NullTexture::Unmap2D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
	}

	void NullTexture::Unmap3D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
	}

	void NullTexture::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
	}

	void NullTexture::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		KFL_UNUSED(init_data);
		KFL_UNUSED(clear_value_hint);
	}

	void NullTexture::DeleteHWResource()
	{
	}

	bool NullTexture::HWResourceReady() const
	{
		return true;
	}

	void NullTexture::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(data);
	}

	void NullTexture::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
	}

	void NullTexture::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(z_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
		KFL_UNUSED(slice_pitch);
	}

	void NullTexture::UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
	}
}
