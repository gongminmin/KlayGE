#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

namespace
{
	void CompressTex(std::string const & in_file, std::string const & out_file, ElementFormat fmt)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		if (IsSigned(in_format))
		{
			fmt = MakeSigned(fmt);
		}
		if (IsSRGB(in_format))
		{
			fmt = MakeSRGB(fmt);
		}

		std::vector<ElementInitData> new_data(in_data.size());
		std::vector<std::vector<uint8_t> > new_data_block(in_data.size());

		for (size_t sub_res = 0; sub_res < in_array_size; ++ sub_res)
		{
			uint32_t the_width = in_width;
			uint32_t the_height = in_height;

			for (uint32_t mip = 0; mip < in_num_mipmaps; ++ mip)
			{
				ElementInitData& src_data = in_data[sub_res * in_num_mipmaps + mip];
				ElementInitData& dst_data = new_data[sub_res * in_num_mipmaps + mip];

				int block_size;
				if ((EF_BC1 == fmt) || (EF_SIGNED_BC1 == fmt) || (EF_BC1_SRGB == fmt)
					|| (EF_BC4 == fmt) || (EF_SIGNED_BC4 == fmt) || (EF_BC4_SRGB == fmt))
				{
					block_size = 8;
				}
				else
				{
					block_size = 16;
				}

				dst_data.row_pitch = ((the_width + 3) / 4) * block_size;
				dst_data.slice_pitch = dst_data.row_pitch* ((the_height + 3) / 4);

				new_data_block[sub_res * in_num_mipmaps + mip].resize(dst_data.slice_pitch);

				dst_data.data = &new_data_block[sub_res * in_num_mipmaps + mip][0];

				ResizeTexture(&new_data_block[sub_res * in_num_mipmaps + mip][0],
					dst_data.row_pitch, dst_data.slice_pitch,
					fmt, the_width, the_height, 1,
					src_data.data, src_data.row_pitch, src_data.slice_pitch,
					in_format, the_width, the_height, 1,
					false);

				the_width = (the_width + 1) / 2;
				the_height = (the_height + 1) / 2;
			}
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, fmt, new_data);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: TexCompressor BC1|BC2|BC3|BC4|BC5 xxx.dds [yyy.dds]" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	std::string fmt_str = argv[1];
	ElementFormat fmt;
	if ("BC1" == fmt_str)
	{
		fmt = EF_BC1;
	}
	else if ("BC2" == fmt_str)
	{
		fmt = EF_BC2;
	}
	else if ("BC3" == fmt_str)
	{
		fmt = EF_BC3;
	}
	else if ("BC4" == fmt_str)
	{
		fmt = EF_BC4;
	}
	else if ("BC5" == fmt_str)
	{
		fmt = EF_BC5;
	}
	else
	{
		cout << "Unknown output format. Should be BC1, BC2, BC3, BC4, or BC5." << endl;
		return 1;
	}

	std::string in_file = argv[2];
	std::string out_file;
	if (argc < 4)
	{
		out_file = in_file;
	}
	else
	{
		out_file = argv[3];
	}

	CompressTex(in_file, out_file, fmt);

	cout << "Compressed texture is saved." << endl;

	return 0;
}
