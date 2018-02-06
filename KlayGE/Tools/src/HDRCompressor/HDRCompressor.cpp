#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Half.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include <boost/assert.hpp>

using namespace std;

namespace
{
	using namespace KlayGE;

	float3 constexpr lum_weight(0.2126f, 0.7152f, 0.0722f);

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
		ElementInitData const & hdr_data, ElementFormat y_format, ElementFormat c_format)
	{
		float const log2 = log(2.0f);

		uint32_t width = hdr_data.row_pitch / (sizeof(float) * 4);
		uint32_t height = hdr_data.slice_pitch / hdr_data.row_pitch;

		y_data.row_pitch = width * sizeof(uint16_t);
		y_data.slice_pitch = y_data.row_pitch * height;
		y_data_block.resize(y_data.slice_pitch);
		y_data.data = &y_data_block[0];

		float const * hdr_src = static_cast<float const *>(hdr_data.data);
		if (EF_R16 == y_format)
		{
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
		}
		else
		{
			half* y_dst = reinterpret_cast<half*>(&y_data_block[0]);

			for (uint32_t y = 0; y < height; ++ y)
			{
				for (uint32_t x = 0; x < width; ++ x)
				{
					float R = hdr_src[(y * width + x) * 4 + 0];
					float G = hdr_src[(y * width + x) * 4 + 1];
					float B = hdr_src[(y * width + x) * 4 + 2];
					float Y = CalcLum(R, G, B);

					float log_y = log(Y) / log2 + 16;

					y_dst[y * width + x] = half(log_y * 2048 / 65535);
				}
			}
		}

		uint32_t c_width = std::max(width / 2, 1U);
		uint32_t c_height = std::max(height / 2, 1U);

		c_data.row_pitch = (c_width + 3) / 4 * 16;
		c_data.slice_pitch = c_data.row_pitch * (c_height + 3) / 4;
		c_data_block.resize(c_data.slice_pitch);
		c_data.data = &c_data_block[0];
		uint8_t* c_dst = &c_data_block[0];

		TexCompressionBC3 bc3_codec;
		TexCompressionBC4 bc4_codec;

		for (uint32_t y_base = 0; y_base < c_height; y_base += 4)
		{
			for (uint32_t x_base = 0; x_base < c_width; x_base += 4)
			{
				uint8_t uncom_u[16];
				uint8_t uncom_v[16];
				for (int y = 0; y < 4; ++ y)
				{
					uint32_t const y0 = MathLib::clamp((y_base + y) * 2 + 0, 0U, height - 1);
					uint32_t const y1 = MathLib::clamp((y_base + y) * 2 + 1, 0U, height - 1);

					for (int x = 0; x < 4; ++ x)
					{
						uint32_t const x0 = MathLib::clamp((x_base + x) * 2 + 0, 0U, width - 1);
						uint32_t const x1 = MathLib::clamp((x_base + x) * 2 + 1, 0U, width - 1);

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

				if (EF_BC5 == c_format)
				{
					BC5Block com_bc5;
					bc4_codec.EncodeBlock(&com_bc5.red, uncom_u, TCM_Quality);
					bc4_codec.EncodeBlock(&com_bc5.green, uncom_v, TCM_Quality);

					std::memcpy(c_dst, &com_bc5, sizeof(com_bc5));
					c_dst += sizeof(com_bc5);
				}
				else
				{
					uint32_t uncom_argb[16];
					for (int y = 0; y < 4; ++ y)
					{
						for (int x = 0; x < 4; ++ x)
						{
							uncom_argb[y * 4 + x] = (uncom_u[y * 4 + x] << 24) | (uncom_v[y * 4 + x] << 8);
						}
					}

					BC3Block com_bc3;
					bc3_codec.EncodeBlock(&com_bc3, uncom_argb, TCM_Quality);

					std::memcpy(c_dst, &com_bc3, sizeof(com_bc3));
					c_dst += sizeof(com_bc3);
				}
			}
		}
	}

	void DecompressHDRSubresource(ElementInitData& hdr_data, std::vector<uint8_t>& hdr_data_block, ElementInitData const & y_data, ElementInitData const & c_data,
		ElementFormat y_format, ElementFormat c_format)
	{
		float const log2 = log(2.0f);

		uint32_t width = y_data.row_pitch / sizeof(uint16_t);
		uint32_t height = y_data.slice_pitch / y_data.row_pitch;

		hdr_data.row_pitch = width * sizeof(float) * 4;
		hdr_data.slice_pitch = hdr_data.row_pitch * height;
		hdr_data_block.resize(hdr_data.slice_pitch);
		hdr_data.data = &hdr_data_block[0];

		uint32_t c_width = std::max(width / 2, 1U);
		uint32_t c_height = std::max(height / 2, 1U);

		std::unique_ptr<TexCompression> tex_codec;
		switch (c_format)
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

		std::vector<uint8_t> c_data_uncom(c_width * c_height * 4);
		for (uint32_t y_base = 0; y_base < c_height; y_base += 4)
		{
			for (uint32_t x_base = 0; x_base < c_width; x_base += 4)
			{
				uint8_t const * src = static_cast<uint8_t const *>(c_data.data) + ((y_base / 4) * c_width / 4 + x_base / 4) * 16;

				uint32_t argb[16];
				if (EF_BC5 == c_format)
				{
					uint16_t gr[16];
					tex_codec->DecodeBlock(gr, src);
					for (int i = 0; i < 16; ++ i)
					{
						argb[i] = (gr[i] & 0xFF00) | ((gr[i] & 0xFF) << 16);
					}
				}
				else
				{
					tex_codec->DecodeBlock(argb, src);
				}

				for (int y = 0; y < 4; ++ y)
				{
					if (y_base + y < c_height)
					{
						for (int x = 0; x < 4; ++ x)
						{
							if (x_base + x < c_width)
							{
								std::memcpy(&c_data_uncom[((y_base + y) * c_width + (x_base + x)) * 4], &argb[y * 4 + x], sizeof(uint32_t));
							}
						}
					}
				}
			}
		}

		if (EF_R16 == y_format)
		{
			uint16_t const * y_src = static_cast<uint16_t const *>(y_data.data);
			float* hdr = reinterpret_cast<float*>(&hdr_data_block[0]);
			for (uint32_t y = 0; y < height; ++ y)
			{
				for (uint32_t x = 0; x < width; ++ x)
				{
					float Y = exp((y_src[y * width + x] / 2048.0f - 16) * log2);
					float B = c_data_uncom[(y / 2 * c_width + x / 2) * 4 + 2] / 256.0f;
					float R = c_data_uncom[(y / 2 * c_width + x / 2) * 4 + 1] / 256.0f;
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
		else
		{
			half const * y_src = static_cast<half const *>(y_data.data);
			float* hdr = reinterpret_cast<float*>(&hdr_data_block[0]);
			for (uint32_t y = 0; y < height; ++ y)
			{
				for (uint32_t x = 0; x < width; ++ x)
				{
					float Y = exp((y_src[y * width + x] * 65535 / 2048.0f - 16) * log2);
					float B = c_data_uncom[(y / 2 * c_width + x / 2) * 4 + 2] / 256.0f;
					float R = c_data_uncom[(y / 2 * c_width + x / 2) * 4 + 1] / 256.0f;
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
	}

	void CompressHDR(std::string const & in_file,
		std::string const & out_y_file, std::string const & out_c_file, ElementFormat y_format, ElementFormat c_format)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

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
				for (size_t j = 0; j < in_data[i].slice_pitch; j += 8)
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

		uint32_t last_width = in_data.back().row_pitch / (sizeof(float) * 4);
		uint32_t last_height = in_data.back().slice_pitch / in_data.back().row_pitch;
		if ((1 == last_width) && (1 == last_height))
		{
			uint32_t array_size = in_array_size;
			if (Texture::TT_Cube == in_type)
			{
				array_size *= 6;
			}

			std::vector<ElementInitData> no_last_in_data(array_size * (in_num_mipmaps - 1));
			for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
			{
				for (uint32_t mip = 0; mip < in_num_mipmaps - 1; ++ mip)
				{
					no_last_in_data[array_index * (in_num_mipmaps - 1) + mip] = in_data[array_index * in_num_mipmaps + mip];
				}
			}

			in_data.swap(no_last_in_data);
			-- in_num_mipmaps;
		}

		std::vector<ElementInitData> y_data(in_data.size());
		std::vector<ElementInitData> c_data(in_data.size());
		std::vector<std::vector<uint8_t>> y_data_block(in_data.size());
		std::vector<std::vector<uint8_t>> c_data_block(in_data.size());
		for (size_t i = 0; i < in_data.size(); ++ i)
		{
			CompressHDRSubresource(y_data[i], c_data[i], y_data_block[i], c_data_block[i], in_data[i], y_format, c_format);
		}

		SaveTexture(out_y_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, y_format, y_data);

		uint32_t c_width = std::max(in_width / 2, 1U);
		uint32_t c_height = std::max(in_height / 2, 1U);
		if (IsCompressedFormat(c_format))
		{
			c_width = (c_width + 3) & ~3;
			c_height = (c_height + 3) & ~3;
		}
		SaveTexture(out_c_file, in_type, c_width, c_height, in_depth, in_num_mipmaps, in_array_size, c_format, c_data);

		float mse = 0;
		int n = 0;
		{
			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				ElementInitData restored_data;
				std::vector<uint8_t> restored_data_block;
				DecompressHDRSubresource(restored_data, restored_data_block, y_data[i], c_data[i], y_format, c_format);

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

				n += width * height;
			}
		}

		mse /= n;
		float psnr = 10 * log10(65504.0f * 65504.0f / std::max(mse, 1e-6f));

		cout << "MSE: " << mse << endl;
		cout << "PSNR: " << psnr << endl;
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 2)
	{
		cout << "Usage: HDRCompressor xxx.dds [R16 | R16F] [BC5 | BC3]" << endl;
		return 1;
	}

	ElementFormat y_format = EF_R16;
	if (argc >= 3)
	{
		std::string format_str(argv[3]);
		if ("R16F" == format_str)
		{
			y_format = EF_R16F;
		}
	}

	ElementFormat c_format = EF_BC5;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("BC3" == format_str)
		{
			c_format = EF_BC3;
		}
	}

	filesystem::path output_path(argv[1]);
	std::string y_file = output_path.stem().string() + "_y" + output_path.extension().string();
	std::string c_file = output_path.stem().string() + "_c" + output_path.extension().string();

	CompressHDR(argv[1], y_file, c_file, y_format, c_format);

	cout << "HDR texture is compressed into " << y_file << " and " << c_file << endl;

	Context::Destroy();

	return 0;
}
