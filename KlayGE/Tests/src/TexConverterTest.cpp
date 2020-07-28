/**
 * @file TexConverterTest.cpp
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KlayGE/DevHelper/TexConverter.hpp>
#include <KlayGE/DevHelper/TexMetadata.hpp>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

class TexConverterTest : public testing::Test
{
public:
	void SetUp() override
	{
		ResLoader::Instance().AddPath("../../Tests/media/TexConverter");
	}

	void RunTest(std::string_view input_name, std::string_view metadata_name, std::string_view sanity_name, float tolerance)
	{
		TexMetadata metadata(metadata_name, false);

		TexConverter tc;
		auto target = tc.Load(input_name, metadata);
		EXPECT_TRUE(target);

		auto target_sanity = SyncLoadTexture(sanity_name, EAH_CPU_Read);

		EXPECT_EQ(target->NumMipMaps(), target_sanity->NumMipMaps());
		EXPECT_EQ(target->ArraySize(), target_sanity->ArraySize());
		EXPECT_EQ(target->Type(), target_sanity->Type());
		EXPECT_EQ(target->Format(), target_sanity->Format());
		for (uint32_t i = 0; i < target->ArraySize(); ++i)
		{
			for (uint32_t m = 0; m < target->NumMipMaps(); ++m)
			{
				EXPECT_EQ(target->Width(m), target_sanity->Width(m));
				EXPECT_EQ(target->Height(m), target_sanity->Height(m));
				EXPECT_EQ(target->Depth(m), target_sanity->Depth(m));

				EXPECT_TRUE(Compare2D(*target_sanity, i, m, 0, 0,
					*target, i, m, 0, 0,
					target->Width(m), target->Height(m), tolerance));
			}
		}
	}
};

TEST_F(TexConverterTest, NoMetadata)
{
	RunTest("lion.jpg", "", "lion_passthrough.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, PassThrough)
{
	RunTest("lion.jpg", "lion_passthrough.kmeta", "lion_passthrough.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Compression)
{
	RunTest("lion.jpg", "lion_bc1.kmeta", "lion_bc1.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, CompressionSRGB)
{
	RunTest("lion.jpg", "lion_bc1_srgb.kmeta", "lion_bc1_srgb.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, CompressionBC7SRGB)
{
	// TODO: Figure out why it's so large
	RunTest("lion.jpg", "lion_bc7_srgb.kmeta", "lion_bc7_srgb.dds", 32.0f / 255);
}

TEST_F(TexConverterTest, Mip)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 32.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	RunTest("lion.jpg", "lion_mip.kmeta", "lion_mip.dds", tolerance);
}

TEST_F(TexConverterTest, Channel)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 64.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	RunTest("lion.jpg", "lion_channel.kmeta", "lion_channel.dds", tolerance);
}

TEST_F(TexConverterTest, Array)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 32.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	RunTest("lion.jpg", "array.kmeta", "array.dds", tolerance);
}

TEST_F(TexConverterTest, ArrayMip)
{
	RunTest("lion.jpg", "array_mip.kmeta", "array_mip.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, DDSSRGB)
{
	RunTest("lion_bc1.dds", "lion_bc1_srgb.kmeta", "lion_bc1_srgb.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, DDSChannel)
{
	RunTest("lion_bc1.dds", "lion_channel.kmeta", "lion_bc1_channel.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, NormalCompressionBC5)
{
	RunTest("lion_ddn.jpg", "lion_ddn_bc5.kmeta", "lion_ddn_bc5.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, NormalCompressionBC3)
{
	RunTest("lion_ddn.jpg", "lion_ddn_bc3.kmeta", "lion_ddn_bc3.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, NormalCompressionGR)
{
	RunTest("lion_ddn.jpg", "lion_ddn_gr.kmeta", "lion_ddn_gr.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Bump2Normal)
{
	RunTest("background.jpg", "background_bump2normal.kmeta", "background_normal.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Bump2Normal_0_4)
{
	RunTest("background.jpg", "background_bump2normal_0_4.kmeta", "background_normal_0_4.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Bump2Occlusion)
{
	RunTest("background.jpg", "background_bump2occlusion.kmeta", "background_occlusion.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Bump2Occlusion_0_6)
{
	RunTest("background.jpg", "background_bump2occlusion_0_6.kmeta", "background_occlusion_0_6.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Rgb2Lum)
{
	RunTest("background.jpg", "background_rgb2lum.kmeta", "background_lum.dds", 1.0f / 255);
}

TEST_F(TexConverterTest, Normal2Height)
{
	RunTest("lion_ddn.jpg", "lion_ddn_normal2height.kmeta", "lion_height.dds", 1.0f / 255);
}
