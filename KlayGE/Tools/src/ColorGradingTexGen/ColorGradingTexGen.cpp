#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

namespace
{
	using namespace KlayGE;

	void generate_flatten(std::string const & out_file, uint32_t size)
	{
		Texture::TextureType type = Texture::TT_2D;
		uint32_t width = size * size;
		uint32_t height = size;
		uint32_t depth = 1;
		uint32_t num_mipmaps = 1;
		uint32_t array_size = 1;
		ElementFormat format = EF_ARGB8;
		std::vector<ElementInitData> data;
		std::vector<uint8_t> data_block;

		uint32_t pitch = width * 4;
		data_block.resize(pitch * height);

		uint32_t* p = reinterpret_cast<uint32_t*>(&data_block[0]);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				Color clr(static_cast<float>(x % size) / (size - 1), static_cast<float>(y) / (size - 1), static_cast<float>(x / size) / (size - 1), 1);
				p[y * width + x] = clr.ARGB();
			}
		}

		data.resize(1);
		data[0].data = &data_block[0];
		data[0].row_pitch = pitch;
		data[0].slice_pitch = pitch * height;

		SaveTexture(out_file, type, width, height, depth, num_mipmaps, array_size, format, data);
	}

	void convert_flatten_to_vol(std::string const & out_file, std::string const & in_file)
	{
		Texture::TextureType type;
		uint32_t width, height, depth;
		uint32_t num_mipmaps;
		uint32_t array_size;
		ElementFormat format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, type, width, height, depth, num_mipmaps, array_size, format, in_data, in_data_block);

		if ((Texture::TT_2D == type) && ((EF_ARGB8 == format) || (EF_ABGR8 == format)))
		{
			uint32_t size = height;

			std::vector<ElementInitData> out_data;
			std::vector<uint8_t> out_data_block;

			uint32_t dst_pitch = size * 4;
			out_data_block.resize(dst_pitch * size * size);

			uint32_t const * src = static_cast<uint32_t const *>(in_data[0].data);
			uint32_t const src_pitch = in_data[0].row_pitch / 4;
			uint32_t* dst = reinterpret_cast<uint32_t*>(&out_data_block[0]);
			for (uint32_t z = 0; z < size; ++ z)
			{
				for (uint32_t y = 0; y < size; ++ y)
				{
					for (uint32_t x = 0; x < size; ++ x)
					{
						dst[(z * size + y) * size + x] = src[y * src_pitch + (z * size + x)];
					}
				}
			}

			out_data.resize(1);
			out_data[0].data = &out_data_block[0];
			out_data[0].row_pitch = dst_pitch;
			out_data[0].slice_pitch = dst_pitch * size;

			SaveTexture(out_file, Texture::TT_3D, size, size, size, num_mipmaps, array_size, format, out_data);
		}
		else
		{
			cout << "Unsupported texture format" << endl;
		}
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 3)
	{
		cout << "Usage: ColorGradingTexGen options xxx.dds yyy.dds" << endl;
		return 1;
	}

	std::string options = argv[1];
	if ("-f" == options)
	{
		generate_flatten(argv[2], 16);
		cout << "Color grading flatten texture is saved to " << argv[2] << endl;
	}
	else if ("-v" == options)
	{
		convert_flatten_to_vol(argv[2], argv[3]);
		cout << "Color grading 3D texture is saved to " << argv[2] << endl;
	}

	Context::Destroy();

	return 0;
}
