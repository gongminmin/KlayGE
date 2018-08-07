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
		TexturePtr in_tex = LoadSoftwareTexture(in_file);
		auto const in_num_mipmaps = in_tex->NumMipMaps();
		auto const in_format = in_tex->Format();

		uint32_t const array_size = in_tex->ArraySize() * ((in_tex->Type() == Texture::TT_Cube) ? 6 : 1);

		std::vector<std::vector<Color>> pixels(array_size * in_num_mipmaps);
		for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
		{
			for (uint32_t level = 0; level < in_num_mipmaps; ++ level)
			{
				uint32_t const subres = array_index * in_num_mipmaps + level;
				uint32_t const width = in_tex->Width(level);
				uint32_t const height = in_tex->Height(level);
				uint32_t const depth = in_tex->Depth(level);

				pixels[subres].resize(width * height * depth);

				Color* dst = pixels[subres].data();
				switch (in_tex->Type())
				{
				case Texture::TT_1D:
					{
						Texture::Mapper mapper_1d(*in_tex, array_index, level, TMA_Read_Only, 0, width);
						auto const * src = mapper_1d.Pointer<uint8_t>();
						ConvertToABGR32F(in_format, src, width, dst);
						src += mapper_1d.RowPitch();
						dst += width;
						break;
					}

				case Texture::TT_2D:
					{
						Texture::Mapper mapper_2d(*in_tex, array_index, level, TMA_Read_Only, 0, 0, width, height);
						auto const * src = mapper_2d.Pointer<uint8_t>();
						for (uint32_t y = 0; y < height; ++ y)
						{
							ConvertToABGR32F(in_format, src, width, dst);
							src += mapper_2d.RowPitch();
							dst += width;
						}
						break;
					}

				case Texture::TT_3D:
					{
						Texture::Mapper mapper_3d(*in_tex, array_index, level, TMA_Read_Only, 0, 0, 0, width, height, depth);
						auto const * src_ori = mapper_3d.Pointer<uint8_t>();
						for (uint32_t z = 0; z < depth; ++ z)
						{
							auto const * src = src_ori + z * mapper_3d.SlicePitch();
							for (uint32_t y = 0; y < height; ++ y)
							{
								ConvertToABGR32F(in_format, src, width, dst);
								src += mapper_3d.RowPitch();
								dst += width;
							}
						}
						break;
					}

				case Texture::TT_Cube:
					{
						uint32_t cube_array_index = array_index / 6;
						Texture::CubeFaces face = static_cast<Texture::CubeFaces>(array_index - cube_array_index * 6);
						Texture::Mapper mapper_cube(*in_tex, cube_array_index, face, level, TMA_Read_Only, 0, 0, width, height);
						auto const * src = mapper_cube.Pointer<uint8_t>();
						for (uint32_t y = 0; y < height; ++ y)
						{
							ConvertToABGR32F(in_format, src, width, dst);
							src += mapper_cube.RowPitch();
							dst += width;
						}
						break;
					}
				}
			}
		}

		std::vector<ElementInitData> new_data(pixels.size());
		std::vector<std::vector<uint8_t>> new_data_block(pixels.size());
		for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
		{
			for (uint32_t level = 0; level < in_num_mipmaps; ++ level)
			{
				uint32_t const subres = array_index * in_num_mipmaps + level;
				uint32_t const width = in_tex->Width(level);
				uint32_t const height = in_tex->Height(level);

				Bump2NormalMapSubresource(width, height, pixels[subres], new_data[subres], new_data_block[subres], offset);
			}
		}

		TexturePtr out_tex = MakeSharedPtr<SoftwareTexture>(in_tex->Type(), in_tex->Width(0), in_tex->Height(0), in_tex->Depth(0),
			in_num_mipmaps, in_tex->ArraySize(), EF_ABGR8, true);
		out_tex->CreateHWResource(new_data, nullptr);
		SaveTexture(out_tex, out_file);
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
