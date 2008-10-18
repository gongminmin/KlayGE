#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

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

	void DecompressNormal(std::vector<uint8_t>& res_normals, std::vector<uint8_t>& com_normals)
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

	TexturePtr CompressNormalMapCube(TexturePtr normal_map, ElementFormat new_format)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		uint32_t const size = normal_map->Width(0);
		TexturePtr new_normal_map = rf.MakeTextureCube(size, 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
		std::vector<uint8_t> normals(size * size * 4);

		for (int i = 0; i < 6; ++ i)
		{
			{
				Texture::Mapper mapper(*normal_map, static_cast<Texture::CubeFaces>(i), 0, TMA_Read_Only, 0, 0, size, size);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < size; ++ y)
				{
					memcpy(&normals[y * size * 4], data, size * normal_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}

			CompressNormal(normals);

			{
				Texture::Mapper mapper(*new_normal_map, static_cast<Texture::CubeFaces>(i), 0, TMA_Write_Only, 0, 0, size, size);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < size; ++ y)
				{
					memcpy(data, &normals[y * size * 4], size * new_normal_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}
		}

		TexturePtr com_normal_map = rf.MakeTextureCube(size, 1, new_format, EAH_CPU_Read | EAH_CPU_Write, NULL);
		new_normal_map->CopyToTexture(*com_normal_map);

		float mse = 0;
		{
			TexturePtr normal_map_restored = rf.MakeTextureCube(size, 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
			com_normal_map->CopyToTexture(*normal_map_restored);

			std::vector<uint8_t> restored_normals(size * size * 4);

			for (int i = 0; i < 6; ++ i)
			{
				{
					Texture::Mapper mapper(*normal_map_restored, static_cast<Texture::CubeFaces>(i), 0, TMA_Read_Only, 0, 0, size, size);
					uint8_t* data = mapper.Pointer<uint8_t>();
					for (uint32_t y = 0; y < size; ++ y)
					{
						memcpy(&normals[y * size * 4], data, size * normal_map->Bpp() / 8);
						data += mapper.RowPitch();
					}
				}

				DecompressNormal(restored_normals, normals);

				{
					Texture::Mapper mapper(*normal_map, static_cast<Texture::CubeFaces>(i), 0, TMA_Read_Only, 0, 0, size, size);
					uint8_t* data = mapper.Pointer<uint8_t>();
					for (uint32_t y = 0; y < size; ++ y)
					{
						memcpy(&normals[y * size * 4], data, size * normal_map->Bpp() / 8);
						data += mapper.RowPitch();
					}
				}

				for (uint32_t y = 0; y < size; ++ y)
				{
					for (uint32_t x = 0; x < size; ++ x)
					{
						float diff_r = (normals[(y * size + x) * 4 + 0] - restored_normals[(y * size + x) * 4 + 0]) / 255.0f;
						float diff_g = (normals[(y * size + x) * 4 + 1] - restored_normals[(y * size + x) * 4 + 1]) / 255.0f;
						float diff_b = (normals[(y * size + x) * 4 + 2] - restored_normals[(y * size + x) * 4 + 2]) / 255.0f;

						mse += diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
					}
				}
			}
		}

		cout << "MSE: " << mse << endl;

		return new_normal_map;
	}

	TexturePtr CompressNormalMap2D(TexturePtr normal_map, ElementFormat new_format)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		uint32_t const width = normal_map->Width(0);
		uint32_t const height = normal_map->Height(0);
		TexturePtr new_normal_map = rf.MakeTexture2D(width, height, 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
		std::vector<uint8_t> normals(width * height * 4);

		{
			{
				Texture::Mapper mapper(*normal_map, 0, TMA_Read_Only, 0, 0, width, height);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < height; ++ y)
				{
					memcpy(&normals[y * width * 4], data, width * normal_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}

			CompressNormal(normals);

			{
				Texture::Mapper mapper(*new_normal_map, 0, TMA_Write_Only, 0, 0, width, height);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < height; ++ y)
				{
					memcpy(data, &normals[y * width * 4], width * new_normal_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}
		}

		TexturePtr com_normal_map = rf.MakeTexture2D(width, height, 1, new_format, EAH_CPU_Read | EAH_CPU_Write, NULL);
		new_normal_map->CopyToTexture(*com_normal_map);

		float mse = 0;
		{
			TexturePtr normal_map_restored = rf.MakeTexture2D(width, height, 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
			com_normal_map->CopyToTexture(*normal_map_restored);

			std::vector<uint8_t> restored_normals(width * height * 4);

			{
				Texture::Mapper mapper(*normal_map_restored, 0, TMA_Read_Only, 0, 0, width, height);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < height; ++ y)
				{
					memcpy(&normals[y * width * 4], data, width * normal_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}

			DecompressNormal(restored_normals, normals);

			{
				Texture::Mapper mapper(*normal_map, 0, TMA_Read_Only, 0, 0, width, height);
				uint8_t* data = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < height; ++ y)
				{
					memcpy(&normals[y * width * 4], data, width * normal_map->Bpp() / 8);
					data += mapper.RowPitch();
				}
			}

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
		}

		cout << "MSE: " << mse << endl;

		return com_normal_map;
	}

	TexturePtr CompressNormalMap(TexturePtr normal_map, ElementFormat new_format)
	{
		TexturePtr new_normal_map;

		switch (normal_map->Type())
		{
		case Texture::TT_2D:
			{
				TexturePtr temp = Context::Instance().RenderFactoryInstance().MakeTexture2D(
					normal_map->Width(0), normal_map->Height(0), 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
				normal_map->CopyToTexture(*temp);
				new_normal_map = CompressNormalMap2D(temp, new_format);
			}
			break;

		case Texture::TT_Cube:
			{
				TexturePtr temp = Context::Instance().RenderFactoryInstance().MakeTextureCube(
					normal_map->Width(0), 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
				normal_map->CopyToTexture(*temp);
				new_normal_map = CompressNormalMapCube(temp, new_format);
			}
			break;

		default:
			cout << "Unsupported texture type" << endl;
			break;
		}

		return new_normal_map;
	}
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp(std::string const & name, KlayGE::RenderSettings const & settings)
		: App3DFramework(name, settings)
	{
	}

	uint32_t DoUpdate(uint32_t /*pass*/)
	{
		return URV_Finished;
	}
};

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 3)
	{
		cout << "使用方法: NormalMapCompressor xxx.dds yyy.dds [AL8 | DXT5]" << endl;
		return 1;
	}

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;

	EmptyApp app("NormalMapCompressor", settings);
	app.Create();

	ElementFormat new_format = EF_BC3;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("AL8" == format_str)
		{
			new_format = EF_AL8;
		}
	}

	TexturePtr new_normal_map = CompressNormalMap(LoadTexture(argv[1], EAH_CPU_Read | EAH_CPU_Write), new_format);
	SaveTexture(new_normal_map, argv[2]);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
