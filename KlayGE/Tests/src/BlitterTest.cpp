#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Blitter.hpp>

#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter in boost
#endif
#include <boost/test/unit_test.hpp>
#ifdef KLAYGE_COMPILER_CLANG
#pragma clang diagnostic pop
#endif

#include <vector>
#include <string>
#include <iostream>
#include <random>

using namespace std;
using namespace KlayGE;

bool Compare2D(std::string const & test_name,
	Texture& tex0, uint32_t tex0_array_index, uint32_t tex0_level, uint32_t tex0_x_offset, uint32_t tex0_y_offset,
	Texture& tex1, uint32_t tex1_array_index, uint32_t tex1_level, uint32_t tex1_x_offset, uint32_t tex1_y_offset,
	uint32_t width, uint32_t height, float tolerance)
{
	BOOST_ASSERT(1 == tex0.SampleCount());

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	ElementFormat const tex0_fmt = tex0.Format();
	ElementFormat const tex1_fmt = tex1.Format();

	TexturePtr tex0_cpu = rf.MakeTexture2D(width, height, 1, 1, tex0_fmt, 1, 0, EAH_CPU_Read, nullptr);
	tex0.CopyToSubTexture2D(*tex0_cpu, 0, 0, 0, 0, width, height,
		tex0_array_index, tex0_level, tex0_x_offset, tex0_y_offset, width, height);

	TexturePtr tex1_cpu = rf.MakeTexture2D(width, height, 1, 1, tex1_fmt, 1, 0, EAH_CPU_Read, nullptr);
	tex1.CopyToSubTexture2D(*tex1_cpu, 0, 0, 0, 0, width, height,
		tex1_array_index, tex1_level, tex1_x_offset, tex1_y_offset, width, height);

	bool match = true;
	{
		uint32_t const tex0_elem_size = NumFormatBytes(tex0_fmt);
		uint32_t const tex1_elem_size = NumFormatBytes(tex1_fmt);

		Texture::Mapper tex0_mapper(*tex0_cpu, 0, 0, TMA_Read_Only, 0, 0, width, height);
		uint8_t const * tex0_p = tex0_mapper.Pointer<uint8_t>();
		uint32_t const tex0_row_pitch = tex0_mapper.RowPitch();

		Texture::Mapper tex1_mapper(*tex1_cpu, 0, 0, TMA_Read_Only, 0, 0, width, height);
		uint8_t const * tex1_p = tex1_mapper.Pointer<uint8_t>();
		uint32_t const tex1_row_pitch = tex1_mapper.RowPitch();

		for (uint32_t y = 0; (y < height) && match; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				Color tex0_clr;
				ConvertToABGR32F(tex0_fmt, tex0_p + y * tex0_row_pitch + x * tex0_elem_size, 1, &tex0_clr);

				Color tex1_clr;
				ConvertToABGR32F(tex1_fmt, tex1_p + y * tex1_row_pitch + x * tex1_elem_size, 1, &tex1_clr);

				if ((abs(tex0_clr.r() - tex1_clr.r()) > tolerance) || (abs(tex0_clr.g() - tex1_clr.g()) > tolerance)
					|| (abs(tex0_clr.b() - tex1_clr.b()) > tolerance) || (abs(tex0_clr.a() - tex1_clr.a()) > tolerance))
				{
					match = false;
					break;
				}
			}
		}
		if (!match)
		{
			SaveTexture(tex0_cpu, test_name + "_tex0.dds");
			SaveTexture(tex1_cpu, test_name + "_tex1.dds");
		}
	}

	return match;
}

void TestBlitter2D(std::string const & test_name, uint32_t array_size, uint32_t mip_levels, bool linear)
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
	TexturePtr src = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data[0]);

	TexturePtr dst = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Write, nullptr);

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
		TexturePtr dst_sanity = rf.MakeTexture2D(dst_x_size, dst_y_size, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read, &sanity_init_data);

		BOOST_CHECK(Compare2D(test_name,
			*dst_sanity, 0, 0, 0, 0,
			*dst, array_index, mip, dst_x_offset, dst_y_offset,
			dst_x_size, dst_y_size, 2.0f / 255));
	}
}

void TestBlitter2DToBuff(std::string const & test_name, uint32_t array_size, uint32_t mip_levels)
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
	TexturePtr src = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data[0]);

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
		TexturePtr dst_sanity = rf.MakeTexture2D(src_x_size, src_y_size, 1, 1, EF_ABGR32F, 1, 0, EAH_CPU_Read, &sanity_init_data);

		BOOST_CHECK(Compare2D(test_name,
			*dst_sanity, 0, 0, 0, 0,
			*src, array_index, mip, src_x_offset, src_y_offset,
			src_x_size, src_y_size, 2.0f / 255));
	}
}

void TestBlitterBuffTo2D(std::string const & test_name, uint32_t array_size, uint32_t mip_levels)
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

	TexturePtr dst = rf.MakeTexture2D(WIDTH, HEIGHT, mip_levels, array_size, EF_ABGR8, 1, 0, EAH_GPU_Write, nullptr);

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
		TexturePtr dst_sanity = rf.MakeTexture2D(dst_x_size, dst_y_size, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read, &sanity_init_data);

		BOOST_CHECK(Compare2D(test_name,
			*dst_sanity, 0, 0, 0, 0,
			*dst, array_index, mip, dst_x_offset, dst_y_offset,
			dst_x_size, dst_y_size, 2.0f / 255));
	}
}

BOOST_AUTO_TEST_CASE(Blit2D)
{
	TestBlitter2D("Blit2D", 1, 3, false);
}

BOOST_AUTO_TEST_CASE(Blit2DArray)
{
	TestBlitter2D("Blit2DArray", 5, 4, false);
}

BOOST_AUTO_TEST_CASE(Blit2DToBuff)
{
	TestBlitter2DToBuff("Blit2DToBuff", 1, 3);
}

BOOST_AUTO_TEST_CASE(Blit2DArrayToBuff)
{
	TestBlitter2DToBuff("Blit2DArrayToBuff", 5, 4);
}

BOOST_AUTO_TEST_CASE(BlitBuffTo2D)
{
	TestBlitterBuffTo2D("BlitBuffTo2D", 5, 4);
}
