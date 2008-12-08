#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Math.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

namespace
{
	struct BC1_layout
	{
		uint16_t clr_0, clr_1;
		uint16_t bitmap[2];
	};

	struct BC4_layout
	{
		uint8_t alpha_0, alpha_1;
		uint8_t bitmap[6];
	};

	struct BC3_layout
	{
		BC4_layout alpha;
		BC1_layout bc1;
	};

	Color RGB565_to_Color(uint16_t rgb)
	{
		return Color(((rgb >> 11) & 0x1F) / 31.0f, ((rgb >> 5) & 0x3F) / 63.0f, ((rgb >> 0) & 0x1F) / 31.0f, 1);
	}

	void DecodeBC1Internal(uint32_t* argb, BC1_layout const & bc1)
	{
		boost::array<Color, 4> clr;
		clr[0] = RGB565_to_Color(bc1.clr_0);
		clr[1] = RGB565_to_Color(bc1.clr_1);
		if (bc1.clr_0 > bc1.clr_1)
		{
			clr[2] = MathLib::lerp(clr[0], clr[1], 1 / 3.0f);
			clr[3] = MathLib::lerp(clr[0], clr[1], 2 / 3.0f);
		}
		else
		{
			clr[2] = (clr[0] + clr[1]) / 2;
			clr[3] = Color(0, 0, 0, 0);
		}

		for (int i = 0; i < 2; ++ i)
		{
			for (int j = 0; j < 8; ++ j)
			{
				argb[i * 8 + j] = clr[(bc1.bitmap[i] >> (j * 2)) & 0x3].ARGB();
			}
		}
	}

