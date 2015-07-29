#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

	void CreateDDM(std::vector<float2>& ddm, std::vector<float3> const & normal_map, float min_z)
	{
		ddm.resize(normal_map.size());
		for (size_t i = 0; i < normal_map.size(); ++ i)
		{
			float3 n = normal_map[i];
			n.z() = std::max(n.z(), min_z);
			ddm[i].x() = n.x() / n.z();
			ddm[i].y() = n.y() / n.z();
		}
	}

	void AccumulateDDM(std::vector<float>& height_map, std::vector<float2> const & ddm, uint32_t width, uint32_t height, int directions, int rings)
	{
		float const step = 2 * PI / directions;
		std::vector<float2> dxdy(directions);
		for (int i = 0; i < directions; ++ i)
		{
			MathLib::sincos(-i * step, dxdy[i].y(), dxdy[i].x());
		}

		std::vector<float2> tmp_hm[2];
		tmp_hm[0].resize(ddm.size(), float2(0, 0));
		tmp_hm[1].resize(ddm.size(), float2(0, 0));
		int active = 0;
		for (int i = 1; i < rings; ++ i)
		{
			for (size_t j = 0; j < ddm.size(); ++ j)
			{
				int y = static_cast<int>(j / width);
				int x = static_cast<int>(j - y * width);

				for (int k = 0; k < directions; ++ k)
				{
					float2 delta = dxdy[k] * static_cast<float>(i);
					float sample_x = x + delta.x();
					float sample_y = y + delta.y();
					int sample_x0 = static_cast<int>(floor(sample_x));
					int sample_y0 = static_cast<int>(floor(sample_y));
					int sample_x1 = sample_x0 + 1;
					int sample_y1 = sample_y0 + 1;
					float weight_x = sample_x - sample_x0;
					float weight_y = sample_y - sample_y0;

					sample_x0 %= width;
					sample_y0 %= height;
					sample_x1 %= width;
					sample_y1 %= height;

					float2 hl0 = MathLib::lerp(tmp_hm[active][sample_y0 * width + sample_x0], tmp_hm[active][sample_y0 * width + sample_x1], weight_x);
					float2 hl1 = MathLib::lerp(tmp_hm[active][sample_y1 * width + sample_x0], tmp_hm[active][sample_y1 * width + sample_x1], weight_x);
					float2 h = MathLib::lerp(hl0, hl1, weight_y);
					float2 ddl0 = MathLib::lerp(ddm[sample_y0 * width + sample_x0], ddm[sample_y0 * width + sample_x1], weight_x);
					float2 ddl1 = MathLib::lerp(ddm[sample_y1 * width + sample_x0], ddm[sample_y1 * width + sample_x1], weight_x);
					float2 dd = MathLib::lerp(ddl0, ddl1, weight_y);

					tmp_hm[!active][j] += h + dd * delta;
				}
			}

			active = !active;
		}

		float const scale = 0.5f / (directions * rings);

		height_map.resize(ddm.size());
		for (size_t i = 0; i < ddm.size(); ++ i)
		{
			float2 const & h = tmp_hm[active][i];
			height_map[i] = (h.x() + h.y()) * scale;
		}
	}

	void CreateHeightMap(std::string const & in_file, std::string const & out_file, float min_z)
	{
		Texture::TextureType type;
		uint32_t width, height, depth;
		uint32_t num_mipmaps;
		uint32_t array_size;
		ElementFormat format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, type, width, height, depth, num_mipmaps, array_size, format, in_data, in_data_block);

		if ((Texture::TT_2D == type) && ((EF_ABGR8 == format) || (EF_ARGB8 == format) || (EF_BC5 == format)))
		{
			TexCompressionBC5 bc5_codec;
			uint32_t const block_width = bc5_codec.BlockWidth();
			uint32_t const block_height = bc5_codec.BlockHeight();
			uint32_t const block_bytes = NumFormatBytes(format) * 4;

			uint32_t the_width = width;
			uint32_t the_height = height;

			std::vector<std::vector<float>> heights(in_data.size());
			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				uint8_t const * p = static_cast<uint8_t const *>(in_data[i].data);

				std::vector<float3> normals(the_width * the_height);
				if (EF_BC5 == format)
				{
					uint32_t const row_pitch = (the_width + block_width - 1) / block_width * block_bytes;
					uint32_t const slice_pitch = (the_height + block_height - 1) / block_height * row_pitch;

					std::vector<uint8_t> gr(std::max(the_width, 4U) * std::max(the_height, 4U) * 2);
					bc5_codec.DecodeMem(the_width, the_height, &gr[0], the_width * 2, the_width * the_height * 2,
						p, row_pitch, slice_pitch);
					for (uint32_t y = 0; y < the_height; ++ y)
					{
						for (uint32_t x = 0; x < the_width; ++ x)
						{
							float nx = gr[(y * the_width + x) * 2 + 0] / 255.0f * 2 - 1;
							float ny = gr[(y * the_width + x) * 2 + 1] / 255.0f * 2 - 1;
							normals[y * the_width + x].x() = nx;
							normals[y * the_width + x].y() = ny;
							normals[y * the_width + x].z() = sqrt(std::max(0.0f, 1 - nx * nx - ny * ny));
						}
					}
				}
				else
				{
					for (uint32_t y = 0; y < the_height; ++ y)
					{
						for (uint32_t x = 0; x < the_width; ++ x)
						{
							normals[y * the_width + x].x() = p[y * in_data[i].row_pitch + x * 4 + 0] / 255.0f * 2 - 1;
							normals[y * the_width + x].y() = p[y * in_data[i].row_pitch + x * 4 + 1] / 255.0f * 2 - 1;
							normals[y * the_width + x].z() = p[y * in_data[i].row_pitch + x * 4 + 2] / 255.0f * 2 - 1;
						}
					}
					if (EF_ARGB8 == format)
					{
						for (uint32_t y = 0; y < the_height; ++ y)
						{
							for (uint32_t x = 0; x < the_width; ++ x)
							{
								std::swap(normals[y * the_width + x].x(), normals[y * the_width + x].z());
							}
						}
					}
				}

				std::vector<float2> ddm;
				CreateDDM(ddm, normals, min_z);

				AccumulateDDM(heights[i], ddm, the_width, the_height, 4, 9);

				the_width = std::max(the_width / 2, 1U);
				the_height = std::max(the_height / 2, 1U);
			}

			float min_height = +1e10f;
			float max_height = -1e10f;
			for (size_t i = 0; i < heights.size(); ++ i)
			{
				for (size_t j = 0; j < heights[i].size(); ++ j)
				{
					min_height = std::min(min_height, heights[i][j]);
					max_height = std::max(max_height, heights[i][j]);
				}
			}
			if (max_height - min_height > 1e-6f)
			{
				for (size_t i = 0; i < heights.size(); ++ i)
				{
					for (size_t j = 0; j < heights[i].size(); ++ j)
					{
						heights[i][j] = (heights[i][j] - min_height) / (max_height - min_height);
					}
				}
			}

			std::vector<uint8_t> data_block;
			std::vector<size_t> base(in_data.size());
			for (size_t i = 0; i < heights.size(); ++ i)
			{
				base[i] = data_block.size();
				data_block.resize(data_block.size() + heights[i].size());

				for (size_t j = 0; j < heights[i].size(); ++ j)
				{
					data_block[base[i] + j] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(heights[i][j] * 255 + 0.5f), 0, 255));
				}
			}

			the_width = width;
			the_height = height;
			std::vector<ElementInitData> heights_data(in_data.size());
			for (size_t i = 0; i < heights.size(); ++ i)
			{
				heights_data[i].data = &data_block[base[i]];
				heights_data[i].row_pitch = the_width;
				heights_data[i].slice_pitch = the_width * the_height;

				the_width = std::max(the_width / 2, 1U);
				the_height = std::max(the_height / 2, 1U);
			}

			SaveTexture(out_file, type, width, height, depth, num_mipmaps, array_size, EF_R8, heights_data);
		}
		else
		{
			cout << "Unsupported texture format" << endl;
		}
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 3)
	{
		cout << "Usage: Normal2Height xxx.dds yyy.dds [min_z]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	float min_z = 1e-6f;
	if (argc >= 4)
	{
		min_z = static_cast<float>(atof(argv[3]));
	}

	CreateHeightMap(in_file, argv[2], min_z);

	cout << "Height map is saved to " << argv[2] << endl;

	Context::Destroy();

	return 0;
}
