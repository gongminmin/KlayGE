#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Math.hpp>
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

	void CompressNormalMapSubresource(uint32_t width, uint32_t height, ElementFormat in_format, 
		ElementInitData const & in_data, ElementFormat new_format, ElementInitData& new_data)
	{
		UNREF_PARAM(in_format);
		BOOST_ASSERT(EF_ARGB8 == in_format);

		std::vector<uint8_t> const & normals = in_data.data;

		if (EF_AL8 == new_format)
		{
			new_data.data.resize(width * height * 2);
			new_data.row_pitch = width * 2;
			new_data.slice_pitch = width * 2 * height;

			std::vector<uint8_t>& com_normals = new_data.data;

			for (uint32_t y = 0; y < height; ++ y)
			{
				for (uint32_t x = 0; x < width; ++ x)
				{
					com_normals[(y * width + x) * 2 + 0] = normals[(y * width + x) * 4 + 1];
					com_normals[(y * width + x) * 2 + 1] = normals[(y * width + x) * 4 + 2];
				}
			}
		}
		else
		{
			new_data.data.resize(width * height);
			new_data.row_pitch = width;
			new_data.slice_pitch = width * height;

			std::vector<uint8_t>& com_normals = new_data.data;

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
							uncom_y[y * 4 + x] = normals[((y_base + y) * width + (x_base + x)) * 4 + 1];
							uncom_x[y * 4 + x] = normals[((y_base + y) * width + (x_base + x)) * 4 + 2];
						}
					}

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
						if (EF_BC3 == new_format)
						{
							BC3_layout com_bc3;
							com_bc3.alpha = x_bc4;

							BC4ToBC1G(com_bc3.bc1, y_bc4);

							memcpy(&com_normals[dest], &com_bc3, sizeof(com_bc3));
							dest += sizeof(com_bc3);
						}
					}
				}
			}
		}
	}

	void DecompressNormalMapSubresource(uint32_t width, uint32_t height, ElementFormat restored_format, 
		ElementInitData& restored_data, ElementFormat com_format, ElementInitData const & com_data)
	{
		UNREF_PARAM(restored_format);
		BOOST_ASSERT(EF_ARGB8 == restored_format);

		std::vector<uint8_t> normals(width * height * 4);

		if (EF_AL8 == com_format)
		{
			for (uint32_t y = 0; y < height; ++ y)
			{
				for (uint32_t x = 0; x < width; ++ x)
				{
					normals[(y * width + x) * 4 + 0] = 0;
					normals[(y * width + x) * 4 + 1] = com_data.data[(y * width + x) * 2 + 0];
					normals[(y * width + x) * 4 + 2] = 0;
					normals[(y * width + x) * 4 + 3] = com_data.data[(y * width + x) * 2 + 1];
				}
			}
		}
		else
		{
			for (uint32_t y_base = 0; y_base < height; y_base += 4)
			{
				for (uint32_t x_base = 0; x_base < width; x_base += 4)
				{
					uint32_t argb[16];
					if (EF_BC5 == com_format)
					{
						DecodeBC5(argb, &com_data.data[((y_base / 4) * width / 4 + x_base / 4) * 16]);
						for (int i = 0; i < 16; ++ i)
						{
							argb[i] = ((argb[i] << 8) & 0xFF000000) | (argb[i] & 0x0000FF00);
						}
					}
					else
					{
						if (EF_BC3 == com_format)
						{
							DecodeBC3(argb, &com_data.data[((y_base / 4) * width / 4 + x_base / 4) * 16]);
						}
					}

					for (int y = 0; y < 4; ++ y)
					{
						for (int x = 0; x < 4; ++ x)
						{
							memcpy(&normals[((y_base + y) * width + (x_base + x)) * 4], &argb[y * 4 + x], sizeof(uint32_t));
						}
					}
				}
			}
		}

		restored_data.row_pitch = width * 4;
		restored_data.slice_pitch = width * height * 4;
		restored_data.data.resize(restored_data.slice_pitch);
		DecompressNormal(restored_data.data, normals);
	}	

	float MSESubresource(uint32_t width, uint32_t height, ElementFormat in_format, 
		ElementInitData const & in_data, ElementFormat new_format, ElementInitData const & new_data)
	{
		ElementInitData restored_data;
		DecompressNormalMapSubresource(width, height, in_format, restored_data, new_format, new_data);

		std::vector<uint8_t> const & restored_normals = restored_data.data;
		std::vector<uint8_t> const & normals = in_data.data;

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

		mse /= width * height;

		return mse;
	}

	void CompressNormalMap(std::string const & in_file, std::string const & out_file, ElementFormat new_format)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint16_t in_numMipMaps;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_numMipMaps, in_format, in_data);

		if (in_format != EF_ARGB8)
		{
			cout << "Unsupported texture format" << endl;
			return;
		}

		std::vector<ElementInitData> new_data(in_data.size());

		for (size_t sub_res = 0; sub_res < in_data.size(); ++ sub_res)
		{
			uint32_t the_width = in_data[sub_res].row_pitch / 4;
			uint32_t the_height = in_data[sub_res].slice_pitch / in_data[sub_res].row_pitch;

			CompressNormalMapSubresource(the_width, the_height, in_format, in_data[sub_res], new_format, new_data[sub_res]);
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_numMipMaps, new_format, new_data);

		float mse = 0;
		{
			for (size_t sub_res = 0; sub_res < in_data.size(); ++ sub_res)
			{
				uint32_t the_width = in_data[sub_res].row_pitch / 4;
				uint32_t the_height = in_data[sub_res].slice_pitch / in_data[sub_res].row_pitch;

				mse += MSESubresource(the_width, the_height, in_format, in_data[sub_res], new_format, new_data[sub_res]);
			}
		}

		cout << "MSE: " << mse << endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "使用方法: NormalMapCompressor xxx.dds yyy.dds [AL8 | BC3 | BC5]" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	ElementFormat new_format = EF_BC5;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("AL8" == format_str)
		{
			new_format = EF_AL8;
		}
		else
		{
			if ("BC3" == format_str)
			{
				new_format = EF_BC3;
			}
		}
	}

	CompressNormalMap(argv[1], argv[2], new_format);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