	void DecodeBC4Internal(uint8_t* alpha_block, BC4_layout const & bc4)
	{
		boost::array<float, 8> alpha;
		alpha[0] = bc4.alpha_0 / 255.0f;
		alpha[1] = bc4.alpha_1 / 255.0f;
		if (alpha[0] > alpha[1])
		{
			alpha[2] = MathLib::lerp(alpha[0], alpha[1], 1 / 7.0f);
			alpha[3] = MathLib::lerp(alpha[0], alpha[1], 2 / 7.0f);
			alpha[4] = MathLib::lerp(alpha[0], alpha[1], 3 / 7.0f);
			alpha[5] = MathLib::lerp(alpha[0], alpha[1], 4 / 7.0f);
			alpha[6] = MathLib::lerp(alpha[0], alpha[1], 5 / 7.0f);
			alpha[7] = MathLib::lerp(alpha[0], alpha[1], 6 / 7.0f);
		}
		else
		{
			alpha[2] = MathLib::lerp(alpha[0], alpha[1], 1 / 5.0f);
			alpha[3] = MathLib::lerp(alpha[0], alpha[1], 2 / 5.0f);
			alpha[4] = MathLib::lerp(alpha[0], alpha[1], 3 / 5.0f);
			alpha[5] = MathLib::lerp(alpha[0], alpha[1], 4 / 5.0f);
			alpha[6] = 0;
			alpha[7] = 1;
		}

		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			for (int j = 0; j < 8; ++ j)
			{
				alpha_block[i * 8 + j] = static_cast<uint8_t>(alpha[(alpha32 >> (j * 3)) & 0x7] * 255.0f + 0.5f);
			}
		}
	}

	void DecodeBC3(uint32_t* argb, uint8_t const * bc3)
	{
		BC3_layout const * bc3_layout = reinterpret_cast<BC3_layout const *>(bc3);

		DecodeBC1Internal(argb, bc3_layout->bc1);

		boost::array<uint8_t, 16> alpha_block;
		DecodeBC4Internal(&alpha_block[0], bc3_layout->alpha);

		for (size_t i = 0; i < alpha_block.size(); ++ i)
		{
			argb[i] &= 0x00FFFFFF;
			argb[i] |= alpha_block[i] << 24;
		}
	}

	void EncodeBC1_G_Only(BC1_layout& bc1, uint8_t const * green)
	{
		float max_value = 0;
		float min_value = 1;
		boost::array<float, 16> float_green;
		for (size_t i = 0; i < float_green.size(); ++ i)
		{
			float value = green[i] / 255.0f;

			float_green[i] = value;

			min_value = std::min(min_value, value);
			max_value = std::max(max_value, value);
		}

		int const L = 4;
		float const num_steps = L - 1.0f;

		/*// Newton's Method
		// x_{n+1} = x_n-\frac{f(x_n)}{f'(x_n)}\,
		// f_min(x) = \int (1-f)(src-(x(1-f)-bf))\, dx
		// f_min'(x) = (1-f)(src-(x(1-f)-bf))
		// f_min''(x) = (1-fs)^2
		// f_max(x) = \int f(src-(a(1-f)-xf))\, dx
		// f_max'(x) = f(src-(a(1-f)-xf))\
		// f_max''(x) = fs^2
		for (size_t i = 0; i < 3; ++ i)
		{
			if ((max_value - min_value) < 1.0f / 256.0f)
			{
				break;
			}

			float const inv_scale = num_steps / (max_value - min_value);

			float steps[4];
			for (size_t j = 0; j < L; ++ j)
			{
				steps[j] = MathLib::lerp(min_value, max_value, j / num_steps);
			}

			float d_min = 0;
			float d_max = 0;
			float d_2_min = 0;
			float d_2_max = 0;
			for (size_t j = 0; j < float_green.size(); ++ j)
			{
				float d = (float_green[j] - min_value) * inv_scale;

				uint32_t s;
				if (d <= 0.0f)
				{
					s = 0;
				}
				else
				{
					if (d >= num_steps)
					{
						s = 1;
					}
					else
					{
						s = static_cast<int>(d + 0.5f);
					}
				}

				if (s < L)
				{
					float const diff = float_green[j] - steps[s];
					float const fs = s / num_steps;

					d_min += (1 - fs) * diff;
					d_2_min += (1 - fs) * (1 - fs);

					d_max += fs * diff;
					d_2_max += fs * fs;
				}
			}

			if (d_2_min > 0.0f)
			{
				min_value -= d_min / d_2_min;
			}
			if (d_2_max > 0.0f)
			{
				max_value -= d_max / d_2_max;
			}

			if (min_value > max_value)
			{
				std::swap(min_value, max_value);
			}

			if ((d_min * d_min < 1.0f / 64) && (d_max * d_max < 1.0f / 64))
			{
				break;
			}
		}*/

		min_value = (min_value < 0.0f) ? 0.0f : (min_value > 1.0f) ? 1.0f : min_value;
		max_value = (max_value < 0.0f) ? 0.0f : (max_value > 1.0f) ? 1.0f : max_value;

		float ref_green0, ref_green1;
		ref_green0 = max_value;
		ref_green1 = min_value;

		bc1.clr_0 = static_cast<uint16_t>(ref_green0 * 255 + 0.5f) >> 2 << 5;
		bc1.clr_1 = static_cast<uint16_t>(ref_green1 * 255 + 0.5f) >> 2 << 5;

		boost::array<float, 4> steps;
		steps[0] = ref_green0;
		steps[1] = ref_green1;
		for (uint32_t i = 1; i < L - 1; ++ i)
		{
			steps[i + 1] = MathLib::lerp(steps[0], steps[1], i / num_steps);
		}

		float inv_scale = (steps[0] != steps[1]) ? (num_steps / (steps[1] - steps[0])) : 0.0f;

		boost::array<uint8_t, 16> bit_code;
		for (uint8_t i = 0; i < float_green.size(); ++ i)
		{
			bit_code[i] = 0;

			float d = (float_green[i] - steps[0]) * inv_scale;

			uint8_t s;
			if (d <= 0.0f)
			{
				s = 0;
			}
			else
			{
				if (d >= num_steps)
				{
					s = 1;
				}
				else
				{
					int id = static_cast<int>(d + 0.5f);
					if (0 == id)
					{
						s = 0;
					}
					else
					{
						if (num_steps == id)
						{
							s = 1;
						}
						else
						{
							s = static_cast<uint8_t>(id + 1);
						}
					}
				}
			}

			bit_code[i] = s;
		}

		bc1.bitmap[0] = (bit_code[7] << 14) | (bit_code[6] << 12) | (bit_code[5] << 10) | (bit_code[4] << 8)
			| (bit_code[3] << 6) | (bit_code[2] << 4) | (bit_code[1] << 2) | (bit_code[0] << 0);
		bc1.bitmap[1] = (bit_code[15] << 14) | (bit_code[14] << 12) | (bit_code[13] << 10) | (bit_code[12] << 8)
			| (bit_code[11] << 6) | (bit_code[10] << 4) | (bit_code[9] << 2) | (bit_code[8] << 0);
	}

	void EncodeBC4(BC4_layout& bc4, uint8_t const * alpha)
	{
		float max_value = 0;
		float min_value = 1;
		boost::array<float, 16> float_alpha;
		for (size_t i = 0; i < float_alpha.size(); ++ i)
		{
			float value = alpha[i] / 255.0f;

			float_alpha[i] = value;

			min_value = std::min(min_value, value);
			max_value = std::max(max_value, value);
		}

		uint32_t const L = ((0.0f == min_value) || (1.0f == max_value)) ? 6 : 8;

		if ((6 == L) && (min_value == max_value))
		{
			max_value = 1;
		}

		// Newton's Method
		// x_{n+1} = x_n-\frac{f(x_n)}{f'(x_n)}\,
		// f_min(x) = \int (1-f)(src-(x(1-f)-bf))\, dx
		// f_min'(x) = (1-f)(src-(x(1-f)-bf))
		// f_min''(x) = (1-fs)^2
		// f_max(x) = \int f(src-(a(1-f)-xf))\, dx
		// f_max'(x) = f(src-(a(1-f)-xf))\
		// f_max''(x) = fs^2
		float const num_steps = L - 1.0f;
		for (size_t i = 0; i < 8; ++ i)
		{
			if ((max_value - min_value) < 1.0f / 256.0f)
			{
				break;
			}

			float const inv_scale = num_steps / (max_value - min_value);

			float steps[8];
			for (size_t j = 0; j < L; ++ j)
			{
				steps[j] = MathLib::lerp(min_value, max_value, j / num_steps);
			}

			if (6 == L)
			{
				steps[6] = 0.0f;
				steps[7] = 1.0f;
			}

			float d_min = 0;
			float d_max = 0;
			float d_2_min = 0;
			float d_2_max = 0;
			for (size_t j = 0; j < float_alpha.size(); ++ j)
			{
				float d = (float_alpha[j] - min_value) * inv_scale;

				uint32_t s;
				if (d <= 0.0f)
				{
					s = ((6 == L) && (float_alpha[j] <= min_value * 0.5f)) ? 6 : 0;
				}
				else
				{
					if (d >= num_steps)
					{
						s = ((6 == L) && (float_alpha[j] >= (max_value + 1.0f) * 0.5f)) ? 7 : (L - 1);
					}
					else
					{
						s = static_cast<int>(d + 0.5f);
					}
				}

				if (s < L)
				{
					float const diff = float_alpha[j] - steps[s];
					float const fs = s / num_steps;

					d_min += (1 - fs) * diff;
					d_2_min += (1 - fs) * (1 - fs);

					d_max += fs * diff;
					d_2_max += fs * fs;
				}
			}

			if (d_2_min > 0.0f)
			{
				min_value -= d_min / d_2_min;
			}
			if (d_2_max > 0.0f)
			{
				max_value -= d_max / d_2_max;
			}

			if (min_value > max_value)
			{
				std::swap(min_value, max_value);
			}

			if ((d_min * d_min < 1.0f / 64) && (d_max * d_max < 1.0f / 64))
			{
				break;
			}
		}

		min_value = (min_value < 0.0f) ? 0.0f : (min_value > 1.0f) ? 1.0f : min_value;
		max_value = (max_value < 0.0f) ? 0.0f : (max_value > 1.0f) ? 1.0f : max_value;

		float ref_alpha0, ref_alpha1;
		if (6 == L)
		{
			ref_alpha0 = min_value;
			ref_alpha1 = max_value;
		}
		else
		{
			ref_alpha0 = max_value;
			ref_alpha1 = min_value;
		}

		bc4.alpha_0 = static_cast<uint8_t>(ref_alpha0 * 255 + 0.5f);
		bc4.alpha_1 = static_cast<uint8_t>(ref_alpha1 * 255 + 0.5f);

		boost::array<float, 8> steps;
		steps[0] = ref_alpha0;
		steps[1] = ref_alpha1;
		for (uint32_t i = 1; i < L - 1; ++ i)
		{
			steps[i + 1] = MathLib::lerp(steps[0], steps[1], i / num_steps);
		}

		if (6 == L)
		{
			steps[6] = 0;
			steps[7] = 1;
		}

		float inv_scale = (steps[0] != steps[1]) ? (num_steps / (steps[1] - steps[0])) : 0.0f;

		boost::array<uint8_t, 16> bit_code;
		for (uint8_t i = 0; i < float_alpha.size(); ++ i)
		{
			bit_code[i] = 0;

			float d = (float_alpha[i] - steps[0]) * inv_scale;

			uint8_t s;
			if (d <= 0.0f)
			{
				s = ((6 == L) && (float_alpha[i] <= steps[0] * 0.5f)) ? 6 : 0;
			}
			else
			{
				if (d >= num_steps)
				{
					s = ((6 == L) && (float_alpha[i] >= (steps[1] + 1.0f) * 0.5f)) ? 7 : 1;
				}
				else
				{
					int id = static_cast<int>(d + 0.5f);
					if (0 == id)
					{
						s = 0;
					}
					else
					{
						if (num_steps == id)
						{
							s = 1;
						}
						else
						{
							s = static_cast<uint8_t>(id + 1);
						}
					}
				}
			}

			bit_code[i] = s;
		}

		bc4.bitmap[0] = ((bit_code[2] & 0x3) << 6) | (bit_code[1] << 3) | (bit_code[0] << 0);
		bc4.bitmap[1] = ((bit_code[5] & 0x1) << 7) | (bit_code[4] << 4) | (bit_code[3] << 1) | ((bit_code[2] >> 2) << 0);
		bc4.bitmap[2] = (bit_code[7] << 5) | (bit_code[6] << 2) | ((bit_code[5] >> 1) << 0);
		bc4.bitmap[3] = ((bit_code[10] & 0x3) << 6) | (bit_code[9] << 3) | (bit_code[8] << 0);
		bc4.bitmap[4] = ((bit_code[13] & 0x1) << 7) | (bit_code[12] << 4) | (bit_code[11] << 1) | ((bit_code[10] >> 2) << 0);
		bc4.bitmap[5] = (bit_code[15] << 5) | (bit_code[14] << 2) | ((bit_code[13] >> 1) << 0);
	}

	void CompressNormal(std::vector<uint8_t>& normals)
	{
		for (size_t i = 0; i < normals.size() / 4; ++ i)
		{
			normals[i * 4 + 1] = normals[i * 4 + 1];
			normals[i * 4 + 3] = normals[i * 4 + 2];
			normals[i * 4 + 0] = 0;
			normals[i * 4 + 2] = 0;		
		}
	}

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
		std::vector<uint8_t> const & normals = in_data.data;

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
				BC1_layout y_bc1;
				EncodeBC1_G_Only(y_bc1, uncom_y);

				memcpy(&com_normals[dest], &x_bc4, sizeof(x_bc4));
				dest += sizeof(x_bc4);
				memcpy(&com_normals[dest], &y_bc1, sizeof(y_bc1));
				dest += sizeof(y_bc1);
			}
		}
	}

	void DecompressNormalMapSubresource(uint32_t width, uint32_t height, ElementFormat restored_format, 
		ElementInitData& restored_data, ElementFormat com_format, ElementInitData const & com_data)
	{
		std::vector<uint8_t> normals(width * height * 4);
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t argb[16];
				DecodeBC3(argb, &com_data.data[((y_base / 4) * width / 4 + x_base / 4) * 16]);

				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						memcpy(&normals[((y_base + y) * width + (x_base + x)) * 4], &argb[y * 4 + x], sizeof(uint32_t));
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
		cout << "使用方法: NormalMapCompressor xxx.dds yyy.dds [AL8 | DXT5]" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	ElementFormat new_format = EF_BC3;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("AL8" == format_str)
		{
			new_format = EF_AL8;
		}
	}

	CompressNormalMap(argv[1], argv[2], new_format);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
