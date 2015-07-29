#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

namespace
{
	void Bump2NormalMapSubresource(uint32_t width, uint32_t height, std::vector<Color>& in_data,
		ElementInitData& new_data, std::vector<uint8_t>& new_data_block, float offset)
	{
		new_data.row_pitch = width * 4;
		new_data.slice_pitch = new_data.row_pitch * height;
		new_data_block.resize(new_data.slice_pitch);
		new_data.data = &new_data_block[0];

		uint8_t* normals = &new_data_block[0];

		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float3 n;
				n.x() = (in_data[y * width + x].r() * 2 - 1) * offset;
				n.y() = (in_data[y * width + x].g() * 2 - 1) * offset;
				n.z() = 1;
				n = MathLib::normalize(n);

				normals[(y * width + x) * 4 + 0] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.x() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
				normals[(y * width + x) * 4 + 1] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.y() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
				normals[(y * width + x) * 4 + 2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>((n.z() * 0.5f + 0.5f) * 255.0f + 0.5f), 0, 255));
				normals[(y * width + x) * 4 + 3] = 255;
			}
		}
	}

	void Bump2NormalMap(std::string const & in_file, std::string const & out_file, float offset)
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

		std::vector<std::vector<Color>> in_color(in_data.size());
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
		std::vector<std::vector<uint8_t>> new_data_block(in_data.size());

		for (size_t sub_res = 0; sub_res < in_data.size(); ++ sub_res)
		{
			uint32_t the_width = in_data[sub_res].row_pitch / 4;
			uint32_t the_height = in_data[sub_res].slice_pitch / in_data[sub_res].row_pitch;

			Bump2NormalMapSubresource(the_width, the_height, in_color[sub_res], new_data[sub_res], new_data_block[sub_res], offset);
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, EF_ABGR8, new_data);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: Bump2Normal xxx.dds yyy.dds [offset=0.5]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	float offset = 0.5f;
	if (argc >= 4)
	{
		offset = static_cast<float>(atof(argv[3]));
	}

	Bump2NormalMap(in_file, argv[2], offset);

	cout << "Normal map is saved to " << argv[2] << endl;

	Context::Destroy();

	return 0;
}
