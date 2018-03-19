#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Blitter.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <random>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

void TestBlitter2D(uint32_t array_size, uint32_t mip_levels, bool linear)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	uint32_t const WIDTH = 512;
	uint32_t const HEIGHT = 512;

	std::vector<std::vector<uint32_t>> data(mip_levels);
	for (uint32_t m = 0; m < mip_levels; ++ m)
	{
		uint32_t const w = std::max(WIDTH >> m, 1U);
		uint32_t const h = std::max(HEIGHT >> m, 1U);

		data[m].resize(array_size * w * h);
		for (uint32_t z = 0; z < array_size; ++ z)
		{
			for (uint32_t y = 0; y < h; ++ y)
			{
				for (uint32_t x = 0; x < w; ++ x)
				{
					data[m][(z * h + y) * w + x] = 0xFF000000 | ((z * h + y) * w + x);
				}
			}
		}
	}

	std::vector<ElementInitData> init_data(array_size * mip_levels);
	for (uint32_t m = 0; m < mip_levels; ++ m)
	{
		uint32_t const w = std::max(WIDTH >> m, 1U);
		uint32_t const h = std::max(HEIGHT >> m, 1U);

		for (uint32_t i = 0; i < array_size; ++ i)
		{
			init_data[i * mip_levels + m].data = &data[m][i * w * h];
			init_data[i * mip_levels + m].row_pitch = w * sizeof(uint32_t);
			init_data[i * mip_levels + m].slice_pitch = init_data[i].row_pitch * h;
		}
	}
	TexturePtr src = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data);

	TexturePtr dst = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Write);

	std::ranlux24_base gen;
	std::uniform_int_distribution<uint32_t> dis_index(0, array_size - 1);
	std::uniform_int_distribution<uint32_t> dis_mip(0, mip_levels - 1);

	Blitter blitter;
	for (int i = 0; i < 10; ++ i)
	{
		uint32_t const array_index = dis_index(gen);
		uint32_t const mip = dis_mip(gen);

		uint32_t const w = std::max(WIDTH >> mip, 1U);
		uint32_t const h = std::max(HEIGHT >> mip, 1U);

		std::uniform_int_distribution<uint32_t> dis_x(0, w - 1);
		std::uniform_int_distribution<uint32_t> dis_y(0, h - 1);

		uint32_t const src_x_offset = dis_x(gen);
		uint32_t const src_y_offset = dis_y(gen);
		uint32_t const dst_x_offset = dis_x(gen);
		uint32_t const dst_y_offset = dis_y(gen);
		uint32_t const src_x_size = std::min(dis_x(gen) + 1, std::min(w - src_x_offset, w - dst_x_offset));
		uint32_t const src_y_size = std::min(dis_y(gen) + 1, std::min(h - src_y_offset, h - dst_y_offset));
		uint32_t const dst_x_size = std::min(src_x_size, std::min(w - src_x_offset, w - dst_x_offset));
		uint32_t const dst_y_size = std::min(src_y_size, std::min(h - src_y_offset, h - dst_y_offset));

		blitter.Blit(dst, array_index, mip, dst_x_offset, dst_y_offset, dst_x_size, dst_y_size,
			src, array_index, mip, src_x_offset, src_y_offset, src_x_size, src_y_size,
			linear);

		std::vector<uint32_t> sanity_data(dst_x_size * dst_y_size);
		ResizeTexture(&sanity_data[0], dst_x_size * sizeof(uint32_t), dst_x_size * dst_y_size * sizeof(uint32_t),
			EF_ABGR8, dst_x_size, dst_y_size, 1,
			&data[mip][(array_index * h + src_y_offset) * w + src_x_offset], w * sizeof(uint32_t), w * h * sizeof(uint32_t),
			EF_ABGR8, src_x_size, src_y_size, 1, linear);

		ElementInitData sanity_init_data;
		sanity_init_data.data = &sanity_data[0];
		sanity_init_data.row_pitch = dst_x_size * sizeof(uint32_t);
		sanity_init_data.slice_pitch = dst_x_size * dst_y_size * sizeof(uint32_t);
		TexturePtr dst_sanity = rf.MakeTexture2D(dst_x_size, dst_y_size, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read, sanity_init_data);

		EXPECT_TRUE(Compare2D(*dst_sanity, 0, 0, 0, 0,
			*dst, array_index, mip, dst_x_offset, dst_y_offset,
			dst_x_size, dst_y_size, 2.0f / 255));
	}
}

