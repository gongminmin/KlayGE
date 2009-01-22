#include <KlayGE/KlayGE.hpp>
#include <KlayGE/half.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/BlockCompression.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#pragma warning(push)
#pragma warning(disable: 4245)
#include <boost/filesystem.hpp>
#pragma warning(pop)
#include <boost/assert.hpp>

using namespace std;
namespace
{
	using namespace KlayGE;

	float3 const lum_weight(0.299f, 0.587f, 0.114f);

	float CalcLum(float r, float g, float b)
	{
		float y = lum_weight.x() * r + lum_weight.y() * g + lum_weight.z() * b;
		if (abs(y) < 0.0001f)
		{
			if (y > 0)
			{
				y = 0.0001f;
			}
			else
			{
				y = -0.0001f;
			}
		}

		return y;
	}

	void CompressHDRSubresource(ElementInitData& y_data, ElementInitData& c_data, std::vector<uint8_t>& y_data_block, std::vector<uint8_t>& c_data_block,
		ElementInitData const & hdr_data)
	{
		float const log2 = log(2.0f);

		uint32_t width = hdr_data.row_pitch / (sizeof(float) * 4);
		uint32_t height = hdr_data.slice_pitch / hdr_data.row_pitch;

		y_data.row_pitch = width * sizeof(uint16_t);
		y_data.slice_pitch = y_data.row_pitch * height;
		y_data_block.resize(y_data.slice_pitch);
		y_data.data = &y_data_block[0];

		float const * hdr_src = static_cast<float const *>(hdr_data.data);
		uint16_t* y_dst = reinterpret_cast<uint16_t*>(&y_data_block[0]);

		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float R = hdr_src[(y * width + x) * 4 + 0];
				float G = hdr_src[(y * width + x) * 4 + 1];
				float B = hdr_src[(y * width + x) * 4 + 2];
				float Y = CalcLum(R, G, B);

				float log_y = log(Y) / log2 + 16;

				y_dst[y * width + x] = static_cast<uint16_t>(MathLib::clamp<uint32_t>(static_cast<uint32_t>(log_y * 2048), 0, 65535));
			}
		}

		c_data.row_pitch = (width / 2 + 3) / 4 * 16;
		c_data.slice_pitch = y_data.row_pitch * (height / 2 + 3) / 4;
		c_data_block.resize(c_data.slice_pitch);
		c_data.data = &c_data_block[0];
		uint8_t* c_dst = &c_data_block[0];

		for (uint32_t y_base = 0; y_base < height / 2; y_base += 4)
		{
			for (uint32_t x_base = 0; x_base < width / 2; x_base += 4)
			{
				uint8_t uncom_u[16];
				uint8_t uncom_v[16];
				for (int y = 0; y < 4; ++ y)
				{
					uint32_t const y0 = (y_base + y) * 2 + 0;
					uint32_t const y1 = (y_base + y) * 2 + 1;

					for (int x = 0; x < 4; ++ x)
					{
						uint32_t const x0 = (x_base + x) * 2 + 0;
						uint32_t const x1 = (x_base + x) * 2 + 1;

						float R = hdr_src[(y0 * width + x0) * 4 + 0]
							+ hdr_src[(y0 * width + x1) * 4 + 0]
							+ hdr_src[(y1 * width + x0) * 4 + 0]
							+ hdr_src[(y1 * width + x1) * 4 + 0];
						float G = hdr_src[(y0 * width + x0) * 4 + 1]
							+ hdr_src[(y0 * width + x1) * 4 + 1]
							+ hdr_src[(y1 * width + x0) * 4 + 1]
							+ hdr_src[(y1 * width + x1) * 4 + 1];
						float B = hdr_src[(y0 * width + x0) * 4 + 2]
							+ hdr_src[(y0 * width + x1) * 4 + 2]
							+ hdr_src[(y1 * width + x0) * 4 + 2]
							+ hdr_src[(y1 * width + x1) * 4 + 2];
						float Y = CalcLum(R, G, B);
						
						float log_u = sqrt(lum_weight.z() * B / Y);
						float log_v = sqrt(lum_weight.x() * R / Y);

						uncom_u[y * 4 + x] = static_cast<uint8_t>(MathLib::clamp(log_u * 256 + 0.5f, 0.0f, 255.0f));
						uncom_v[y * 4 + x] = static_cast<uint8_t>(MathLib::clamp(log_v * 256 + 0.5f, 0.0f, 255.0f));
					}
				}

				BC5_layout com_bc5;
				EncodeBC4(com_bc5.red, uncom_u);
				EncodeBC4(com_bc5.green, uncom_v);

				memcpy(c_dst, &com_bc5, sizeof(com_bc5));
				c_dst += sizeof(com_bc5);
			}
		}
	}

	void DecompressHDRSubresource(ElementInitData& hdr_data, std::vector<uint8_t>& hdr_data_block, ElementInitData const & y_data, ElementInitData const & c_data)
	{
		float const log2 = log(2.0f);

		uint32_t width = y_data.row_pitch / sizeof(uint16_t);
		uint32_t height = y_data.slice_pitch / y_data.row_pitch;

		hdr_data.row_pitch = width * sizeof(float) * 4;
		hdr_data.slice_pitch = hdr_data.row_pitch * height;
		hdr_data_block.resize(hdr_data.slice_pitch);
		hdr_data.data = &hdr_data_block[0];

		std::vector<uint8_t> c_data_uncom(width * height);
		for (uint32_t y_base = 0; y_base < height / 2; y_base += 4)
		{
			for (uint32_t x_base = 0; x_base < width / 2; x_base += 4)
			{
				uint32_t argb[16];
				DecodeBC5(argb, static_cast<uint8_t const *>(c_data.data) + ((y_base / 4) * width / 2 / 4 + x_base / 4) * 16);

				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						memcpy(&c_data_uncom[((y_base + y) * width / 2 + (x_base + x)) * 4], &argb[y * 4 + x], sizeof(uint32_t));
					}
				}
			}
		}

		uint16_t const * y_src = static_cast<uint16_t const *>(y_data.data);
		float* hdr = reinterpret_cast<float*>(&hdr_data_block[0]);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float Y = exp((y_src[y * width + x] / 2048.0f - 16) * log2);
				float B = c_data_uncom[(y / 2 * width / 2 + x / 2) * 4 + 2] / 256.0f;
				float R = c_data_uncom[(y / 2 * width / 2 + x / 2) * 4 + 1] / 256.0f;
				B = B * B * Y;
				R = R * R * Y;
				float G = (Y - R - B) / lum_weight.y();

				hdr[(y * width + x) * 4 + 0] = R / lum_weight.x();
				hdr[(y * width + x) * 4 + 1] = G;
				hdr[(y * width + x) * 4 + 2] = B / lum_weight.z();
				hdr[(y * width + x) * 4 + 3] = 1;
			}
		}
	}

	void CompressHDR(std::string const & in_file,
		std::string const & out_y_file, std::string const & out_c_file)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint16_t in_numMipMaps;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_numMipMaps, in_format, in_data, in_data_block);

		if (EF_ABGR16F == in_format)
		{
			std::vector<ElementInitData> tran_data(in_data.size());
			std::vector<size_t> base(in_data.size());
			std::vector<uint8_t> tran_data_block;
			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				tran_data[i].row_pitch = in_data[i].row_pitch * 2;
				tran_data[i].slice_pitch = in_data[i].slice_pitch * 2;
				base[i] = tran_data_block.size();
				tran_data_block.resize(tran_data_block.size() + tran_data[i].slice_pitch);
				for (size_t j = 0; j < tran_data[i].slice_pitch; j += 8)
				{
					float* f32 = reinterpret_cast<float*>(&tran_data_block[base[i]] + j * 2);
					half const * f16 = static_cast<half const *>(in_data[i].data) + j / sizeof(half);

					f32[0] = f16[0];
					f32[1] = f16[1];
					f32[2] = f16[2];
					f32[3] = f16[3];
				}
			}

			in_data = tran_data;
			in_data_block = tran_data_block;
			in_format = EF_ABGR32F;

			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				in_data[i].data = &in_data_block[base[i]];
			}
		}

		if (in_format != EF_ABGR32F)
		{
			cout << "Unsupported texture format" << endl;
			return;
		}

		std::vector<ElementInitData> y_data(in_data.size());
		std::vector<ElementInitData> c_data(in_data.size());
		std::vector<uint8_t> y_data_block;
		std::vector<uint8_t> c_data_block;
		for (size_t i = 0; i < in_data.size(); ++ i)
		{
			CompressHDRSubresource(y_data[i], c_data[i], y_data_block, c_data_block, in_data[i]);
		}

		SaveTexture(out_y_file, in_type, in_width, in_height, in_depth, in_numMipMaps, EF_L16, y_data);
		SaveTexture(out_c_file, in_type, in_width / 2, in_height / 2, in_depth, in_numMipMaps, EF_BC5, c_data);

		float mse = 0;
		{
			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				ElementInitData restored_data;
				std::vector<uint8_t> restored_data_block;
				DecompressHDRSubresource(restored_data, restored_data_block, y_data[i], c_data[i]);

				uint32_t width = in_data[i].row_pitch / (sizeof(float) * 4);
				uint32_t height = in_data[i].slice_pitch / in_data[i].row_pitch;

				float const * org = static_cast<float const *>(in_data[i].data);
				float const * restored = static_cast<float const *>(restored_data.data);

				for (uint32_t y = 0; y < height; ++ y)
				{
					for (uint32_t x = 0; x < width; ++ x)
					{
						float diff_r = org[(y * width + x) * 4 + 0] - restored[(y * width + x) * 4 + 0];
						float diff_g = org[(y * width + x) * 4 + 1] - restored[(y * width + x) * 4 + 1];
						float diff_b = org[(y * width + x) * 4 + 2] - restored[(y * width + x) * 4 + 2];

						mse += diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
					}
				}
			}
		}

		cout << "MSE: " << mse << endl;
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;
	using namespace boost::filesystem;

	if (argc < 2)
	{
		cout << "使用方法: HDRCompressor xxx.dds" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	path output_path(argv[1]);
	std::string y_file = basename(output_path) + "_y" + extension(output_path);
	std::string c_file = basename(output_path) + "_c" + extension(output_path);

	CompressHDR(argv[1], y_file, c_file);

	cout << "HDR texture is compressed into " << y_file << " and " << c_file << endl;

	return 0;
}
