#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/BlockCompression.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

namespace
{
	void ForceTexSRGB(std::string const & in_file, std::string const & out_file)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		uint32_t num_sub_res = in_array_size;
		if (Texture::TT_Cube == in_type)
		{
			num_sub_res *= 6;
		}

		if (IsSRGB(in_format))
		{
			cout << "This texture is already in sRGB format." << endl;
			return;
		}
		ElementFormat new_format = MakeSRGB(in_format);
		if (new_format == in_format)
		{
			cout << "This texture format don't have a sRGB counterpart." << endl;
			return;
		}

		std::vector<ElementInitData> new_data(in_data.size());
		std::vector<std::vector<uint8_t> > new_data_block(in_data.size());

		std::vector<std::vector<uint8_t> > decom_data_block(in_data.size());
		if ((EF_BC1 == in_format) || (EF_BC2 == in_format) || (EF_BC3 == in_format))
		{
			std::vector<ElementInitData> decom_data(in_data.size());
			for (size_t index = 0; index < num_sub_res; ++ index)
			{
				decom_data[index * in_num_mipmaps].row_pitch = in_width * 4;
				decom_data[index * in_num_mipmaps].slice_pitch = decom_data[index * in_num_mipmaps].row_pitch * in_height;

				decom_data_block[index * in_num_mipmaps].resize(decom_data[index * in_num_mipmaps].slice_pitch);
				decom_data[index * in_num_mipmaps].data = &decom_data_block[index * in_num_mipmaps][0];

				switch (in_format)
				{
				case EF_BC1:
					DecodeBC1(&decom_data_block[index * in_num_mipmaps][0], decom_data[index * in_num_mipmaps].row_pitch,
						in_data[index * in_num_mipmaps].data, in_width, in_height);
					break;

				case EF_BC2:
					DecodeBC2(&decom_data_block[index * in_num_mipmaps][0], decom_data[index * in_num_mipmaps].row_pitch,
						in_data[index * in_num_mipmaps].data, in_width, in_height);
					break;

				default:
					DecodeBC3(&decom_data_block[index * in_num_mipmaps][0], decom_data[index * in_num_mipmaps].row_pitch,
						in_data[index * in_num_mipmaps].data, in_width, in_height);
					break;
				}
			}

			std::swap(in_data, decom_data);
		}

