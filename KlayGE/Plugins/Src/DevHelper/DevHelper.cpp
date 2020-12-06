/**
 * @file ToolCommon.cpp
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
#include <KFL/CXX17/filesystem.hpp>
#include <KlayGE/ResLoader.hpp>

#include <regex>
#include <string>

#include <KlayGE/DevHelper.hpp>
#include <KlayGE/DevHelper/MeshMetadata.hpp>
#include <KlayGE/DevHelper/MeshConverter.hpp>
#include <KlayGE/DevHelper/TexMetadata.hpp>
#include <KlayGE/DevHelper/TexConverter.hpp>

namespace KlayGE
{
	class DevHelperImp : public DevHelper
	{
	public:
		~DevHelperImp() override
		{
		}

		RenderModelPtr ConvertModel(std::string_view input_name, std::string_view metadata_name, std::string_view output_name,
			RenderDeviceCaps const * caps) override
		{
			KFL_UNUSED(caps);

			MeshMetadata metadata;
			if (!metadata_name.empty())
			{
				metadata.Load(metadata_name);
			}

			MeshConverter mc;
			auto model = mc.Load(input_name, metadata);

			FILESYSTEM_NS::path input_path(input_name.begin(), input_name.end());
			FILESYSTEM_NS::path output_path(output_name.begin(), output_name.end());
			if (output_path.parent_path() == input_path.parent_path())
			{
				output_path = FILESYSTEM_NS::path(ResLoader::Instance().Locate(input_name)).parent_path() / output_path.filename();
			}

			mc.Save(*model, output_path.string());

			return model;
		}

		TexturePtr ConvertTexture(std::string_view input_name, std::string_view metadata_name, std::string_view output_name,
			RenderDeviceCaps const * caps) override
		{
			auto metadata = this->LoadTexMetadata(metadata_name, caps);

			TexConverter tc;
			auto texture = tc.Load(input_name, metadata);

			FILESYSTEM_NS::path input_path(input_name.begin(), input_name.end());
			FILESYSTEM_NS::path output_path(output_name.begin(), output_name.end());
			if (output_path.parent_path() == input_path.parent_path())
			{
				output_path = FILESYSTEM_NS::path(ResLoader::Instance().Locate(input_name)).parent_path() / output_path.filename();
			}

			SaveTexture(texture, output_path.string());

			return texture;
		}
		
		void GetImageInfo(std::string_view input_name, std::string_view metadata_name, RenderDeviceCaps const * caps,
			Texture::TextureType& type,
			uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
			ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch) override
		{
			auto metadata = this->LoadTexMetadata(metadata_name, caps);

			TexConverter tc;
			tc.GetImageInfo(input_name, metadata,
				type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
		}

	private:
		TexMetadata LoadTexMetadata(std::string_view metadata_name, RenderDeviceCaps const * caps)
		{
			TexMetadata metadata;
			if (!metadata_name.empty())
			{
				metadata.Load(metadata_name);
			}
			if (caps)
			{
				metadata.DeviceDependentAdjustment(*caps);
			}

			return metadata;
		}
	};
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeDevHelper(std::unique_ptr<KlayGE::DevHelper>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::DevHelperImp>();
	}
}
