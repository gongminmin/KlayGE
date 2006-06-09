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

		std::vector<uint8_t> heights(width, height);
		height_map->CopyToMemory2D(0, &heights[0]);

		std::vector<char> dx;
		dx.resize(heights.size());
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width - 1; ++ x)
			{
				dx[y * width + x] = heights[y * width + (x + 1)] - heights[y * width + x];
			}
			dx[y * width + (width - 1)] = 0;
		}

		std::vector<char> dy;
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
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float3 normal = MathLib::normalize(float3(dx[y * width + x] / 255.0f, dy[y * width + x] / 255.0f, 1.0f));
				normal = normal * 0.5f + float3(0.5f, 0.5f, 0.5f);

				normals.push_back(static_cast<uint8_t>(normal.z() * 255));
				normals.push_back(static_cast<uint8_t>(normal.y() * 255));
				normals.push_back(static_cast<uint8_t>(normal.x() * 255));
				normals.push_back(255);
			}
		}

		TexturePtr normal_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, EF_ARGB8);
		normal_map->CopyMemoryToTexture2D(0, &normals[0], EF_ARGB8, width, height, 0, 0, width, height);
		return normal_map;
	}
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	void DoUpdate(uint32_t /*pass*/)
	{
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

	EmptyApp app;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("NormalMapGen", settings);

	TexturePtr temp = LoadTexture(argv[1]);
	TexturePtr height_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(temp->Width(0), temp->Height(0), 1, EF_ARGB8);
	temp->CopyToTexture(*height_map);
	TexturePtr normal_map = CreateNormalMap(height_map);
	SaveTexture(normal_map, argv[2]);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
