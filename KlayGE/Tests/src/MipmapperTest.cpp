/**
 * @file MipmapperTest.cpp
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

#include <KlayGE/Mipmapper.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/DevHelper/TexConverter.hpp>
#include <KlayGE/DevHelper/TexMetadata.hpp>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

class MipmapperTest : public testing::Test
{
public:
	void SetUp() override
	{
		ResLoader::Instance().AddPath("../../Tests/media/Mipmap");
		ResLoader::Instance().AddPath("../../Tests/media/EncodeDecodeTex");
		ResLoader::Instance().AddPath("../../Tests/media/TexConverter");
	}

	void TestMipmapPoT2D(std::string_view input_name, TexMetadata const& input_metadata, TextureFilter filter,
		TexMetadata const& sanity_metadata, float threshold)
	{
		TexConverter tc;
		auto input_tex = tc.Load(input_name, input_metadata);
		EXPECT_TRUE(input_tex);

		uint32_t const array_size = input_tex->ArraySize();
		uint32_t const mip_levels = input_tex->NumMipMaps();

		{
			uint32_t const width = input_tex->Width(0);
			uint32_t const height = input_tex->Height(0);

			auto& rf = Context::Instance().RenderFactoryInstance();
			auto input_gpu_tex = rf.MakeTexture2D(
				width, height, mip_levels, array_size, input_tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered);
			for (uint32_t index = 0; index < array_size; ++index)
			{
				input_tex->CopyToSubTexture2D(
					*input_gpu_tex, index, 0, 0, 0, width, height, index, 0, 0, 0, width, height, TextureFilter::Point);
			}

			input_tex = input_gpu_tex;
		}

		Mipmapper mipmapper;
		mipmapper.BuildSubLevels(input_tex, filter);

		auto sanity_tex = tc.Load(input_name, sanity_metadata);
		EXPECT_TRUE(sanity_tex);

		for (uint32_t index = 0; index < array_size; ++index)
		{
			for (uint32_t mip = 0; mip < mip_levels; ++mip)
			{
				EXPECT_TRUE(Compare2D(*sanity_tex, index, mip, 0, 0, *input_tex, index, mip, 0, 0, input_tex->Width(mip),
					input_tex->Height(mip), threshold));
			}
		}
	}
};

TEST_F(MipmapperTest, MipmapPoT2DPoint)
{
	uint32_t const mip_levels = 8;

	TexMetadata input_metadata("2D.kmeta", false);
	input_metadata.ForceSRGB(false);
	input_metadata.NumMipmaps(mip_levels);
	input_metadata.AutoGenMipmap(false);
	input_metadata.LinearMipmap(false);

	TexMetadata sanity_metadata("2D.kmeta", false);
	sanity_metadata.ForceSRGB(false);
	sanity_metadata.NumMipmaps(mip_levels);
	sanity_metadata.AutoGenMipmap(true);
	sanity_metadata.LinearMipmap(false);
	
	TestMipmapPoT2D("lion.jpg", input_metadata, TextureFilter::Point, sanity_metadata, 2.0f / 255);
}

TEST_F(MipmapperTest, MipmapPoT2DLinear)
{
	uint32_t const mip_levels = 8;

	TexMetadata input_metadata("2D.kmeta", false);
	input_metadata.ForceSRGB(false);
	input_metadata.NumMipmaps(mip_levels);
	input_metadata.AutoGenMipmap(false);
	input_metadata.LinearMipmap(true);

	TexMetadata sanity_metadata("2D.kmeta", false);
	sanity_metadata.ForceSRGB(false);
	sanity_metadata.NumMipmaps(mip_levels);
	sanity_metadata.AutoGenMipmap(true);
	sanity_metadata.LinearMipmap(true);

	TestMipmapPoT2D("lion.jpg", input_metadata, TextureFilter::Linear, sanity_metadata, 3.0f / 255);
}

TEST_F(MipmapperTest, MipmapPoT2DArrayPoint)
{
	uint32_t const mip_levels = 9;

	TexMetadata input_metadata("2DArray.kmeta", false);
	input_metadata.ForceSRGB(false);
	input_metadata.NumMipmaps(mip_levels);
	input_metadata.AutoGenMipmap(false);
	input_metadata.LinearMipmap(false);

	TexMetadata sanity_metadata("2DArray.kmeta", false);
	sanity_metadata.ForceSRGB(false);
	sanity_metadata.NumMipmaps(mip_levels);
	sanity_metadata.AutoGenMipmap(true);
	sanity_metadata.LinearMipmap(false);

	TestMipmapPoT2D("lion.jpg", input_metadata, TextureFilter::Point, sanity_metadata, 2.0f / 255);
}

TEST_F(MipmapperTest, MipmapPoT2DArrayLinear)
{
	uint32_t const mip_levels = 9;

	TexMetadata input_metadata("2DArray.kmeta", false);
	input_metadata.ForceSRGB(false);
	input_metadata.NumMipmaps(mip_levels);
	input_metadata.AutoGenMipmap(false);
	input_metadata.LinearMipmap(true);

	TexMetadata sanity_metadata("2DArray.kmeta", false);
	sanity_metadata.ForceSRGB(false);
	sanity_metadata.NumMipmaps(mip_levels);
	sanity_metadata.AutoGenMipmap(true);
	sanity_metadata.LinearMipmap(true);

	TestMipmapPoT2D("lion.jpg", input_metadata, TextureFilter::Linear, sanity_metadata, 3.0f / 255);
}

TEST_F(MipmapperTest, MipmapPoTSRGB2DPoint)
{
	uint32_t const mip_levels = 8;

	TexMetadata input_metadata("2D.kmeta", false);
	input_metadata.ForceSRGB(true);
	input_metadata.NumMipmaps(mip_levels);
	input_metadata.AutoGenMipmap(false);
	input_metadata.LinearMipmap(false);

	TexMetadata sanity_metadata("2D.kmeta", false);
	sanity_metadata.ForceSRGB(true);
	sanity_metadata.NumMipmaps(mip_levels);
	sanity_metadata.AutoGenMipmap(true);
	sanity_metadata.LinearMipmap(false);

	TestMipmapPoT2D("lion.jpg", input_metadata, TextureFilter::Point, sanity_metadata, 4.0f / 255);
}

TEST_F(MipmapperTest, MipmapPoTSRGB2DLinear)
{
	uint32_t const mip_levels = 8;

	TexMetadata input_metadata("2D.kmeta", false);
	input_metadata.ForceSRGB(true);
	input_metadata.NumMipmaps(mip_levels);
	input_metadata.AutoGenMipmap(false);
	input_metadata.LinearMipmap(true);

	TexMetadata sanity_metadata("2D.kmeta", false);
	sanity_metadata.ForceSRGB(true);
	sanity_metadata.NumMipmaps(mip_levels);
	sanity_metadata.AutoGenMipmap(true);
	sanity_metadata.LinearMipmap(true);

	TestMipmapPoT2D("lion.jpg", input_metadata, TextureFilter::Linear, sanity_metadata, 4.0f / 255);
}