		std::vector<float> linear_data;
		std::vector<float> linear_data_small;
		for (size_t index = 0; index < num_sub_res; ++ index)
		{
			linear_data.resize(in_width * in_height * 4);
			linear_data_small.resize(((in_width + 1) / 2) * ((in_height + 1) / 2) * 4);

			new_data[index * in_num_mipmaps].row_pitch = in_width * 4;
			new_data[index * in_num_mipmaps].slice_pitch = new_data[index * in_num_mipmaps].row_pitch * in_height;

			new_data_block[index * in_num_mipmaps].resize(new_data[index * in_num_mipmaps].slice_pitch);
			new_data[index * in_num_mipmaps].data = &new_data_block[index * in_num_mipmaps][0];

			uint8_t const * src = static_cast<uint8_t const *>(in_data[index * in_num_mipmaps].data);
			uint8_t* dst = &new_data_block[index * in_num_mipmaps][0];
			for (uint32_t y = 0; y < in_height; ++ y)
			{
				for (uint32_t x = 0; x < in_width; ++ x)
				{
					for (uint32_t ch = 0; ch < 4; ++ ch)
					{
						if (ch < 3)
						{
							linear_data[(y * in_width + x) * 4 + ch] = src[x * 4 + ch] > 0 ? pow(src[x * 4 + ch] / 255.0f, 2.2f) : 0;
							dst[x * 4 + ch] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::linear_to_srgb(linear_data[(y * in_width + x) * 4 + ch]) * 255.0f + 0.5f), 0, 255));
						}
						else
						{
							linear_data[(y * in_width + x) * 4 + ch] = src[x * 4 + ch] / 255.0f;
							dst[x * 4 + ch] = src[x * 4 + ch];
						}
					}
				}

				src += in_data[index * in_num_mipmaps].row_pitch;
				dst += new_data[index * in_num_mipmaps].row_pitch;
			}

			uint32_t the_width = in_width;
			uint32_t the_height = in_height;
			for (uint32_t l = 1; l < in_num_mipmaps; ++ l)
			{
				uint32_t new_width = (the_width + 1) / 2;
				uint32_t new_height = (the_height + 1) / 2;

				new_data[index * in_num_mipmaps + l].row_pitch = new_width * 4;
				new_data[index * in_num_mipmaps + l].slice_pitch = new_data[index * in_num_mipmaps + l].row_pitch * new_height;

				new_data_block[index * in_num_mipmaps + l].resize(new_data[index * in_num_mipmaps + l].slice_pitch);
				new_data[index * in_num_mipmaps + l].data = &new_data_block[index * in_num_mipmaps + l][0];

				dst = &new_data_block[index * in_num_mipmaps + l][0];
				for (uint32_t y = 0; y < new_height; ++ y)
				{
					for (uint32_t x = 0; x < new_width; ++ x)
					{
						int x0 = x * 2 + 0;
						int y0 = y * 2 + 0;
						int x1 = MathLib::clamp<uint32_t>(x * 2 + 1, 0, the_width - 1);
						int y1 = MathLib::clamp<uint32_t>(y * 2 + 1, 0, the_height - 1);

						for (uint32_t ch = 0; ch < 4; ++ ch)
						{
							linear_data_small[(y * new_width + x) * 4 + ch] = (linear_data[(y0 * the_width + x0) * 4 + ch] + linear_data[(y0 * the_width + x1) * 4 + ch]
								+ linear_data[(y1 * the_width + x0) * 4 + ch] + linear_data[(y1 * the_width + x1) * 4 + ch]) * 0.25f;

							if (ch < 3)
							{
								dst[x * 4 + ch] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::linear_to_srgb(linear_data_small[(y * new_width + x) * 4 + ch]) * 255.0f + 0.5f), 0, 255));
							}
							else
							{
								dst[x * 4 + ch] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(linear_data_small[(y * new_width + x) * 4 + ch] * 255.0f + 0.5f), 0, 255));
							}
						}
					}

					dst += new_data[index * in_num_mipmaps + l].row_pitch;
				}

				std::swap(linear_data_small, linear_data);

				the_width = new_width;
				the_height = new_height;
			}
		}

		std::vector<std::vector<uint8_t> > com_data_block(in_data.size());
		if ((EF_BC1 == in_format) || (EF_BC2 == in_format) || (EF_BC3 == in_format))
		{
			int block_size;
			if ((EF_BC1 == in_format) || (EF_SIGNED_BC1 == in_format) || (EF_BC1_SRGB == in_format)
				|| (EF_BC4 == in_format) || (EF_SIGNED_BC4 == in_format) || (EF_BC4_SRGB == in_format))
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			std::vector<ElementInitData> com_data(in_data.size());
			for (size_t index = 0; index < num_sub_res; ++ index)
			{
				uint32_t the_width = in_width;
				uint32_t the_height = in_height;
				for (uint32_t l = 0; l < in_num_mipmaps; ++ l)
				{
					com_data[index * in_num_mipmaps + l].row_pitch = (the_width + 3) / 4 * block_size;
					com_data[index * in_num_mipmaps + l].slice_pitch = com_data[index * in_num_mipmaps + l].row_pitch * (the_height + 3) / 4;

					com_data_block[index * in_num_mipmaps + l].resize(com_data[index * in_num_mipmaps + l].slice_pitch);
					com_data[index * in_num_mipmaps + l].data = &com_data_block[index * in_num_mipmaps + l][0];

					switch (in_format)
					{
					case EF_BC1:
						EncodeBC1_sRGB(&com_data_block[index * in_num_mipmaps + l][0], com_data[index * in_num_mipmaps + l].row_pitch,
							new_data[index * in_num_mipmaps + l].data, the_width, the_height, the_width * 4, EBCM_Quality);
						break;

					case EF_BC2:
						EncodeBC2_sRGB(&com_data_block[index * in_num_mipmaps + l][0], com_data[index * in_num_mipmaps + l].row_pitch,
							new_data[index * in_num_mipmaps].data, the_width, the_height, the_width * 4, EBCM_Quality);
						break;

					default:
						EncodeBC3_sRGB(&com_data_block[index * in_num_mipmaps + l][0], com_data[index * in_num_mipmaps + l].row_pitch,
							new_data[index * in_num_mipmaps].data, the_width, the_height, the_width * 4, EBCM_Quality);
						break;
					}

					the_width = (the_width + 1) / 2;
					the_height = (the_height + 1) / 2;
				}
			}

			std::swap(new_data, com_data);
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, new_format, new_data);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: ForceTexSRGB xxx.dds [yyy.dds]" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	std::string in_file = argv[1];
	std::string out_file;
	if (argc >= 3)
	{
		out_file = argv[2];
	}
	else
	{
		out_file = argv[1];
	}

	ForceTexSRGB(in_file, out_file);

	cout << "sRGB texture is saved to " << out_file << endl;

	return 0;
}
