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
	void DecompressNormal(std::vector<uint8_t>& res_normals, std::vector<uint8_t> const & com_normals)
	{
		for (size_t i = 0; i < com_normals.size() / 4; ++ i)
		{
			float x = com_normals[i * 4 + 3] / 255.0f * 2 - 1;
			float y = com_normals[i * 4 + 1] / 255.0f * 2 - 1;

			res_normals[i * 4 + 1] = com_normals[i * 4 + 1];
			res_normals[i * 4 + 2] = com_normals[i * 4 + 3];
			res_normals[i * 4 + 0] = static_cast<uint8_t>(MathLib::clamp((sqrt(1 - x * x - y * y) / 2 + 1) * 255, 0.0f, 255.0f));
			res_normals[i * 4 + 3] = 0;		
		}
	}

	void CompressNormalMapSubresource(uint32_t width, uint32_t height, std::vector<Color>& in_data,
		ElementFormat new_format, ElementInitData& new_data, std::vector<uint8_t>& new_data_block)
	{
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
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						float3 n;
						n.x() = in_data[(y_base + y) * width + (x_base + x)].r() * 2 - 1;
						n.y() = in_data[(y_base + y) * width + (x_base + x)].g() * 2 - 1;
						n.z() = in_data[(y_base + y) * width + (x_base + x)].b() * 2 - 1;
						n = MathLib::normalize(n);

						uncom_x[y * 4 + x] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.x() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
						uncom_y[y * 4 + x] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.y() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
					}
				}

				if (IsCompressedFormat(new_format))
				{
					BC4_layout x_bc4;
					EncodeBC4(x_bc4, uncom_x);
					BC4_layout y_bc4;
					EncodeBC4(y_bc4, uncom_y);

					if (EF_BC5 == new_format)
					{
						BC5_layout com_bc5;
						com_bc5.red = x_bc4;
						com_bc5.green = y_bc4;

						memcpy(&com_normals[dest], &com_bc5, sizeof(com_bc5));
						dest += sizeof(com_bc5);
					}
					else
					{
						BOOST_ASSERT(EF_BC3 == new_format);

						BC3_layout com_bc3;
						com_bc3.alpha = x_bc4;

						BC4ToBC1G(com_bc3.bc1, y_bc4);

						memcpy(&com_normals[dest], &com_bc3, sizeof(com_bc3));
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
							com_normals[((y_base + y) * new_data.row_pitch + (x_base + x)) * 2 + 0] = uncom_x[y * 4 + x];
							com_normals[((y_base + y) * new_data.row_pitch + (x_base + x)) * 2 + 1] = uncom_y[y * 4 + x];
						}
					}
				}
			}
		}
	}

	void DecompressNormalMapSubresource(uint32_t width, uint32_t height, ElementFormat restored_format, 
		ElementInitData& restored_data, std::vector<uint8_t>& restored_data_block, ElementFormat com_format, ElementInitData const & com_data)
	{
		UNREF_PARAM(restored_format);
		BOOST_ASSERT(EF_ARGB8 == restored_format);

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
						uint8_t r[16];
						uint8_t g[16];
						DecodeBC5(r, g, static_cast<uint8_t const *>(com_data.data) + ((y_base / 4) * width / 4 + x_base / 4) * 16);
						for (int i = 0; i < 16; ++ i)
						{
							argb[i] = (r[i] << 8) | (g[i] << 24);
						}
					}
					else
					{
						BOOST_ASSERT(EF_BC3 == com_format);

						DecodeBC3(argb, static_cast<uint8_t const *>(com_data.data) + ((y_base / 4) * width / 4 + x_base / 4) * 16);
					}

					for (int y = 0; y < 4; ++ y)
					{
						if (y_base + y < height)
						{
							for (int x = 0; x < 4; ++ x)
							{
								if (x_base + x < width)
								{
									memcpy(&normals[((y_base + y) * width + (x_base + x)) * 4], &argb[y * 4 + x], sizeof(uint32_t));
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

		restored_data_block.resize(width * height * 4);
		restored_data.row_pitch = width * 4;
		restored_data.slice_pitch = width * height * 4;
		restored_data.data = &restored_data_block[0];
		DecompressNormal(restored_data_block, normals);
	}

	float MSESubresource(uint32_t width, uint32_t height, ElementFormat in_format, 
		ElementInitData const & in_data, ElementFormat new_format, ElementInitData const & new_data)
	{
		ElementInitData restored_data;
		std::vector<uint8_t> restored_data_block;
		DecompressNormalMapSubresource(width, height, in_format, restored_data, restored_data_block, new_format, new_data);

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

		uint32_t const elem_size = NumFormatBytes(in_format);

		std::vector<std::vector<Color> > in_color(in_data.size());
		for (size_t sub_res = 0; sub_res < in_color.size(); ++ sub_res)
		{
			uint32_t the_width = in_data[sub_res].row_pitch / elem_size;
			uint32_t the_height = in_data[sub_res].slice_pitch / in_data[sub_res].row_pitch;
			in_color[sub_res].resize(the_width * the_height);

			uint8_t const * src = static_cast<uint8_t const *>(in_data[sub_res].data);
			Color* dst = &in_color[sub_res][0];
			for (uint32_t y = 0; y < the_height; ++ y)
			{
				ConvertToABGR32F(in_format, src, the_width, dst);
				src += in_data[sub_res].row_pitch;
				dst += the_width;
			}
		}

		std::vector<ElementInitData> new_data(in_data.size());
		std::vector<std::vector<uint8_t> > new_data_block(in_data.size());

		for (size_t sub_res = 0; sub_res < in_data.size(); ++ sub_res)
		{
			uint32_t the_width = in_data[sub_res].row_pitch / 4;
			uint32_t the_height = in_data[sub_res].slice_pitch / in_data[sub_res].row_pitch;

			CompressNormalMapSubresource(the_width, the_height, in_color[sub_res], new_format, new_data[sub_res], new_data_block[sub_res]);
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, new_format, new_data);

		float mse = 0;
		int n = 0;
		{
			for (size_t sub_res = 0; sub_res < in_data.size(); ++ sub_res)
			{
				uint32_t the_width = in_data[sub_res].row_pitch / 4;
				uint32_t the_height = in_data[sub_res].slice_pitch / in_data[sub_res].row_pitch;

				mse += MSESubresource(the_width, the_height, in_format, in_data[sub_res], new_format, new_data[sub_res]);
				n += the_width * the_height;
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

	ResLoader::Instance().AddPath("../../../bin");

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

	CompressNormalMap(argv[1], argv[2], new_format);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
