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

	TexturePtr CompressNormalMap(TexturePtr normal_map)
	{
		uint32_t const width = normal_map->Width(0);
		uint32_t const height = normal_map->Height(0);

		std::vector<uint8_t> normals(width * height * 4);
		normal_map->CopyToMemory2D(0, &normals[0]);

		std::vector<uint8_t> normal_xys(width * height * 4);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				normal_xys[(y * width + x) * 4 + 0] = 0;
				normal_xys[(y * width + x) * 4 + 1] = normals[(y * width + x) * 4 + 1];
				normal_xys[(y * width + x) * 4 + 2] = 0;
				normal_xys[(y * width + x) * 4 + 3] = normals[(y * width + x) * 4 + 2];
			}
		}

		TexturePtr new_normal_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, EF_DXT5);
		new_normal_map->CopyMemoryToTexture2D(0, &normal_xys[0], EF_ARGB8, width, height, 0, 0, width, height);
		return new_normal_map;
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
		cout << "使用方法: NormalMapCompressor xxx.dds yyy.dds" << endl;
		return 1;
	}

	EmptyApp app;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;

	app.Create("NormalMapCompressor", settings);

	TexturePtr temp = LoadTexture(argv[1]);
	TexturePtr normal_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(temp->Width(0), temp->Height(0), 1, EF_ARGB8);
	temp->CopyToTexture(*normal_map);
	TexturePtr new_normal_map = CompressNormalMap(normal_map);
	SaveTexture(new_normal_map, argv[2]);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
