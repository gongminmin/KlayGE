#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
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
	void DecompressNormal(std::vector<uint8_t>& res_normals, std::vector<uint8_t> const & com_normals)
	{
		for (size_t i = 0; i < com_normals.size() / 4; ++ i)
		{
			float x = com_normals[i * 4 + 2] / 255.0f * 2 - 1;
			float y = com_normals[i * 4 + 1] / 255.0f * 2 - 1;
			float z = sqrt(1 - x * x - y * y);

			res_normals[i * 4 + 0] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((z * 0.5f + 0.5f) * 255 + 0.5f), 0, 255));
			res_normals[i * 4 + 1] = com_normals[i * 4 + 1];
			res_normals[i * 4 + 2] = com_normals[i * 4 + 2];
			res_normals[i * 4 + 3] = 0;
		}
	}

	void DecompressNormalMapSubresource(uint32_t width, uint32_t height, ElementFormat restored_format, 
		ElementInitData& restored_data, std::vector<uint8_t>& restored_data_block, ElementFormat com_format, ElementInitData const & com_data)
	{
		KFL_UNUSED(restored_format);

		std::vector<uint8_t> normals(width * height * 4);

		if (IsCompressedFormat(com_format))
		{
			std::unique_ptr<TexCompression> tex_codec;
			switch (com_format)
			{
			case EF_BC3:
				tex_codec = MakeUniquePtr<TexCompressionBC3>();
				break;

			case EF_BC5:
				tex_codec = MakeUniquePtr<TexCompressionBC5>();
				break;

			default:
				KFL_UNREACHABLE("Compression formats other than BC3 and BC5 are not supported");
			}

			for (uint32_t y_base = 0; y_base < height; y_base += 4)
			{
				for (uint32_t x_base = 0; x_base < width; x_base += 4)
				{
					uint32_t argb[16];
				
					if (EF_BC5 == com_format)
					{
						uint16_t gr[16];
						tex_codec->DecodeBlock(gr, static_cast<uint8_t const *>(com_data.data) + ((y_base / 4) * width / 4 + x_base / 4) * 16);
						for (int i = 0; i < 16; ++ i)
						{
							argb[i] = (gr[i] & 0xFF00) | ((gr[i] & 0xFF) << 16);
						}
					}
					else
					{
						BOOST_ASSERT(EF_BC3 == com_format);

						tex_codec->DecodeBlock(argb, static_cast<uint8_t const *>(com_data.data) + ((y_base / 4) * width / 4 + x_base / 4) * 16);
					}

					for (int y = 0; y < 4; ++ y)
					{
						if (y_base + y < height)
						{
							for (int x = 0; x < 4; ++ x)
							{
								if (x_base + x < width)
								{
									std::memcpy(&normals[((y_base + y) * width + (x_base + x)) * 4], &argb[y * 4 + x], sizeof(uint32_t));
								}
							}
						}
					}
				}
			}
		}
		else
		{
			if (EF_GR8 == com_format)
			{
				uint8_t const * gr_data = static_cast<uint8_t const *>(com_data.data);
				for (uint32_t y = 0; y < height; ++ y)
				{
					for (uint32_t x = 0; x < width; ++ x)
					{
						normals[(y * width + x) * 4 + 0] = 0;
						normals[(y * width + x) * 4 + 1] = gr_data[y * com_data.row_pitch + x * 2 + 1];
						normals[(y * width + x) * 4 + 2] = gr_data[y * com_data.row_pitch + x * 2 + 0];
						normals[(y * width + x) * 4 + 3] = 0xFF;
					}
				}
			}
			else
			{
				BOOST_ASSERT(EF_ABGR8 == com_format);

				uint8_t const * abgr_data = static_cast<uint8_t const *>(com_data.data);
				for (uint32_t y = 0; y < height; ++ y)
				{
					for (uint32_t x = 0; x < width; ++ x)
					{
						normals[(y * width + x) * 4 + 0] = 0;
						normals[(y * width + x) * 4 + 1] = abgr_data[y * com_data.row_pitch + x * 4 + 1];
						normals[(y * width + x) * 4 + 2] = abgr_data[y * com_data.row_pitch + x * 4 + 0];
						normals[(y * width + x) * 4 + 3] = 0xFF;
					}
				}
			}
		}

		if (restored_format != EF_ARGB8)
		{
			std::vector<uint8_t> argb8_normals(width * height * 4);
			ResizeTexture(&argb8_normals[0], width * 4, width * height * 4, EF_ARGB8, width, height, 1, &normals[0], width * 4,
				width * height * 4, restored_format, width, height, 1, TextureFilter::Point);
			normals.swap(argb8_normals);
		}

		restored_data_block.resize(width * height * 4);
		restored_data.row_pitch = width * 4;
		restored_data.slice_pitch = width * height * 4;
		restored_data.data = &restored_data_block[0];
		DecompressNormal(restored_data_block, normals);
	}

	void Normal2NaLength(std::string const & in_file, std::string const & out_file, ElementFormat new_format)
	{
		TexturePtr in_tex = LoadSoftwareTexture(in_file);
		auto const in_type = in_tex->Type();
		auto const in_width = in_tex->Width(0);
		auto const in_height = in_tex->Height(0);
		auto const in_depth = in_tex->Depth(0);
		auto const in_num_mipmaps = in_tex->NumMipMaps();
		auto const in_array_size = in_tex->ArraySize();
		auto const in_format = in_tex->Format();
		auto const & in_data = checked_cast<SoftwareTexture&>(*in_tex).SubresourceData();

		TexCompressionBC4 bc4_codec;

		std::vector<std::vector<uint8_t>> level_lengths(in_num_mipmaps * in_array_size);
		std::vector<ElementInitData> new_data(level_lengths.size());
		for (size_t array_index = 0; array_index < in_array_size; ++ array_index)
		{
			ElementInitData restored_data;
			std::vector<uint8_t> restored_data_block;
			if (in_format != EF_ARGB8)
			{
				DecompressNormalMapSubresource(in_width, in_height, EF_ARGB8, restored_data, restored_data_block, in_format, in_data[array_index * in_num_mipmaps]);
			}
			else
			{
				restored_data = in_data[array_index * in_num_mipmaps];
			}

			std::vector<float3> the_normals(in_width * in_height);
			ResizeTexture(&the_normals[0], in_width * sizeof(float3), in_width * in_height * sizeof(float3), EF_BGR32F, in_width, in_height,
				1, restored_data.data, restored_data.row_pitch, restored_data.slice_pitch, EF_ARGB8, in_width, in_height, 1,
				TextureFilter::Point);

			{
				if (IsCompressedFormat(new_format))
				{
					new_data[array_index * in_num_mipmaps + 0].row_pitch = (in_width + 3) / 4 * 8;
					new_data[array_index * in_num_mipmaps + 0].slice_pitch = (in_width + 3) / 4 * (in_height + 3) / 4 * 8;
					std::vector<uint8_t>& new_lengths = level_lengths[array_index * in_num_mipmaps + 0];
					new_lengths.resize(new_data[array_index * in_num_mipmaps + 0].slice_pitch);
					new_data[array_index * in_num_mipmaps + 0].data = &new_lengths[0];

					array<uint8_t, 16> uncom_len;
					uncom_len.fill(255);
					BC4Block len_bc4;
					bc4_codec.EncodeBlock(&len_bc4, &uncom_len[0], TCM_Quality);

					uint32_t dest = 0;
					for (uint32_t y_base = 0; y_base < in_height; y_base += 4)
					{
						for (uint32_t x_base = 0; x_base < in_width; x_base += 4)
						{
							if (EF_BC4 == new_format)
							{
								std::memcpy(&new_lengths[dest], &len_bc4, sizeof(len_bc4));
								dest += sizeof(len_bc4);
							}
							else
							{
								BOOST_ASSERT(EF_BC1 == new_format);

								BC1Block len_bc1;
								BC4ToBC1G(len_bc1, len_bc4);

								std::memcpy(&new_lengths[dest], &len_bc1, sizeof(len_bc1));
								dest += sizeof(len_bc1);
							}
						}
					}
				}
				else
				{
					new_data[array_index * in_num_mipmaps + 0].row_pitch = in_width * sizeof(uint8_t);
					new_data[array_index * in_num_mipmaps + 0].slice_pitch = in_width * in_height * sizeof(uint8_t);
					std::vector<uint8_t>& new_lengths = level_lengths[array_index * in_num_mipmaps + 0];
					new_lengths.resize(new_data[array_index * in_num_mipmaps + 0].slice_pitch, 255);
					new_data[array_index * in_num_mipmaps + 0].data = &new_lengths[0];
				}
			}

			uint32_t the_width = in_width;
			uint32_t the_height = in_height;
			for (uint32_t level = 1; level < in_num_mipmaps; ++ level)
			{
				uint32_t new_width = std::max(the_width / 2, 1U);
				uint32_t new_height = std::max(the_height / 2, 1U);
				std::vector<float3> new_normals(new_width * new_height);
				if (IsCompressedFormat(new_format))
				{
					new_data[array_index * in_num_mipmaps + level].row_pitch = (new_width + 3) / 4 * 8;
					new_data[array_index * in_num_mipmaps + level].slice_pitch = (new_width + 3) / 4 * (new_height + 3) / 4 * 8;
				}
				else
				{
					new_data[array_index * in_num_mipmaps + level].row_pitch = new_width * sizeof(uint8_t);
					new_data[array_index * in_num_mipmaps + level].slice_pitch = new_width * new_height * sizeof(uint8_t);
				}
				std::vector<uint8_t>& new_lengths = level_lengths[array_index * in_num_mipmaps + level];
				new_lengths.resize(new_data[array_index * in_num_mipmaps + level].slice_pitch);
				new_data[array_index * in_num_mipmaps + level].data = &new_lengths[0];

				uint32_t dest = 0;
				for (uint32_t y_base = 0; y_base < new_height; y_base += 4)
				{
					for (uint32_t x_base = 0; x_base < new_width; x_base += 4)
					{
						array<uint8_t, 16> uncom_len;
						for (uint32_t y = 0; y < 4; ++ y)
						{
							uint32_t y0 = MathLib::clamp((y_base + y) * 2 + 0, 0U, the_height - 1);
							uint32_t y1 = MathLib::clamp((y_base + y) * 2 + 1, 0U, the_height - 1);
							for (uint32_t x = 0; x < 4; ++ x)
							{
								uint32_t x0 = MathLib::clamp((x_base + x) * 2 + 0, 0U, the_width - 1);
								uint32_t x1 = MathLib::clamp((x_base + x) * 2 + 1, 0U, the_width - 1);

								float3 new_n = (the_normals[y0 * the_width + x0] + the_normals[y0 * the_width + x1]
									+ the_normals[y1 * the_width + x0] + the_normals[y1 * the_width + x1]) * 0.25f;
								float len = MathLib::length(new_n);
								uncom_len[y * 4 + x] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(len * 255.0f + 0.5f), 0, 255));
								if ((x_base + x < new_width) && (y_base + y < new_height))
								{
									new_normals[(y_base + y) * new_width + (x_base + x)] = new_n;
								}
							}
						}

						if (IsCompressedFormat(new_format))
						{
							BC4Block len_bc4;
							bc4_codec.EncodeBlock(&len_bc4, &uncom_len[0], TCM_Quality);

							if (EF_BC4 == new_format)
							{
								std::memcpy(&new_lengths[dest], &len_bc4, sizeof(len_bc4));
								dest += sizeof(len_bc4);
							}
							else
							{
								BOOST_ASSERT(EF_BC1 == new_format);

								BC1Block len_bc1;
								BC4ToBC1G(len_bc1, len_bc4);

								std::memcpy(&new_lengths[dest], &len_bc1, sizeof(len_bc1));
								dest += sizeof(len_bc1);
							}
						}
						else
						{
							for (uint32_t y = 0; y < 4; ++ y)
							{
								for (uint32_t x = 0; x < 4; ++ x)
								{
									if ((x_base + x < new_width) && (y_base + y < new_height))
									{
										new_lengths[(y_base + y) * new_width + (x_base + x)] = uncom_len[y * 4 + x];
									}
								}
							}
						}
					}
				}

				the_width = new_width;
				the_height = new_height;
				the_normals.swap(new_normals);
			}
		}

		TexturePtr out_tex = MakeSharedPtr<SoftwareTexture>(in_type, in_width, in_height, in_depth,
			in_num_mipmaps, in_array_size, new_format, true);
		out_tex->CreateHWResource(new_data, nullptr);
		SaveTexture(out_tex, out_file);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: Normal2NaLength xxx.dds yyy.dds [BC4 | BC1 | R]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	ElementFormat new_format = EF_BC4;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("BC4" == format_str)
		{
			new_format = EF_BC4;
		}
		else if ("BC1" == format_str)
		{
			new_format = EF_BC1;
		}
		else if ("R" == format_str)
		{
			new_format = EF_R8;
		}
	}

	Normal2NaLength(in_file, argv[2], new_format);

	cout << "Na Length map is saved to " << argv[2] << endl;

	Context::Destroy();

	return 0;
}