void TestBlitter2DToBuff(uint32_t array_size, uint32_t mip_levels)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	uint32_t const WIDTH = 512;
	uint32_t const HEIGHT = 512;

	std::vector<std::vector<uint32_t>> data(mip_levels);
	for (uint32_t m = 0; m < mip_levels; ++ m)
	{
		uint32_t const w = std::max(WIDTH >> m, 1U);
		uint32_t const h = std::max(HEIGHT >> m, 1U);

		data[m].resize(array_size * w * h);
		for (uint32_t z = 0; z < array_size; ++ z)
		{
			for (uint32_t y = 0; y < h; ++ y)
			{
				for (uint32_t x = 0; x < w; ++ x)
				{
					data[m][(z * h + y) * w + x] = 0xFF000000 | ((z * h + y) * w + x);
				}
			}
		}
	}

	std::vector<ElementInitData> init_data(array_size * mip_levels);
	for (uint32_t m = 0; m < mip_levels; ++ m)
	{
		uint32_t const w = std::max(WIDTH >> m, 1U);
		uint32_t const h = std::max(HEIGHT >> m, 1U);

		for (uint32_t i = 0; i < array_size; ++ i)
		{
			init_data[i * mip_levels + m].data = &data[m][i * w * h];
			init_data[i * mip_levels + m].row_pitch = w * sizeof(uint32_t);
			init_data[i * mip_levels + m].slice_pitch = init_data[i].row_pitch * h;
		}
	}
	TexturePtr src = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_Immutable, init_data);

	GraphicsBufferPtr dst = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Write, WIDTH * HEIGHT * sizeof(float4), nullptr, EF_ABGR32F);

	std::ranlux24_base gen;
	std::uniform_int_distribution<uint32_t> dis_index(0, array_size - 1);
	std::uniform_int_distribution<uint32_t> dis_mip(0, mip_levels - 1);

	Blitter blitter;
	for (int i = 0; i < 10; ++ i)
	{
		uint32_t const array_index = dis_index(gen);
		uint32_t const mip = dis_mip(gen);

		uint32_t const w = std::max(WIDTH >> mip, 1U);
		uint32_t const h = std::max(HEIGHT >> mip, 1U);

		std::uniform_int_distribution<uint32_t> dis_x(0, w - 1);
		std::uniform_int_distribution<uint32_t> dis_y(0, h - 1);

		uint32_t const src_x_offset = dis_x(gen);
		uint32_t const src_y_offset = dis_y(gen);
		uint32_t const src_x_size = std::min(dis_x(gen) + 1, w - src_x_offset);
		uint32_t const src_y_size = std::min(dis_y(gen) + 1, h - src_y_offset);
		// TODO: Supports arbitrary src_buffer_offset
		//uint32_t const dst_buff_offset = dis_x(gen);
		uint32_t const dst_buff_offset = 0;

		blitter.Blit(dst, dst_buff_offset,
			src, array_index, mip, src_x_offset, src_y_offset, src_x_size, src_y_size);

		GraphicsBufferPtr dst_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, dst->Size(), nullptr);
		dst->CopyToBuffer(*dst_cpu);

		GraphicsBuffer::Mapper mapper(*dst_cpu, BA_Read_Only);

		ElementInitData sanity_init_data;
		sanity_init_data.data = mapper.Pointer<float4>();
		sanity_init_data.row_pitch = src_x_size * sizeof(float4);
		sanity_init_data.slice_pitch = src_x_size * src_y_size * sizeof(float4);
		TexturePtr dst_sanity = rf.MakeTexture2D(src_x_size, src_y_size, 1, 1, EF_ABGR32F, 1, 0, EAH_CPU_Read, sanity_init_data);

		EXPECT_TRUE(Compare2D(*dst_sanity, 0, 0, 0, 0,
			*src, array_index, mip, src_x_offset, src_y_offset,
			src_x_size, src_y_size, 2.0f / 255));
	}
}

