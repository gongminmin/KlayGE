#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

	void CreateNormalMap(std::string const & in_file, std::string const & out_file, float scale)
	{
		TexturePtr in_tex = LoadSoftwareTexture(in_file);
		auto const type = in_tex->Type();
		auto const width = in_tex->Width(0);
		auto const height = in_tex->Height(0);
		auto const depth = in_tex->Depth(0);
		auto const num_mipmaps = in_tex->NumMipMaps();
		auto const array_size = in_tex->ArraySize();
		auto const format = in_tex->Format();
		auto const & in_data = checked_cast<SoftwareTexture*>(in_tex.get())->SubresourceData();

		if ((Texture::TT_2D == type) && (EF_R8 == format))
		{
			uint32_t the_width = width;
			uint32_t the_height = height;

			std::vector<ElementInitData> normals(in_data.size());
			std::vector<uint8_t> data_block;
			std::vector<size_t> base(in_data.size());
			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				std::vector<uint8_t> heights(the_width * the_height);
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					for (uint32_t x = 0; x < the_width; ++ x)
					{
						heights[y * the_width + x] = static_cast<uint8_t const *>(in_data[i].data)[y * in_data[i].row_pitch + x];
					}
				}

				std::vector<int> dx;
				dx.resize(heights.size());
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					for (uint32_t x = 0; x < the_width; ++ x)
					{
						int x0 = x;
						int x1 = (x0 + 1) % the_width;
						dx[y * the_width + x] = heights[y * the_width + x1] - heights[y * the_width + x0];
					}
				}

				std::vector<int> dy;
				dy.resize(heights.size());
				for (uint32_t x = 0; x < the_width; ++ x)
				{
					for (uint32_t y = 0; y < the_height; ++ y)
					{
						int y0 = y;
						int y1 = (y0 + 1) % the_height;
						dy[y * the_width + x] = heights[y1 * the_width + x] - heights[y0 * the_width + x];
					}
				}

				base[i] = data_block.size();
				data_block.resize(data_block.size() + the_width * the_height * 4);
				for (uint32_t y = 0; y < the_height; ++ y)
				{
					for (uint32_t x = 0; x < the_width; ++ x)
					{
						float3 normal = MathLib::normalize(float3(-dx[y * the_width + x] / 255.0f,
							-dy[y * the_width + x] / 255.0f, scale));
						normal = normal * 0.5f + float3(0.5f, 0.5f, 0.5f);

						data_block[base[i] + (y * the_width + x) * 4 + 0] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.z() * 255 + 0.5f), 0, 255));
						data_block[base[i] + (y * the_width + x) * 4 + 1] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.y() * 255 + 0.5f), 0, 255));
						data_block[base[i] + (y * the_width + x) * 4 + 2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.x() * 255 + 0.5f), 0, 255));
						data_block[base[i] + (y * the_width + x) * 4 + 3] = 255;
					}
				}

				the_width /= 2;
				the_height /= 2;
			}

			for (size_t i = 0; i < in_data.size(); ++ i)
			{
				normals[i].data = &data_block[base[i]];
				normals[i].row_pitch = width * 4;
				normals[i].slice_pitch = normals[i].row_pitch * height;
			}

			TexturePtr out_tex = MakeSharedPtr<SoftwareTexture>(type, width, height, depth, num_mipmaps, array_size, EF_ARGB8, true);
			out_tex->CreateHWResource(normals, nullptr);
			SaveTexture(out_tex, out_file);
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
		cout << "Usage: NormalMapGen xxx.dds yyy.dds [scale=8.0]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}
	
	float scale = 1;
	if (argc >= 4)
	{
		scale = static_cast<float>(atof(argv[3]));
	}

	CreateNormalMap(in_file, argv[2], scale);

	cout << "Normal map is saved to " << argv[2] << endl;

	Context::Destroy();

	return 0;
}
