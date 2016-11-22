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

	void CompressNormalMapSubresource(uint32_t width, uint32_t height, std::vector<Color>& in_data,
		ElementFormat new_format, ElementInitData& new_data, std::vector<uint8_t>& new_data_block)
	{
		TexCompressionBC4 bc4_codec;

		if (IsCompressedFormat(new_format))
		{
			new_data.row_pitch = (width + 3) / 4 * 16;
			new_data.slice_pitch = new_data.row_pitch * (height + 3) / 4;
		}
		else
		{
			new_data.row_pitch = width * 2;
			new_data.slice_pitch = new_data.row_pitch * height;
		}
		new_data_block.resize(new_data.slice_pitch);
		new_data.data = &new_data_block[0];

		uint8_t* com_normals = &new_data_block[0];

		int dest = 0;
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint8_t uncom_x[16];
				uint8_t uncom_y[16];
				for (uint32_t dy = 0; dy < 4; ++ dy)
				{
					uint32_t y = MathLib::clamp(y_base + dy, 0U, height - 1);
					for (uint32_t dx = 0; dx < 4; ++ dx)
					{
						uint32_t x = MathLib::clamp(x_base + dx, 0U, width - 1);

						float3 n;
						n.x() = in_data[y * width + x].r() * 2 - 1;
						n.y() = in_data[y * width + x].g() * 2 - 1;
						n.z() = in_data[y * width + x].b() * 2 - 1;
						n = MathLib::normalize(n);

						uncom_x[dy * 4 + dx] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.x() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
						uncom_y[dy * 4 + dx] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.y() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
					}
				}

				if (IsCompressedFormat(new_format))
				{
					BC4Block x_bc4;
					bc4_codec.EncodeBlock(&x_bc4, uncom_x, TCM_Quality);
					BC4Block y_bc4;
					bc4_codec.EncodeBlock(&y_bc4, uncom_y, TCM_Quality);

					if (EF_BC5 == new_format)
					{
						BC5Block com_bc5;
						com_bc5.red = x_bc4;
						com_bc5.green = y_bc4;

						std::memcpy(&com_normals[dest], &com_bc5, sizeof(com_bc5));
						dest += sizeof(com_bc5);
					}
					else
					{
						BOOST_ASSERT(EF_BC3 == new_format);

						BC3Block com_bc3;
						com_bc3.alpha = x_bc4;

						BC4ToBC1G(com_bc3.bc1, y_bc4);

						std::memcpy(&com_normals[dest], &com_bc3, sizeof(com_bc3));
						dest += sizeof(com_bc3);
					}
				}
				else
				{
					BOOST_ASSERT(EF_GR8 == new_format);

					for (int y = 0; y < 4; ++ y)
					{
						for (int x = 0; x < 4; ++ x)
						{
							com_normals[(y_base + y) * new_data.row_pitch + (x_base + x) * 2 + 0] = uncom_x[y * 4 + x];
							com_normals[(y_base + y) * new_data.row_pitch + (x_base + x) * 2 + 1] = uncom_y[y * 4 + x];
						}
					}
				}
			}
		}
	}

	void DecompressNormalMapSubresource(uint32_t width, uint32_t height, ElementFormat restored_format, 
		ElementInitData& restored_data, std::vector<uint8_t>& restored_data_block, ElementFormat com_format, ElementInitData const & com_data)
	{
		KFL_UNUSED(restored_format);

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
			BOOST_ASSERT(false);
			break;
		}

		std::vector<uint8_t> normals(width * height * 4);

		if (IsCompressedFormat(com_format))
		{
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
			BOOST_ASSERT(EF_GR8 == com_format);

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

		if (restored_format != EF_ARGB8)
		{
			std::vector<uint8_t> argb8_normals(width * height * 4);
			ResizeTexture(&argb8_normals[0], width * 4, width * height * 4, EF_ARGB8, width, height, 1,
				&normals[0], width * 4, width * height * 4, restored_format, width, height, 1, false);
			normals.swap(argb8_normals);
		}

		restored_data_block.resize(width * height * 4);
		restored_data.row_pitch = width * 4;
		restored_data.slice_pitch = width * height * 4;
		restored_data.data = &restored_data_block[0];
		DecompressNormal(restored_data_block, normals);
	}

	float MSESubresource(uint32_t width, uint32_t height, ElementFormat in_format, 
		ElementInitData const & in_data, ElementFormat new_format, ElementInitData const & new_data)
	{
		uint32_t out_width = (width + 3) & ~3;
		uint32_t out_height = (height + 3) & ~3;

		ElementInitData restored_data;
		std::vector<uint8_t> restored_data_block;
		DecompressNormalMapSubresource(out_width, out_height, in_format, restored_data, restored_data_block, new_format, new_data);

		if ((out_width != width) || (out_height != height))
		{
			ElementInitData resized_restored_data;
			std::vector<uint8_t> resized_restored_data_block;

			uint32_t const elem_size = NumFormatBytes(in_format);
			resized_restored_data.row_pitch = width * elem_size;
			resized_restored_data.slice_pitch = width * height * elem_size;
			resized_restored_data_block.resize(resized_restored_data.slice_pitch);
			resized_restored_data.data = &resized_restored_data_block[0];

			ResizeTexture(&resized_restored_data_block[0],
				width * elem_size, width * out_height * elem_size,
				in_format, width, height, 1,
				restored_data.data, restored_data.row_pitch, restored_data.slice_pitch,
				in_format, out_width, out_height, 1,
				true);

			restored_data = resized_restored_data;
			restored_data_block = resized_restored_data_block;
			restored_data.data = &restored_data_block[0];
		}

		uint8_t const * restored_normals = static_cast<uint8_t const *>(restored_data.data);
		uint8_t const * normals = static_cast<uint8_t const *>(in_data.data);

		float mse = 0;
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float diff_r = (normals[(y * width + x) * 4 + 0] - restored_normals[(y * width + x) * 4 + 0]) / 255.0f;
				float diff_g = (normals[(y * width + x) * 4 + 1] - restored_normals[(y * width + x) * 4 + 1]) / 255.0f;
				float diff_b = (normals[(y * width + x) * 4 + 2] - restored_normals[(y * width + x) * 4 + 2]) / 255.0f;

				mse += diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
			}
		}

		return mse;
	}

	void CompressNormalMap(std::string const & in_file, std::string const & out_file, ElementFormat new_format)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		uint32_t out_width = (in_width + 3) & ~3;
		uint32_t out_height = (in_height + 3) & ~3;

		std::vector<std::vector<Color>> in_color(in_data.size());
		for (size_t sub_res = 0; sub_res < in_array_size; ++ sub_res)
		{
			uint32_t src_width = in_width;
			uint32_t src_height = in_height;

			uint32_t dst_width = out_width;
			uint32_t dst_height = out_height;

			for (uint32_t mip = 0; mip < in_num_mipmaps; ++ mip)
			{
				in_color[sub_res * in_num_mipmaps + mip].resize(dst_width * dst_height);

				ResizeTexture(&in_color[sub_res * in_num_mipmaps + mip][0],
					dst_width * sizeof(Color), dst_width * dst_height * sizeof(Color),
					EF_ABGR32F, dst_width, dst_height, 1,
					in_data[sub_res * in_num_mipmaps + mip].data,
					in_data[sub_res * in_num_mipmaps + mip].row_pitch,
					in_data[sub_res * in_num_mipmaps + mip].slice_pitch,
					in_format, src_width, src_height, 1,
					true);

				src_width = std::max(src_width / 2, 1U);
				src_height = std::max(src_height / 2, 1U);

				dst_width = std::max(dst_width / 2, 1U);
				dst_height = std::max(dst_height / 2, 1U);
			}
		}

		std::vector<ElementInitData> new_data(in_data.size());
		std::vector<std::vector<uint8_t>> new_data_block(in_data.size());

		for (size_t sub_res = 0; sub_res < in_array_size; ++ sub_res)
		{
			uint32_t the_width = out_width;
			uint32_t the_height = out_height;

			for (uint32_t mip = 0; mip < in_num_mipmaps; ++ mip)
			{
				CompressNormalMapSubresource(the_width, the_height, in_color[sub_res * in_num_mipmaps + mip], new_format,
					new_data[sub_res * in_num_mipmaps + mip], new_data_block[sub_res * in_num_mipmaps + mip]);

				the_width = std::max(the_width / 2, 1U);
				the_height = std::max(the_height / 2, 1U);
			}
		}

		SaveTexture(out_file, in_type, out_width, out_height, in_depth, in_num_mipmaps, in_array_size, new_format, new_data);

		float mse = 0;
		int n = 0;
		for (size_t sub_res = 0; sub_res < in_array_size; ++ sub_res)
		{
			uint32_t the_width = in_width;
			uint32_t the_height = in_height;

			for (uint32_t mip = 0; mip < in_num_mipmaps; ++ mip)
			{
				mse += MSESubresource(the_width, the_height, in_format, in_data[sub_res * in_num_mipmaps + mip],
					new_format, new_data[sub_res * in_num_mipmaps + mip]);
				n += the_width * the_height;

				the_width = std::max(the_width / 2, 1U);
				the_height = std::max(the_height / 2, 1U);
			}
		}

		mse /= n;
		float psnr = 10 * log10(255 * 255 / std::max(mse, 1e-6f));

		cout << "MSE: " << mse << endl;
		cout << "PSNR: " << psnr << endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: NormalMapCompressor xxx.dds yyy.dds [BC3 | BC5 | GR]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	ElementFormat new_format = EF_BC5;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("BC3" == format_str)
		{
			new_format = EF_BC3;
		}
		else if ("GR" == format_str)
		{
			new_format = EF_GR8;
		}
	}

	CompressNormalMap(in_file, argv[2], new_format);

	cout << "Normal map is saved to " << argv[2] << endl;

	Context::Destroy();

	return 0;
}
