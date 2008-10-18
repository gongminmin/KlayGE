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

	TexturePtr CreateNormalMap(TexturePtr height_map)
	{
		uint32_t const width = height_map->Width(0);
		uint32_t const height = height_map->Height(0);

		std::vector<uint8_t> heights(width * height);

		{
			Texture::Mapper mapper(*height_map, 0, TMA_Read_Only, 0, 0, width, height);
			uint8_t* data = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < height; ++ y)
			{
				memcpy(&heights[y * width], data, width * height_map->Bpp() / 8);
				data += mapper.RowPitch();
			}
		}

		std::vector<int> dx;
		dx.resize(heights.size());
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width - 1; ++ x)
			{
				dx[y * width + x] = heights[y * width + (x + 1)] - heights[y * width + x];
			}
			dx[y * width + (width - 1)] = 0;
		}

		std::vector<int> dy;
		dy.resize(heights.size());
		for (uint32_t x = 0; x < width; ++ x)
		{
			for (uint32_t y = 0; y < height - 1; ++ y)
			{
				dy[y * width + x] = heights[(y + 1) * width + x] - heights[y * width + x];
			}
			dy[(height - 1) * width + x] = 0;
		}

		std::vector<uint8_t> normals;
		normals.reserve(width * height * 4);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float3 normal = MathLib::normalize(float3(-dx[y * width + x], -dy[y * width + x], 8));
				normal = normal * 0.5f + float3(0.5f, 0.5f, 0.5f);

				normals.push_back(static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.z() * 255 + 0.5f), 0, 255)));
				normals.push_back(static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.y() * 255 + 0.5f), 0, 255)));
				normals.push_back(static_cast<uint8_t>(MathLib::clamp(static_cast<int>(normal.x() * 255 + 0.5f), 0, 255)));
				normals.push_back(255);
			}
		}

		TexturePtr normal_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, EF_ARGB8, EAH_CPU_Read | EAH_CPU_Write, NULL);
		{
			Texture::Mapper mapper(*normal_map, 0, TMA_Write_Only, 0, 0, width, height);
			uint8_t* data = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < height; ++ y)
			{
				memcpy(data, &normals[y * width * 4], width * normal_map->Bpp() / 8);
				data += mapper.RowPitch();
			}
		}

		return normal_map;
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

	if (argc != 3)
	{
		cout << "使用方法: NormalMapGen xxx.dds yyy.dds" << endl;
		return 1;
	}

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;

	EmptyApp app("NormalMapGen", settings);
	app.Create();

	TexturePtr temp = LoadTexture(argv[1], EAH_CPU_Read | EAH_CPU_Write);
	TexturePtr height_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(temp->Width(0), temp->Height(0), 1, EF_L8, EAH_CPU_Read | EAH_CPU_Write, NULL);
	temp->CopyToTexture(*height_map);
	TexturePtr normal_map = CreateNormalMap(height_map);
	SaveTexture(normal_map, argv[2]);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
