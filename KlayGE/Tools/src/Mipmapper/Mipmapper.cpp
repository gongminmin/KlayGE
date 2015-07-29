#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;
using namespace KlayGE;

namespace
{
	void GenMipmap(std::string const & in_file, std::string const & out_file)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		uint32_t const elem_size = NumFormatBytes(in_format);

		uint32_t num_full_mip_maps = 1;
		uint32_t w = in_width;
		uint32_t h = in_height;
		while ((w != 1) || (h != 1))
		{
			++ num_full_mip_maps;

			w = std::max<uint32_t>(1U, w / 2);
			h = std::max<uint32_t>(1U, h / 2);
		}

		std::vector<ElementInitData> new_data(in_array_size * num_full_mip_maps);
		std::vector<std::vector<uint8_t>> new_data_block(new_data.size());

		for (uint32_t sub_res = 0; sub_res < in_array_size; ++ sub_res)
		{
			uint32_t the_width = in_width;
			uint32_t the_height = in_height;

			{
				ElementInitData& dst_data = new_data[sub_res * num_full_mip_maps];

				dst_data.row_pitch = the_width * elem_size;
				dst_data.slice_pitch = dst_data.row_pitch * the_height;

				new_data_block[sub_res * num_full_mip_maps].resize(dst_data.slice_pitch);

				dst_data.data = &new_data_block[sub_res * num_full_mip_maps][0];

				ElementInitData const & src_data = in_data[sub_res * in_num_mipmaps];

				uint8_t const * src = static_cast<uint8_t const *>(src_data.data);
				uint8_t* dst = &new_data_block[sub_res * num_full_mip_maps][0];
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					std::memcpy(dst, src, dst_data.row_pitch);

					src += src_data.row_pitch;
					dst += dst_data.row_pitch;
				}
			}

			for (uint32_t mip = 0; mip < num_full_mip_maps - 1; ++ mip)
			{
				uint32_t new_width = std::max(the_width / 2, 1U);
				uint32_t new_height = std::max(the_height / 2, 1U);

				ElementInitData& src_data = new_data[sub_res * num_full_mip_maps + mip];
				ElementInitData& dst_data = new_data[sub_res * num_full_mip_maps + mip + 1];

				dst_data.row_pitch = new_width * elem_size;
				dst_data.slice_pitch = dst_data.row_pitch * new_height;

				new_data_block[sub_res * num_full_mip_maps + mip + 1].resize(dst_data.slice_pitch);

				dst_data.data = &new_data_block[sub_res * num_full_mip_maps + mip + 1][0];

				ResizeTexture(&new_data_block[sub_res * num_full_mip_maps + mip + 1][0],
					dst_data.row_pitch, dst_data.slice_pitch,
					in_format, new_width, new_height, 1,
					src_data.data, src_data.row_pitch, src_data.slice_pitch,
					in_format, the_width, the_height, 1,
					true);

				the_width = new_width;
				the_height = new_height;
			}
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, num_full_mip_maps, in_array_size, in_format, new_data);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: Mipmapper xxx.dds [yyy.dds]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	std::string out_file;
	if (argc < 3)
	{
		out_file = in_file;
	}
	else
	{
		out_file = argv[2];
	}

	GenMipmap(in_file, out_file);

	cout << "Mipmapped texture is saved." << endl;

	Context::Destroy();

	return 0;
}
