#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>

#include <boost/algorithm/string/case_conv.hpp>

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

		if (IsCompressedFormat(in_format))
		{
			cout << "This texture is already in compressed format." << endl;
			if (in_file != out_file)
			{
				SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data);
			}
			return;
		}

		uint32_t out_width = (in_width + 3) & ~3;
		uint32_t out_height = (in_height + 3) & ~3;

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
			uint32_t src_width = in_width;
			uint32_t src_height = in_height;

			uint32_t dst_width = out_width;
			uint32_t dst_height = out_height;

			for (uint32_t mip = 0; mip < in_num_mipmaps; ++ mip)
			{
				ElementInitData& src_data = in_data[sub_res * in_num_mipmaps + mip];
				ElementInitData& dst_data = new_data[sub_res * in_num_mipmaps + mip];

				uint32_t const block_size = NumFormatBytes(fmt) * 4;

				dst_data.row_pitch = ((dst_width + 3) / 4) * block_size;
				dst_data.slice_pitch = dst_data.row_pitch * ((dst_height + 3) / 4);

				new_data_block[sub_res * in_num_mipmaps + mip].resize(dst_data.slice_pitch);

				dst_data.data = &new_data_block[sub_res * in_num_mipmaps + mip][0];

				ResizeTexture(&new_data_block[sub_res * in_num_mipmaps + mip][0],
					dst_data.row_pitch, dst_data.slice_pitch,
					fmt, dst_width, dst_height, 1,
					src_data.data, src_data.row_pitch, src_data.slice_pitch,
					in_format, src_width, src_height, 1,
					false);

				src_width = std::max(src_width / 2, 1U);
				src_height = std::max(src_height / 2, 1U);

				dst_width = std::max(dst_width / 2, 1U);
				dst_height = std::max(dst_height / 2, 1U);
			}
		}

		SaveTexture(out_file, in_type, out_width, out_height, in_depth, in_num_mipmaps, in_array_size, fmt, new_data);
	}

	void PrintSupportedFormats()
	{
		cout << "Supported formats: bc1, bc2, bc3, bc4, bc5, bc7, etc1" << endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: TexCompressor format xxx.dds [yyy.dds]" << endl;
		cout << "\t";
		PrintSupportedFormats();
		return 1;
	}

	std::string fmt_str = argv[1];
	boost::algorithm::to_lower(fmt_str);
	size_t const fmt_hash = RT_HASH(fmt_str.c_str());

	ElementFormat fmt;
	if (CT_HASH("bc1") == fmt_hash)
	{
		fmt = EF_BC1;
	}
	else if (CT_HASH("bc2") == fmt_hash)
	{
		fmt = EF_BC2;
	}
	else if (CT_HASH("bc3") == fmt_hash)
	{
		fmt = EF_BC3;
	}
	else if (CT_HASH("bc4") == fmt_hash)
	{
		fmt = EF_BC4;
	}
	else if (CT_HASH("bc5") == fmt_hash)
	{
		fmt = EF_BC5;
	}
	else if (CT_HASH("bc7") == fmt_hash)
	{
		fmt = EF_BC7;
	}
	else if (CT_HASH("etc1") == fmt_hash)
	{
		fmt = EF_ETC1;
	}
	else
	{
		cout << "Unknown output format. ";
		PrintSupportedFormats();
		return 1;
	}

	std::string in_file = argv[2];
	if (ResLoader::Instance().Locate(in_file).empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		ResLoader::Destroy();
		return 1;
	}

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

	Context::Destroy();

	return 0;
}
