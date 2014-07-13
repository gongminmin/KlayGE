#include <KlayGE/KlayGE.hpp>
#include <KFL/Half.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
	#include <filesystem>
	namespace KlayGE
	{
		namespace filesystem = std::tr2::sys;
	}
#else
	#include <boost/filesystem.hpp>
	namespace KlayGE
	{
		namespace filesystem = boost::filesystem;
	}
#endif
#include <boost/assert.hpp>

using namespace std;
namespace
{
	using namespace KlayGE;

	float3 ToDir(uint32_t face, uint32_t x, uint32_t y, uint32_t size)
	{
		float3 dir;
		switch (face)
		{
		case Texture::CF_Positive_X:
			dir.x() = +1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = (size - 1 - x + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Negative_X:
			dir.x() = -1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = (x + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Positive_Y:
			dir.x() = (x + 0.5f) / size * 2 - 1;
			dir.y() = +1;
			dir.z() = (y + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Negative_Y:
			dir.x() = (x + 0.5f) / size * 2 - 1;
			dir.y() = -1;
			dir.z() = (size - 1 - y + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Positive_Z:
			dir.x() = (x + 0.5f) / size * 2 - 1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = +1;
			break;

		case Texture::CF_Negative_Z:
		default:
			dir.x() = (size - 1 - x + 0.5f) / size * 2 - 1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = -1;
			break;
		}

		return MathLib::normalize(dir);
	}

	void ToAddress(uint32_t& face, uint32_t& x, uint32_t& y, float3 const & dir, uint32_t size)
	{
		float3 n_dir = MathLib::normalize(dir);
		float3 abs_dir = MathLib::abs(n_dir);
		if (abs_dir.x() > abs_dir.y())
		{
			if (abs_dir.x() > abs_dir.z())
			{
				face = n_dir.x() > 0 ? Texture::CF_Positive_X : Texture::CF_Negative_X;
			}
			else
			{
				face = n_dir.z() > 0 ? Texture::CF_Positive_Z : Texture::CF_Negative_Z;
			}
		}
		else
		{
			if (abs_dir.y() > abs_dir.z())
			{
				face = n_dir.y() > 0 ? Texture::CF_Positive_Y : Texture::CF_Negative_Y;
			}
			else
			{
				face = n_dir.z() > 0 ? Texture::CF_Positive_Z : Texture::CF_Negative_Z;
			}
		}

		switch (face)
		{
		case Texture::CF_Positive_X:
			n_dir /= abs_dir.x();
			x = MathLib::clamp(static_cast<uint32_t>((-n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Negative_X:
			n_dir /= abs_dir.x();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Positive_Y:
			n_dir /= abs_dir.y();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((+n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Negative_Y:
			n_dir /= abs_dir.y();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Positive_Z:
			n_dir /= abs_dir.z();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Negative_Z:
		default:
			n_dir /= abs_dir.z();
			x = MathLib::clamp(static_cast<uint32_t>((-n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;
		}
	}

	uint32_t ReverseBits(uint32_t bits)
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
		bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
		bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
		return bits;
	}

	float RadicalInverseVdC(uint32_t bits)
	{
		return ReverseBits(bits) * 2.3283064365386963e-10f; // / 0x100000000
	}

	float2 Hammersley2D(uint32_t i, uint32_t N)
	{
		return float2(static_cast<float>(i) / N, RadicalInverseVdC(i));
	}

	float3 ImportanceSampleGGX(float2 const & xi, float roughness)
	{
		float alpha = roughness * roughness;
		float phi = 2 * PI * xi.x();
		float cos_theta = sqrt((1 - xi.y()) / ((alpha * alpha - 1) * xi.y() + 1));
		float sin_theta = sqrt(1 - cos_theta * cos_theta);
		return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
	}

	float3 ImportanceSampleGGX(float2 const & xi, float roughness, float3 const & normal)
	{
		float3 h = ImportanceSampleGGX(xi, roughness);

		float3 up_vec = abs(normal.z()) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
		float3 tangent = MathLib::normalize(MathLib::cross(up_vec, normal));
		float3 binormal = MathLib::cross(normal, tangent);
		return tangent * h.x() + binormal * h.y() + normal * h.z();
	}

	Color PrefilterEnvMap(float roughness, float3 const & r, Color* env_map[6], uint32_t size)
	{
		float3 normal = r;
		float3 view = r;
		Color prefiltered_clr(0.0f, 0.0f, 0.0f, 0.0f);
		float total_weight = 0;

		uint32_t const NUM_SAMPLES = 1024;
		for (uint32_t i = 0; i < NUM_SAMPLES; ++ i)
		{
			float2 xi = Hammersley2D(i, NUM_SAMPLES);
			float3 h = ImportanceSampleGGX(xi, roughness, normal);
			float3 l = -MathLib::reflect(view, h);
			float n_dot_l = MathLib::clamp(MathLib::dot(normal, l), 0.0f, 1.0f);
			if (n_dot_l > 0)
			{
				uint32_t face, x, y;
				ToAddress(face, x, y, l, size);
				prefiltered_clr += env_map[face][y * size + x] * n_dot_l;
				total_weight += n_dot_l;
			}
		}

		return prefiltered_clr / total_weight;
	}

	void PrefilterCube(std::string const & in_file, std::string const & out_file)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		uint32_t out_num_mipmaps = 1;
		{
			uint32_t w = in_width;
			while (w > 8)
			{
				++ out_num_mipmaps;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}

		uint32_t w = in_width;
		std::vector<std::vector<Color> > prefilted_data(out_num_mipmaps * 6);
		Color* env_map[6];
		for (uint32_t face = 0; face < 6; ++ face)
		{
			prefilted_data[face * out_num_mipmaps].resize(w * w);
			
			uint8_t const * src = static_cast<uint8_t const *>(in_data[face * in_num_mipmaps].data);
			for (uint32_t y = 0; y < w; ++ y)
			{
				ConvertToABGR32F(in_format, src, w, &prefilted_data[face * out_num_mipmaps][y * w]);
				src += in_data[face * in_num_mipmaps].row_pitch;
			}

			env_map[face] = &prefilted_data[face * out_num_mipmaps][0];
		}

		for (uint32_t face = 0; face < 6; ++ face)
		{
			w = std::max<uint32_t>(1U, in_width / 2);
			for (uint32_t mip = 1; mip < out_num_mipmaps; ++ mip)
			{
				prefilted_data[face * out_num_mipmaps + mip].resize(w * w);
				float roughness = 1 - static_cast<float>(out_num_mipmaps - 1 - mip) / (out_num_mipmaps - 1);

				for (uint32_t y = 0; y < w; ++ y)
				{
					for (uint32_t x = 0; x < w; ++ x)
					{
						prefilted_data[face * out_num_mipmaps + mip][y * w + x]
							= PrefilterEnvMap(roughness, ToDir(face, x, y, w), env_map, in_width);
					}
				}

				w = std::max<uint32_t>(1U, w / 2);
			}
		}

		std::vector<ElementInitData> out_data(out_num_mipmaps * 6);
		std::vector<std::vector<half> > out_data_block(out_num_mipmaps * 6);
		for (uint32_t face = 0; face < 6; ++ face)
		{
			w = in_width;
			for (uint32_t mip = 0; mip < out_num_mipmaps; ++ mip)
			{
				out_data_block[face * out_num_mipmaps + mip].resize(w * w * sizeof(half) * 4);
				out_data[face * out_num_mipmaps + mip].data = &out_data_block[face * out_num_mipmaps + mip][0];
				out_data[face * out_num_mipmaps + mip].row_pitch = w * sizeof(half) * 4;
				out_data[face * out_num_mipmaps + mip].slice_pitch = w * out_data[face * out_num_mipmaps + mip].row_pitch;

				ConvertFromABGR32F(EF_ABGR16F, &prefilted_data[face * out_num_mipmaps + mip][0], w * w,
					&out_data_block[face * out_num_mipmaps + mip][0]);

				w = std::max<uint32_t>(1U, w / 2);
			}
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, out_num_mipmaps, in_array_size, EF_ABGR16F, out_data);
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 2)
	{
		cout << "Usage: PrefilterCube xxx.dds [xxx_filtered.dds]" << endl;
		return 1;
	}

	std::string input(argv[1]);
	std::string output;
	if (argc >= 3)
	{
		output = argv[2];
	}
	else
	{
		filesystem::path output_path(argv[1]);
		output = filesystem::basename(output_path) + "_filtered.dds";
	}

	PrefilterCube(input, output);

	cout << "Filtered cube map is saved into " << output << endl;

	ResLoader::Destroy();

	return 0;
}
