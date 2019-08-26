/**
 * @file StreamOutputTest.cpp
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
#include <KlayGE/Query.hpp>
#include <KlayGE/Texture.hpp>

#include <random>
#include <vector>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

class StreamOutputTest : public testing::Test
{
public:
	void SetUp() override
	{
		auto const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		draw_indirect_support_ = caps.draw_indirect_support;
		uavs_at_every_stage_support_ = caps.uavs_at_every_stage_support;
		gs_support_ = caps.gs_support;
	}

	void TestCopyBuffer(float tolerance)
	{
		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(-100, 100);

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto tech = effect->TechniqueByName("CopyBuffer");

		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto vb_in = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);
		auto vb_out = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_in->Size(), nullptr);

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->BindVertexStream(vb_in, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		rl_out->BindVertexStream(vb_out, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto& re = rf.RenderEngineInstance();
		
		for (uint32_t i = 0; i < 10; ++ i)
		{
			{
				GraphicsBuffer::Mapper mapper(*vb_in, BA_Write_Only);
				float4* input_data = mapper.Pointer<float4>();
				for (uint32_t j = 0; j < num_vertices; ++ j)
				{
					float const x = static_cast<float>(dis(gen));
					float const y = static_cast<float>(dis(gen));
					float const z = static_cast<float>(dis(gen));
					float const w = static_cast<float>(dis(gen));

					input_data[j] = float4(x, y, z, w);
				}
			}

			re.BindSOBuffers(rl_out);
			re.Render(*effect, *tech, *rl_in);
			re.BindSOBuffers(RenderLayoutPtr());

			EXPECT_TRUE(CompareBuffer(*vb_in, 0,
				*vb_out, 0,
				num_vertices * 4, tolerance));
		}
	}

	void TestVertexIDToBuffer(float tolerance)
	{
		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto tech = effect->TechniqueByName("VertexIDToBuffer");

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->NumVertices(num_vertices);

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		auto vb_out = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, num_vertices * sizeof(float4), nullptr);
		rl_out->BindVertexStream(vb_out, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto& re = rf.RenderEngineInstance();
		re.BindSOBuffers(rl_out);
		re.Render(*effect, *tech, *rl_in);
		re.BindSOBuffers(RenderLayoutPtr());

		std::vector<float4> sanity_data(num_vertices);
		for (uint32_t i = 0; i < num_vertices; ++ i)
		{
			sanity_data[i] = float4(i + 0.0f, i + 0.25f, i + 0.5f, i + 0.75f);
		}

		auto vb_sanity = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, num_vertices * sizeof(float4), sanity_data.data());

		EXPECT_TRUE(CompareBuffer(*vb_sanity, 0,
			*vb_out, 0,
			static_cast<uint32_t>(sanity_data.size() * 4), tolerance));
	}

	void TestConditionalCopyBuffer(float tolerance)
	{
		if (!gs_support_)
		{
			return;
		}

		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(-100, 100);

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto tech = effect->TechniqueByName("ConditionalCopyBuffer");

		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto vb_in = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);
		auto vb_sanity = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read | EAH_CPU_Write, vb_in->Size(), nullptr);

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->BindVertexStream(vb_in, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		auto vb_out = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_in->Size(), nullptr);
		rl_out->BindVertexStream(vb_out, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto query = rf.MakeSOStatisticsQuery();

		auto& re = rf.RenderEngineInstance();

		for (uint32_t i = 0; i < 10; ++ i)
		{
			uint32_t sanity_size = 0;
			{
				GraphicsBuffer::Mapper input_mapper(*vb_in, BA_Write_Only);
				float4* input_data = input_mapper.Pointer<float4>();
				GraphicsBuffer::Mapper sanity_mapper(*vb_sanity, BA_Write_Only);
				float4* sanity_data = sanity_mapper.Pointer<float4>();
				for (uint32_t j = 0; j < num_vertices; ++ j)
				{
					float const x = static_cast<float>(dis(gen));
					float const y = static_cast<float>(dis(gen));
					float const z = static_cast<float>(dis(gen));
					float const w = static_cast<float>(dis(gen));

					input_data[j] = float4(x, y, z, w);

					if (w > 0)
					{
						sanity_data[sanity_size] = input_data[j];
						++ sanity_size;
					}
				}
			}

			re.BindSOBuffers(rl_out);
			query->Begin();
			re.Render(*effect, *tech, *rl_in);
			query->End();
			re.BindSOBuffers(RenderLayoutPtr());

			uint64_t const output_primitives = checked_pointer_cast<SOStatisticsQuery>(query)->NumPrimitivesWritten();

			EXPECT_TRUE(output_primitives == sanity_size);
			EXPECT_TRUE(CompareBuffer(*vb_sanity, 0,
				*vb_out, 0,
				sanity_size * 4, tolerance));
		}
	}

	void TestDrawIndirectCopyBuffer(float tolerance)
	{
		if (!gs_support_ || !draw_indirect_support_ || !uavs_at_every_stage_support_)
		{
			return;
		}

		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(-100, 100);

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto conditional_copy_buffer_tech = effect->TechniqueByName("ConditionalCopyBufferRw");
		auto copy_buffer_tech = effect->TechniqueByName("CopyBuffer");

		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto vb_in = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);
		auto vb_sanity = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);

		uint32_t const indirect_args[] = { 0, 1, 0, 0 };
		auto vb_num = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write | EAH_DrawIndirectArgs | EAH_GPU_Unordered | EAH_Raw,
			sizeof(indirect_args), indirect_args, sizeof(uint32_t));
		auto vb_num_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, vb_num->Size(), nullptr);

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->BindVertexStream(vb_in, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		auto vb_out = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_in->Size(), nullptr);
		rl_out->BindVertexStream(vb_out, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto rl_intermediate = rf.MakeRenderLayout();
		rl_intermediate->TopologyType(RenderLayout::TT_PointList);
		auto vb_intermediate = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_in->Size(), nullptr);
		rl_intermediate->BindVertexStream(vb_intermediate, VertexElement(VEU_Position, 0, EF_ABGR32F));
		rl_intermediate->BindIndirectArgs(vb_num);
		rl_intermediate->IndirectArgsOffset(0);

		auto fb = rf.MakeFrameBuffer();
		auto vb_num_uav = rf.MakeBufferUav(vb_num, EF_R32UI);
		fb->Attach(0, vb_num_uav);

		*(effect->ParameterByName("rw_output_primitives_buff")) = vb_num_uav;

		auto& re = rf.RenderEngineInstance();

		for (uint32_t i = 0; i < 10; ++ i)
		{
			uint32_t sanity_size = 0;
			{
				GraphicsBuffer::Mapper input_mapper(*vb_in, BA_Write_Only);
				float4* input_data = input_mapper.Pointer<float4>();
				GraphicsBuffer::Mapper sanity_mapper(*vb_sanity, BA_Write_Only);
				float4* sanity_data = sanity_mapper.Pointer<float4>();
				for (uint32_t j = 0; j < num_vertices; ++ j)
				{
					float const x = static_cast<float>(dis(gen));
					float const y = static_cast<float>(dis(gen));
					float const z = static_cast<float>(dis(gen));
					float const w = static_cast<float>(dis(gen));

					input_data[j] = float4(x, y, z, w);

					if (w > 0)
					{
						sanity_data[sanity_size] = input_data[j];
						++ sanity_size;
					}
				}
			}

			vb_num->UpdateSubresource(0, sizeof(indirect_args), indirect_args);

			re.BindSOBuffers(rl_intermediate);
			re.BindFrameBuffer(fb);
			re.Render(*effect, *conditional_copy_buffer_tech, *rl_in);
			re.BindFrameBuffer(FrameBufferPtr());
			re.BindSOBuffers(RenderLayoutPtr());

			vb_num->CopyToBuffer(*vb_num_cpu);
			uint32_t output_primitives;
			{
				GraphicsBuffer::Mapper mapper(*vb_num_cpu, BA_Read_Only);
				output_primitives = *mapper.Pointer<uint32_t>();
			}

			EXPECT_TRUE(output_primitives == sanity_size);

			re.BindSOBuffers(rl_out);
			re.Render(*effect, *copy_buffer_tech, *rl_intermediate);
			re.BindSOBuffers(RenderLayoutPtr());

			EXPECT_TRUE(CompareBuffer(*vb_sanity, 0,
				*vb_out, 0,
				sanity_size * 4, tolerance));
		}
	}

	void TestMultipleBuffers(float tolerance)
	{
		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(-100, 100);

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto tech = effect->TechniqueByName("MultipleBuffers");

		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();
		GraphicsBufferPtr vb_ins[2];
		GraphicsBufferPtr vb_outs[2];
		for (uint32_t i = 0; i < 2; ++ i)
		{
			uint32_t const size = (i == 0) ? sizeof(float4) : sizeof(float2);
			vb_ins[i] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, num_vertices * size, nullptr);
			vb_outs[i] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_ins[i]->Size(), nullptr);
		}

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->BindVertexStream(vb_ins[0], VertexElement(VEU_Position, 0, EF_ABGR32F));
		rl_in->BindVertexStream(vb_ins[1], VertexElement(VEU_TextureCoord, 0, EF_GR32F));

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		rl_out->BindVertexStream(vb_outs[0], VertexElement(VEU_Position, 0, EF_ABGR32F));
		rl_out->BindVertexStream(vb_outs[1], VertexElement(VEU_TextureCoord, 0, EF_GR32F));

		auto& re = rf.RenderEngineInstance();

		for (uint32_t t = 0; t < 10; ++ t)
		{
			for (uint32_t i = 0; i < 2; ++ i)
			{
				GraphicsBuffer::Mapper mapper(*vb_ins[i], BA_Write_Only);
				if (i == 0)
				{
					float4* input_data = mapper.Pointer<float4>();
					for (uint32_t j = 0; j < num_vertices; ++ j)
					{
						float const x = static_cast<float>(dis(gen));
						float const y = static_cast<float>(dis(gen));
						float const z = static_cast<float>(dis(gen));
						float const w = static_cast<float>(dis(gen));

						input_data[j] = float4(x, y, z, w);
					}
				}
				else
				{
					float2* input_data = mapper.Pointer<float2>();
					for (uint32_t j = 0; j < num_vertices; ++ j)
					{
						float const x = static_cast<float>(dis(gen));
						float const y = static_cast<float>(dis(gen));

						input_data[j] = float2(x, y);
					}
				}
			}

			re.BindSOBuffers(rl_out);
			re.Render(*effect, *tech, *rl_in);
			re.BindSOBuffers(RenderLayoutPtr());

			for (uint32_t i = 0; i < 2; ++ i)
			{
				EXPECT_TRUE(CompareBuffer(*vb_ins[i], 0,
					*vb_outs[i], 0,
					num_vertices * ((i == 0) ? 4 : 2), tolerance));
			}
		}
	}

private:
	bool draw_indirect_support_;
	bool uavs_at_every_stage_support_;
	bool gs_support_;
};

TEST_F(StreamOutputTest, CopyBuffer)
{
	TestCopyBuffer(1.0f / 255);
}

TEST_F(StreamOutputTest, VertexIDToBuffer)
{
	TestVertexIDToBuffer(1.0f / 255);
}

TEST_F(StreamOutputTest, ConditionalCopyBuffer)
{
	TestConditionalCopyBuffer(1.0f / 255);
}

TEST_F(StreamOutputTest, DrawIndirectCopyBuffer)
{
	TestDrawIndirectCopyBuffer(1.0f / 255);
}

TEST_F(StreamOutputTest, MultipleBuffers)
{
	TestMultipleBuffers(1.0f / 255);
}