void TestBlitterBuffTo2D(uint32_t array_size, uint32_t mip_levels)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	uint32_t const WIDTH = 512;
	uint32_t const HEIGHT = 512;

	std::vector<uint32_t> data(WIDTH * HEIGHT);
	for (uint32_t y = 0; y < HEIGHT; ++y)
	{
		for (uint32_t x = 0; x < WIDTH; ++x)
		{
			data[y * WIDTH + x] = 0xFF000000 | (y * WIDTH + x);
		}
	}

	GraphicsBufferPtr src = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
		static_cast<uint32_t>(data.size() * sizeof(data[0])), &data[0]);

	TexturePtr dst = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Write);

	std::ranlux24_base gen;
	std::uniform_int_distribution<uint32_t> dis_index(0, array_size - 1);
	std::uniform_int_distribution<uint32_t> dis_mip(0, mip_levels - 1);

	Blitter blitter;
	for (int i = 0; i < 10; ++ i)
	{
		uint32_t const array_index = dis_index(gen);
		uint32_t const mip = dis_mip(gen);

		uint32_t const w = std::max(WIDTH >> mip, 1U);
		uint32_t const h = std::max(HEIGHT >> mip, 1U);

		std::uniform_int_distribution<uint32_t> dis_buff(0, static_cast<uint32_t>(data.size() - 1));
		std::uniform_int_distribution<uint32_t> dis_x(0, w - 1);
		std::uniform_int_distribution<uint32_t> dis_y(0, h - 1);

		uint32_t const src_x_offset = dis_x(gen);
		uint32_t const src_y_offset = dis_y(gen);
		uint32_t const dst_x_offset = dis_x(gen);
		uint32_t const dst_y_offset = dis_y(gen);
		uint32_t const src_x_size = std::min(dis_x(gen) + 1, std::min(w - src_x_offset, w - dst_x_offset));
		uint32_t const src_y_size = std::min(dis_y(gen) + 1, std::min(h - src_y_offset, h - dst_y_offset));
		uint32_t const dst_x_size = std::min(src_x_size, std::min(w - src_x_offset, w - dst_x_offset));
		uint32_t const dst_y_size = std::min(src_y_size, std::min(h - src_y_offset, h - dst_y_offset));
		// TODO: Supports arbitrary src_buffer_offset
		//uint32_t const src_buff_offset = std::min(dis_buff(gen) + 1, WIDTH * HEIGHT - src_x_size * src_y_size);
		uint32_t const src_buff_offset = 0;

		blitter.Blit(dst, array_index, mip, dst_x_offset, dst_y_offset, dst_x_size, dst_y_size,
			src, src_buff_offset, EF_ABGR8);

		std::vector<uint32_t> sanity_data(dst_x_size * dst_y_size);
		for (size_t j = 0; j < sanity_data.size(); ++ j)
		{
			sanity_data[j] = data[src_buff_offset + j];
		}

		ElementInitData sanity_init_data;
		sanity_init_data.data = &sanity_data[0];
		sanity_init_data.row_pitch = dst_x_size * sizeof(uint32_t);
		sanity_init_data.slice_pitch = dst_x_size * dst_y_size * sizeof(uint32_t);
		TexturePtr dst_sanity = rf.MakeTexture2D(dst_x_size, dst_y_size, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read, sanity_init_data);

		EXPECT_TRUE(Compare2D(*dst_sanity, 0, 0, 0, 0,
			*dst, array_index, mip, dst_x_offset, dst_y_offset,
			dst_x_size, dst_y_size, 2.0f / 255));
	}
}

TEST(BlitterTest, Blit2D)
{
	TestBlitter2D(1, 3, false);
}

TEST(BlitterTest, Blit2DArray)
{
	TestBlitter2D( 5, 4, false);
}

TEST(BlitterTest, Blit2DToBuff)
{
	TestBlitter2DToBuff(1, 3);
}

TEST(BlitterTest, Blit2DArrayToBuff)
{
	TestBlitter2DToBuff(5, 4);
}

TEST(BlitterTest, BlitBuffTo2D)
{
	TestBlitterBuffTo2D(5, 4);
}
