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

#include <iostream>
#include <random>
#include <string>
#include <vector>

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

class StreamOutputTest : public testing::Test
{
public:
	void SetUp() override
	{
		auto const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		stream_output_support_ = caps.stream_output_support;
		gs_support_ = caps.gs_support;
	}

	void TestCopyBuffer(float tolerance)
	{
		if (!stream_output_support_)
		{
			return;
		}

		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(-100, 100);

		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto vb_in = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);

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

			auto vb_out = CopyBuffer(vb_in);

			EXPECT_TRUE(CompareBuffer(*vb_in, 0,
				*vb_out, 0,
				num_vertices * 4, tolerance));
		}
	}

	void TestVertexIDToBuffer(float tolerance)
	{
		if (!stream_output_support_)
		{
			return;
		}

		uint32_t const num_vertices = 1024;
		auto vb_out = VertexIDToBuffer(num_vertices);

		std::vector<float4> sanity_data(num_vertices);
		for (uint32_t i = 0; i < num_vertices; ++ i)
		{
			sanity_data[i] = float4(i + 0.0f, i + 0.25f, i + 0.5f, i + 0.75f);
		}

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto vb_sanity = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, num_vertices * sizeof(float4), sanity_data.data());

		EXPECT_TRUE(CompareBuffer(*vb_sanity, 0,
			*vb_out, 0,
			static_cast<uint32_t>(sanity_data.size() * 4), tolerance));
	}

	void TestConditionalCopyBuffer(float tolerance)
	{
		if (!stream_output_support_ || !gs_support_)
		{
			return;
		}

		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(-100, 100);

		uint32_t const num_vertices = 1024;

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto vb_in = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);
		auto vb_sanity = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Read | EAH_CPU_Write, num_vertices * sizeof(float4), nullptr);

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

			uint64_t output_primitives;
			auto vb_out = ConditionalCopyBuffer(vb_in, output_primitives);

			EXPECT_TRUE(output_primitives == sanity_size);
			EXPECT_TRUE(CompareBuffer(*vb_sanity, 0,
				*vb_out, 0,
				sanity_size * 4, tolerance));
		}
	}

private:
	GraphicsBufferPtr CopyBuffer(GraphicsBufferPtr const & vb_in)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto tech = effect->TechniqueByName("CopyBuffer");

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->BindVertexStream(vb_in, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		auto vb_out = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_in->Size(), nullptr);
		rl_out->BindVertexStream(vb_out, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto& re = rf.RenderEngineInstance();
		re.BindSOBuffers(rl_out);
		re.Render(*effect, *tech, *rl_in);
		re.BindSOBuffers(RenderLayoutPtr());

		return vb_out;
	}

	GraphicsBufferPtr VertexIDToBuffer(uint32_t num_vertices)
	{
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

		return vb_out;
	}

	GraphicsBufferPtr ConditionalCopyBuffer(GraphicsBufferPtr const & vb_in, uint64_t& output_primitives)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();

		auto effect = SyncLoadRenderEffect("StreamOutput/StreamOutputTest.fxml");
		auto tech = effect->TechniqueByName("ConditionalCopyBuffer");

		auto rl_in = rf.MakeRenderLayout();
		rl_in->TopologyType(RenderLayout::TT_PointList);
		rl_in->BindVertexStream(vb_in, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto rl_out = rf.MakeRenderLayout();
		rl_out->TopologyType(RenderLayout::TT_PointList);
		auto vb_out = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Write, vb_in->Size(), nullptr);
		rl_out->BindVertexStream(vb_out, VertexElement(VEU_Position, 0, EF_ABGR32F));

		auto query = rf.MakeSOStatisticsQuery();

		auto& re = rf.RenderEngineInstance();
		re.BindSOBuffers(rl_out);
		query->Begin();
		re.Render(*effect, *tech, *rl_in);
		query->End();
		re.BindSOBuffers(RenderLayoutPtr());

		output_primitives = checked_pointer_cast<SOStatisticsQuery>(query)->NumPrimitivesWritten();

		return vb_out;
	}

private:
	bool stream_output_support_;
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
