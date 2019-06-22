/**
 * @file UavOutputTest.cpp
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
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/ResLoader.hpp>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

class UavOutputTest : public testing::Test
{
public:
	void SetUp() override
	{
		auto const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		uav_output_support_ = caps.max_simultaneous_uavs > 0;
		uavs_at_every_stage_support_ = caps.uavs_at_every_stage_support;
	}

	void TestRasterizeToUav(float tolerance)
	{
		if (!uav_output_support_)
		{
			return;
		}

		auto effect = SyncLoadRenderEffect("UavOutput/UavOutputTest.fxml");
		auto tech = effect->TechniqueByName("RasterizeToUav");

		uint32_t const width = 32;
		uint32_t const height = 32;
		uint32_t const num_vertices = width * height;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		auto out_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured | EAH_Counter,
			num_vertices * sizeof(float4), nullptr, sizeof(float4));
		auto out_uav = rf.MakeBufferUav(out_vb, EF_ABGR32F);

		auto fb = rf.MakeFrameBuffer();
		fb->Attach(0, out_uav);
		fb->Viewport()->Width(width);
		fb->Viewport()->Height(height);

		re.BindFrameBuffer(fb);

		*effect->ParameterByName("rw_output_buffer") = out_uav;
		*effect->ParameterByName("frame_width") = static_cast<int32_t>(width);
		re.Render(*effect, *tech, *re.PostProcessRenderLayout());

		std::vector<float4> sanity_data(num_vertices);
		for (uint32_t y = 0; y < height; ++y)
		{
			float const tc_y = (y + 0.5f) / height;
			for (uint32_t x = 0; x < width; ++x)
			{
				float const tc_x = (x + 0.5f) / width;
				sanity_data[y * width + x] = float4(tc_x, tc_y, tc_x, tc_y);
			}
		}

		auto sanity_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, num_vertices * sizeof(float4), sanity_data.data());

		EXPECT_TRUE(CompareBuffer(*sanity_vb, 0, *out_vb, 0, num_vertices * 4, tolerance));

		re.BindFrameBuffer(FrameBufferPtr());
	}

	void TestRasterizeToCs(float tolerance)
	{
		if (!uav_output_support_)
		{
			return;
		}

		auto effect = SyncLoadRenderEffect("UavOutput/UavOutputTest.fxml");
		auto ps_tech = effect->TechniqueByName("RasterizeToCounterUav");
		auto cs_tech = effect->TechniqueByName("UavToCs");

		uint32_t const width = 32;
		uint32_t const height = 32;
		uint32_t const num_vertices = width * height;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		auto ps_out_vb =
			rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured | EAH_Counter,
				num_vertices * sizeof(float4), nullptr, sizeof(float4));
		auto ps_out_srv = rf.MakeBufferSrv(ps_out_vb, EF_ABGR32F);
		auto ps_out_uav = rf.MakeBufferUav(ps_out_vb, EF_ABGR32F);

		auto fb = rf.MakeFrameBuffer();
		fb->Attach(0, ps_out_uav);
		fb->Viewport()->Width(width);
		fb->Viewport()->Height(height);

		re.BindFrameBuffer(fb);

		*effect->ParameterByName("frame_width") = static_cast<int32_t>(width);
		re.Render(*effect, *ps_tech, *re.PostProcessRenderLayout());

		auto cs_out_vb =
			rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured | EAH_Counter,
				num_vertices * sizeof(float4), nullptr, sizeof(float4));
		auto cs_out_uav = rf.MakeBufferUav(cs_out_vb, EF_ABGR32F);

		re.BindFrameBuffer(FrameBufferPtr());
		*effect->ParameterByName("input_buffer") = ps_out_srv;
		*effect->ParameterByName("rw_output_buffer") = cs_out_uav;
		re.Dispatch(*effect, *cs_tech, width, height, 1);

		std::vector<float4> sanity_data(num_vertices);
		{
			auto ps_out_cpu_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, ps_out_vb->Size(), nullptr);
			ps_out_vb->CopyToBuffer(*ps_out_cpu_vb);

			GraphicsBuffer::Mapper mapper(*ps_out_cpu_vb, BA_Read_Only);
			float4* ps_out_data = mapper.Pointer<float4>();

			for (uint32_t i = 0; i < num_vertices; ++i)
			{
				sanity_data[i] = ps_out_data[i] * float4(1, 2, 3, 4);
			}
		}

		auto sanity_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, num_vertices * sizeof(float4), sanity_data.data());

		EXPECT_TRUE(CompareBuffer(*sanity_vb, 0, *cs_out_vb, 0, num_vertices * 4, tolerance));
	}

	void TestRasterizeCounter(float tolerance)
	{
		if (!uav_output_support_)
		{
			return;
		}

		auto effect = SyncLoadRenderEffect("UavOutput/UavOutputTest.fxml");
		auto ps_tech = effect->TechniqueByName("RasterizeToCounterUav");
		auto cs_tech = effect->TechniqueByName("CounterUavToCs");

		uint32_t const width = 32;
		uint32_t const height = 32;
		uint32_t const num_vertices = width * height;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();

		auto ps_out_vb =
			rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered | EAH_GPU_Structured | EAH_Counter,
				num_vertices * 2 * sizeof(float4), nullptr, sizeof(float4));
		auto ps_out_srv = rf.MakeBufferSrv(ps_out_vb, EF_ABGR32F);
		auto ps_out_uav = rf.MakeBufferUav(ps_out_vb, EF_ABGR32F);
		ps_out_uav->InitCount(static_cast<uint32_t>(-1));

		auto fb = rf.MakeFrameBuffer();
		fb->Attach(0, ps_out_uav);
		fb->Viewport()->Width(width);
		fb->Viewport()->Height(height);

		re.BindFrameBuffer(fb);

		re.Render(*effect, *ps_tech, *re.PostProcessRenderLayout());

		std::vector<float4> sanity_data(num_vertices * 2);
		{
			auto ps_out_cpu_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, ps_out_vb->Size() / 2, nullptr);
			ps_out_vb->CopyToSubBuffer(*ps_out_cpu_vb, 0, 0, ps_out_vb->Size() / 2);

			GraphicsBuffer::Mapper mapper(*ps_out_cpu_vb, BA_Read_Only);
			float4* ps_out_data = mapper.Pointer<float4>();

			for (uint32_t i = 0; i < num_vertices; ++i)
			{
				sanity_data[i] = ps_out_data[i];
			}
			for (uint32_t i = 0; i < num_vertices; ++i)
			{
				sanity_data[i + num_vertices] = ps_out_data[i] * float4(1, 2, 3, 4);
			}
		}

		re.BindFrameBuffer(FrameBufferPtr());
		*effect->ParameterByName("rw_output_buffer") = ps_out_uav;
		*effect->ParameterByName("num_vertices") = static_cast<int32_t>(num_vertices);
		re.Dispatch(*effect, *cs_tech, width, height, 1);

		auto sanity_vb = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, num_vertices * 2 * sizeof(float4), sanity_data.data());

		EXPECT_TRUE(CompareBuffer(*sanity_vb, 0, *ps_out_vb, 0, num_vertices * 2 * 4, tolerance));
	}

private:
	bool uav_output_support_;
	bool uavs_at_every_stage_support_;
};

TEST_F(UavOutputTest, RasterizeToUav)
{
	TestRasterizeToUav(1.0f / 255);
}

TEST_F(UavOutputTest, RasterizeToCs)
{
	TestRasterizeToCs(1.0f / 255);
}

TEST_F(UavOutputTest, RasterizeCounter)
{
	TestRasterizeCounter(1.0f / 255);
}

