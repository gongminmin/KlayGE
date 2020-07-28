/**
 * @file TextureTest.cpp
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
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>

#include <vector>
#include <string>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

class TextureTest : public testing::Test
{
public:
	void SetUp() override
	{
		ResLoader::Instance().AddPath("../../Tests/media/EncodeDecodeTex");
		ResLoader::Instance().AddPath("../../Tests/media/Texture");
	}

	void TestCopyToTexture(std::string_view input_name, std::string_view sanity_name, float scale, bool gpu_tex, float tolerance)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		auto target = SyncLoadTexture(input_name, gpu_tex ? EAH_GPU_Read : EAH_CPU_Read);
		auto target_scaled = rf.MakeTexture2D(
			static_cast<uint32_t>(target->Width(0) * scale), static_cast<uint32_t>(target->Height(0) * scale),
			1, 1, target->Format(), 1, 0, EAH_CPU_Read);
		target->CopyToTexture(*target_scaled, TextureFilter::Linear);

		auto target_sanity = SyncLoadTexture(sanity_name, EAH_CPU_Read);

		EXPECT_TRUE(Compare2D(*target_sanity, 0, 0, 0, 0,
			*target_scaled, 0, 0, 0, 0,
			target_sanity->Width(0), target_sanity->Height(0), tolerance));

		ResLoader::Instance().Unload(target);
	}

	void TestCopyToSubTexture(std::string_view input_name, std::string_view sanity_name, bool gpu_tex, float tolerance)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		auto target = SyncLoadTexture(input_name, gpu_tex ? EAH_GPU_Read : EAH_CPU_Read);
		auto target_scaled = rf.MakeTexture2D(target->Width(0) / 4, target->Height(0) / 4, 1, 1, target->Format(), 1, 0, EAH_CPU_Read);
		target->CopyToTexture(*target_scaled, TextureFilter::Linear);
		target_scaled->CopyToSubTexture2D(*target, 0, 0, 24, 48, 20, 32, 0, 0, 4, 8, 24, 20, TextureFilter::Linear);

		auto target_sanity = SyncLoadTexture(sanity_name, EAH_CPU_Read);

		EXPECT_TRUE(Compare2D(*target_sanity, 0, 0, 0, 0,
			*target, 0, 0, 0, 0,
			target_sanity->Width(0), target_sanity->Height(0), tolerance));

		ResLoader::Instance().Unload(target);
	}

	void TestUpdateSubTexture(std::string_view input_name, std::string_view sanity_name, bool gpu_tex, float tolerance)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		auto target = SyncLoadTexture(input_name, gpu_tex ? EAH_GPU_Read : EAH_CPU_Read);
		auto target_scaled = rf.MakeTexture2D(target->Width(0) / 4, target->Height(0) / 4, 1, 1, target->Format(), 1, 0, EAH_CPU_Read);
		target->CopyToTexture(*target_scaled, TextureFilter::Linear);

		auto target_cropped = rf.MakeTexture2D(20, 32, 1, 1, target->Format(), 1, 0, EAH_CPU_Read);
		target_scaled->CopyToSubTexture2D(
			*target_cropped, 0, 0, 0, 0, target_cropped->Width(0), target_cropped->Height(0), 0, 0, 4, 8, 24, 20, TextureFilter::Linear);

		{
			Texture::Mapper mapper(*target_cropped, 0, 0, TMA_Read_Only, 0, 0, target_cropped->Width(0), target_cropped->Height(0));

			target->UpdateSubresource2D(0, 0, 24, 48, target_cropped->Width(0), target_cropped->Height(0),
				mapper.Pointer<void>(), mapper.RowPitch());
		}

		auto target_sanity = SyncLoadTexture(sanity_name, EAH_CPU_Read);

		EXPECT_TRUE(Compare2D(*target_sanity, 0, 0, 0, 0,
			*target, 0, 0, 0, 0,
			target_sanity->Width(0), target_sanity->Height(0), tolerance));

		ResLoader::Instance().Unload(target);
	}
};

TEST_F(TextureTest, CopyToFullGPUTexture)
{
	TestCopyToTexture("Lenna.dds", "Lenna.dds", 1.0f, true, 1.0f / 255);
}

TEST_F(TextureTest, CopyToFullCompressedGPUTexture)
{
	TestCopyToTexture("Lenna_bc1.dds", "Lenna_bc1.dds", 1.0f, true, 1.0f / 255);
}

TEST_F(TextureTest, CopyToFullCPUTexture)
{
	TestCopyToTexture("Lenna.dds", "Lenna.dds", 1.0f, false, 1.0f / 255);
}

TEST_F(TextureTest, CopyToFullCompressedCPUTexture)
{
	TestCopyToTexture("Lenna_bc1.dds", "Lenna_bc1.dds", 1.0f, false, 1.0f / 255);
}

TEST_F(TextureTest, CopyToScaledGPUTexture)
{
	TestCopyToTexture("Lenna.dds", "Lenna_quarter.dds", 0.25f, true, 1.0f / 255);
}

TEST_F(TextureTest, CopyToScaledCompressedGPUTexture)
{
	TestCopyToTexture("Lenna_bc1.dds", "Lenna_quarter_bc1.dds", 0.25f, true, 1.0f / 255);
}

TEST_F(TextureTest, CopyToScaledCPUTexture)
{
	TestCopyToTexture("Lenna.dds", "Lenna_quarter.dds", 0.25f, false, 1.0f / 255);
}

TEST_F(TextureTest, CopyToScaledCompressedCPUTexture)
{
	TestCopyToTexture("Lenna_bc1.dds", "Lenna_quarter_bc1.dds", 0.25f, false, 1.0f / 255);
}

TEST_F(TextureTest, CopyToGPUSubTexture)
{
	TestCopyToSubTexture("Lenna.dds", "Lenna_SubTexture.dds", true, 1.0f / 255);
}

TEST_F(TextureTest, CopyToCompressedGPUSubTexture)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 40.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	TestCopyToSubTexture("Lenna_bc1.dds", "Lenna_SubTexture_bc1.dds", true, tolerance);
}

TEST_F(TextureTest, CopyToCPUSubTexture)
{
	TestCopyToSubTexture("Lenna.dds", "Lenna_SubTexture.dds", false, 1.0f / 255);
}

TEST_F(TextureTest, CopyToCompressedCPUSubTexture)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 40.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	TestCopyToSubTexture("Lenna_bc1.dds", "Lenna_SubTexture_bc1.dds", false, tolerance);
}

TEST_F(TextureTest, UpdateGPUSubTexture)
{
	TestUpdateSubTexture("Lenna.dds", "Lenna_SubTexture.dds", true, 1.0f / 255);
}

TEST_F(TextureTest, UpdateCompressedGPUSubTexture)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 40.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	TestUpdateSubTexture("Lenna_bc1.dds", "Lenna_SubTexture_bc1.dds", true, tolerance);
}

TEST_F(TextureTest, UpdateCPUSubTexture)
{
	TestUpdateSubTexture("Lenna.dds", "Lenna_SubTexture.dds", false, 1.0f / 255);
}

TEST_F(TextureTest, UpdateCompressedCPUSubTexture)
{
	// TODO: Figure out why it's so large on debug mode
#ifdef KLAYGE_DEBUG
	float const tolerance = 40.0f / 255;
#else
	float const tolerance = 1.0f / 255;
#endif
	TestUpdateSubTexture("Lenna_bc1.dds", "Lenna_SubTexture_bc1.dds", false, tolerance);
}
