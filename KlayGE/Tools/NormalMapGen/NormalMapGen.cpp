#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

	void CreateNormalMap(std::string const & in_file, std::string const & out_file)
	{
		Texture::TextureType type;
		uint32_t width, height, depth;
		uint16_t numMipMaps;
		ElementFormat format;
		std::vector<ElementInitData> in_data;
		LoadTexture(in_file, type, width, height, depth, numMipMaps, format, in_data);

		if ((Texture::TT_2D == type) && (EF_L8 == format))
		{
			uint32_t the_width = width;
			uint32_t the_height = height;

			std::vector<ElementInitData> normals(in_data.size());
			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				std::vector<uint8_t> heights(the_width * the_height);
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					for (uint32_t x = 0; x < the_width; ++ x)
					{
						heights[y * the_width + x] = in_data[i].data[y * in_data[i].row_pitch + x];
					}
				}

				std::vector<int> dx;
				dx.resize(heights.size());
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					for (uint32_t x = 0; x < the_width - 1; ++ x)
					{
						dx[y * the_width + x] = heights[y * the_width + (x + 1)] - heights[y * the_width + x];
					}
					dx[y * the_width + (the_width - 1)] = 0;
				}

				std::vector<int> dy;
				dy.resize(heights.size());
				for (uint32_t x = 0; x < the_width; ++ x)
				{
					for (uint32_t y = 0; y < the_height - 1; ++ y)
					{
						dy[y * the_width + x] = heights[(y + 1) * the_width + x] - heights[y * the_width + x];
					}
					dy[(the_height - 1) * the_width + x] = 0;
				}

				normals[i].data.reserve(the_width * the_height * 4);
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					for (uint32_t x = 0; x < the_width; ++ x)
					{
						float3 normal = MathLib::normalize(float3(static_cast<float>(-dx[y * the_width + x]),
							static_cast<float>(-dy[y * the_width + x]), 8));
						normal = normal * 0.5f + float3(0.5f, 0.5f, 0.5f);

						normals[0].data.push_back(static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.z() * 255 + 0.5f), 0, 255)));
						normals[0].data.push_back(static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.y() * 255 + 0.5f), 0, 255)));
						normals[0].data.push_back(static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.x() * 255 + 0.5f), 0, 255)));
						normals[0].data.push_back(255);
					}
				}

				the_width /= 2;
				the_height /= 2;
			}

			SaveTexture(out_file, type, width, height, depth, numMipMaps, EF_ARGB8, normals);
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

	if (argc != 3)
	{
		cout << "使用方法: NormalMapGen xxx.dds yyy.dds" << endl;
		return 1;
	}

	CreateNormalMap(argv[1], argv[2]);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
