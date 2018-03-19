/**
 * @file RenderToTextureTest.cpp
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
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>

#include <vector>
#include <string>
#include <iostream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

uint32_t const WIDTH = 256;
uint32_t const HEIGHT = 256;

class RenderToTextureTest : public testing::Test
{
public:
	void SetUp() override
	{
		auto const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		render_to_msaa_texture_support_ = caps.render_to_msaa_texture_support;
	}

	void TestRenderToTexture(uint32_t sample_count, float tolerance)
	{
		if ((sample_count > 1) && !render_to_msaa_texture_support_)
		{
			return;
		}

		ResLoader::Instance().AddPath("../../Tests/media/RenderToTexture");

		auto target = RenderToTexture(WIDTH, HEIGHT, EF_ABGR8, sample_count);
		auto target_resolved = HwResolveToTexture(target);

		std::string sanity_name = "RenderToTexture";
		if (sample_count > 1)
		{
			sanity_name += "MS" + boost::lexical_cast<std::string>(sample_count);
		}

		auto target_sanity = SyncLoadTexture(sanity_name + "Test.dds", EAH_CPU_Read);

		EXPECT_TRUE(Compare2D(*target_sanity, 0, 0, 0, 0,
			*target_resolved, 0, 0, 0, 0,
			WIDTH, HEIGHT, tolerance));
	}

	void TestResolveToTexture(uint32_t sample_count, float tolerance)
	{
		if ((sample_count > 1) && !render_to_msaa_texture_support_)
		{
			return;
		}

		ResLoader::Instance().AddPath("../../Tests/media/RenderToTexture");

		auto target = RenderToTexture(WIDTH, HEIGHT, EF_ABGR8, sample_count);
		auto target_resolved = ManualResolveToTexture(target);

		std::string sanity_name = "RenderToTexture";
		if (sample_count > 1)
		{
			sanity_name += "MS" + boost::lexical_cast<std::string>(sample_count);
		}

		auto target_sanity = SyncLoadTexture(sanity_name + "Test.dds", EAH_CPU_Read);

		EXPECT_TRUE(Compare2D(*target_sanity, 0, 0, 0, 0,
			*target_resolved, 0, 0, 0, 0,
			WIDTH, HEIGHT, tolerance));
	}

	void TestCopyToTexture(uint32_t sample_count, float tolerance)
	{
		bool skip = false;

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const & caps = re.DeviceCaps();
		if (sample_count > 1)
		{
			skip = !caps.render_to_msaa_texture_support;
		}

		if (skip)
		{
			return;
		}

		ResLoader::Instance().AddPath("../../Tests/media/RenderToTexture");

		auto target = RenderToTexture(WIDTH, HEIGHT, EF_ABGR8, sample_count);
		auto target_copied = CopyToTexture(target);
		auto target_resolved = ManualResolveToTexture(target_copied);

		std::string sanity_name = "RenderToTexture";
		if (sample_count > 1)
		{
			sanity_name += "MS" + boost::lexical_cast<std::string>(sample_count);
		}

		auto target_sanity = SyncLoadTexture(sanity_name + "Test.dds", EAH_CPU_Read);

		EXPECT_TRUE(Compare2D(*target_sanity, 0, 0, 0, 0,
			*target_resolved, 0, 0, 0, 0,
			WIDTH, HEIGHT, tolerance));
	}

private:
	TexturePtr RenderToTexture(uint32_t width, uint32_t height, ElementFormat format, uint32_t sample_count)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		float const flipping = re.RequiresFlipping() ? -1.0f : 1.0f;

		auto target = rf.MakeTexture2D(width, height, 1, 1, format, sample_count, 0, EAH_GPU_Read | EAH_GPU_Write);
		auto target_ds = rf.MakeTexture2D(width, height, 1, 1, EF_D24S8, sample_count, 0, EAH_GPU_Write);
		auto fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*target, 0, 1, 0));
		fb->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*target_ds, 0, 1, 0));

		auto effect = SyncLoadRenderEffect("RenderToTexture/RenderToTextureTest.fxml");
		auto tech = effect->TechniqueByName("RenderToTexture");

		float2 const vertices[] =
		{
			float2(+0.0f, -0.5f * flipping),
			float2(+0.5f, +0.5f * flipping),
			float2(-0.5f, +0.5f * flipping)
		};

		auto rl = rf.MakeRenderLayout();
		rl->TopologyType(RenderLayout::TT_TriangleList);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rl->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_GR32F));

		re.BindFrameBuffer(fb);
		fb->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1, 0);
		re.Render(*effect, *tech, *rl);
		re.BindFrameBuffer(FrameBufferPtr());

		return target;
	}

	TexturePtr HwResolveToTexture(TexturePtr const & source)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		auto target_resolved = rf.MakeTexture2D(source->Width(0), source->Height(0), 1, 1, source->Format(), 1, 0,
			EAH_GPU_Read | EAH_GPU_Write);
		source->CopyToTexture(*target_resolved);

		return target_resolved;
	}

	TexturePtr ManualResolveToTexture(TexturePtr const & source)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		float const flipping = re.RequiresFlipping() ? -1.0f : 1.0f;

		auto target = rf.MakeTexture2D(source->Width(0), source->Height(0), 1, 1, source->Format(),
			1, 0, EAH_GPU_Read | EAH_GPU_Write);
		auto fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*target, 0, 1, 0));

		auto effect = SyncLoadRenderEffect("RenderToTexture/RenderToTextureTest.fxml");

		uint32_t const sample_count = source->SampleCount();
		auto tech = effect->TechniqueByName("ResolveToTextureMS" + boost::lexical_cast<std::string>(sample_count));
		*(effect->ParameterByName("src_ms")) = source;

		float2 const vertices[] =
		{
			float2(-1.0f, -1.0f * flipping),
			float2(+1.0f, -1.0f * flipping),
			float2(-1.0f, +1.0f * flipping),
			float2(+1.0f, +1.0f * flipping)
		};

		auto rl = rf.MakeRenderLayout();
		rl->TopologyType(RenderLayout::TT_TriangleStrip);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rl->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_GR32F));

		re.BindFrameBuffer(fb);
		re.Render(*effect, *tech, *rl);
		re.BindFrameBuffer(FrameBufferPtr());

		return target;
	}

	TexturePtr CopyToTexture(TexturePtr const & source)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		float const flipping = re.RequiresFlipping() ? -1.0f : 1.0f;

		auto target = rf.MakeTexture2D(source->Width(0), source->Height(0), 1, 1, source->Format(),
			source->SampleCount(), source->SampleQuality(), EAH_GPU_Read | EAH_GPU_Write);
		auto fb = rf.MakeFrameBuffer();
		fb->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*target, 0, 1, 0));

		auto effect = SyncLoadRenderEffect("RenderToTexture/RenderToTextureTest.fxml");

		uint32_t const sample_count = source->SampleCount();
		auto tech = effect->TechniqueByName("CopyToTextureMS" + boost::lexical_cast<std::string>(sample_count));
		*(effect->ParameterByName("src_ms")) = source;

		float2 const vertices[] =
		{
			float2(-1.0f, -1.0f * flipping),
			float2(+1.0f, -1.0f * flipping),
			float2(-1.0f, +1.0f * flipping),
			float2(+1.0f, +1.0f * flipping)
		};

		auto rl = rf.MakeRenderLayout();
		rl->TopologyType(RenderLayout::TT_TriangleStrip);

		auto vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices);
		rl->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_GR32F));

		re.BindFrameBuffer(fb);
		re.Render(*effect, *tech, *rl);
		re.BindFrameBuffer(FrameBufferPtr());

		return target;
	}

private:
	bool render_to_msaa_texture_support_;
};

TEST_F(RenderToTextureTest, RenderToTexture)
{
	TestRenderToTexture(1, 1.0f / 255);
}

TEST_F(RenderToTextureTest, RenderToTextureMS2)
{
	TestRenderToTexture(2, 1.0f / 255);
}

TEST_F(RenderToTextureTest, RenderToTextureMS4)
{
	TestRenderToTexture(4, 1.0f / 255);
}

TEST_F(RenderToTextureTest, RenderToTextureMS8)
{
	TestRenderToTexture(8, 1.0f / 255);
}

TEST_F(RenderToTextureTest, ResolveToTextureMS2)
{
	TestResolveToTexture(2, 1.0f / 255);
}

TEST_F(RenderToTextureTest, ResolveToTextureMS4)
{
	TestResolveToTexture(4, 1.0f / 255);
}

TEST_F(RenderToTextureTest, ResolveToTextureMS8)
{
	TestResolveToTexture(8, 1.0f / 255);
}

TEST_F(RenderToTextureTest, CopyToTextureMS2)
{
	TestCopyToTexture(2, 1.0f / 255);
}

TEST_F(RenderToTextureTest, CopyToTextureMS4)
{
	TestCopyToTexture(4, 1.0f / 255);
}

TEST_F(RenderToTextureTest, CopyToTextureMS8)
{
	TestCopyToTexture(8, 1.0f / 255);
}
