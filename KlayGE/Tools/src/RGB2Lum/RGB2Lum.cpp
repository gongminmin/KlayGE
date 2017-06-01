#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;
using namespace KlayGE;

namespace
{
	void RGB2FakeHeight(std::string const & in_file, std::string const & out_file)
	{
		float3 const RGB_TO_LUM(0.2126f, 0.7152f, 0.0722f);

		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		std::vector<ElementInitData> height_data(in_data.size());
		std::vector<std::vector<uint8_t>> height_data_block(in_data.size());
		for (size_t array_index = 0; array_index < in_array_size; ++ array_index)
		{
			auto& subres = in_data[array_index * in_num_mipmaps];
			uint8_t const * rgba_data;
			uint32_t rgba_row_pitch;
			uint32_t rgba_slice_pitch;
			std::vector<uint8_t> decoded_data;
			if (in_format != EF_ABGR8)
			{
				decoded_data.resize(in_width * in_height * in_depth * 4);
				ResizeTexture(&decoded_data[0], in_width * 4, in_width * in_height * 4, EF_ABGR8,
					in_width, in_height, in_depth, subres.data, subres.row_pitch, subres.slice_pitch, in_format,
					in_width, in_height, in_depth, false);

				rgba_data = &decoded_data[0];
				rgba_row_pitch = in_width * 4;
				rgba_slice_pitch = in_width * in_height * 4;
			}
			else
			{
				rgba_data = static_cast<uint8_t const *>(subres.data);
				rgba_row_pitch = subres.row_pitch;
				rgba_slice_pitch = subres.slice_pitch;
			}

			height_data_block[array_index * in_num_mipmaps].resize(in_width * in_height * in_depth);
			height_data[array_index * in_num_mipmaps].data = &height_data_block[array_index * in_num_mipmaps][0];
			height_data[array_index * in_num_mipmaps].row_pitch = in_width;
			height_data[array_index * in_num_mipmaps].slice_pitch = in_width * in_height;

			for (uint32_t z = 0; z < in_depth; ++ z)
			{
				for (uint32_t y = 0; y < in_height; ++ y)
				{
					for (uint32_t x = 0; x < in_width; ++ x)
					{
						float const r = rgba_data[z * rgba_slice_pitch + y * rgba_row_pitch + x * 4 + 0] / 255.0f;
						float const g = rgba_data[z * rgba_slice_pitch + y * rgba_row_pitch + x * 4 + 1] / 255.0f;
						float const b = rgba_data[z * rgba_slice_pitch + y * rgba_row_pitch + x * 4 + 2] / 255.0f;

						float const lum = MathLib::dot(float3(r, g, b), RGB_TO_LUM);
						height_data_block[array_index * in_num_mipmaps][(z * in_height + y) * in_width + x]
							= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(lum * 255 + 0.5f), 0, 255));
					}
				}

				uint32_t last_width = in_width;
				uint32_t last_height = in_height;
				uint32_t last_depth = in_depth;
				uint32_t this_width = std::max(in_width / 2, 1U);
				uint32_t this_height = std::max(in_height / 2, 1U);
				uint32_t this_depth = std::max(in_depth / 2, 1U);
				for (uint32_t mip = 1; mip < in_num_mipmaps; ++ mip)
				{
					height_data_block[array_index * in_num_mipmaps + mip].resize(this_width * this_height * this_depth);
					ResizeTexture(&height_data_block[array_index * in_num_mipmaps + mip][0], this_width, this_width * this_height, EF_R8,
						this_width, this_height, this_depth, &height_data_block[array_index * in_num_mipmaps + mip - 1][0],
						last_width, last_width * last_height, EF_R8,
						last_width, last_height, last_depth, false);

					this_width = std::max(this_width / 2, 1U);
					this_height = std::max(this_height / 2, 1U);
					this_depth = std::max(this_depth / 2, 1U);
				}
			}
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, EF_R8, height_data);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: RGB2FakeHeight xxx.dds yyy.dds" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	RGB2FakeHeight(in_file, argv[2]);

	cout << "The height map is saved to " << argv[2] << endl;

	Context::Destroy();

	return 0;
}
