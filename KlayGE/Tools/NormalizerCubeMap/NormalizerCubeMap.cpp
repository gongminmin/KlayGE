#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
using namespace KlayGE;

D3DXVECTOR3 CubeVector(int face, int cube_size, int x, int y)
{
	float s = static_cast<float>(x) / cube_size;
	float t = static_cast<float>(y) / cube_size;
	float sc = s * 2.0f - 1.0f;
	float tc = t * 2.0f - 1.0f;

	D3DXVECTOR3 v;

	switch (face)
	{
	case 0:
		v.x = 1.0f;
		v.y = -tc;
		v.z = -sc;
		break;

	case 1:
		v.x = -1.0f;
		v.y = -tc;
		v.z = sc;
		break;

	case 2:
		v.x = sc;
		v.y = 1.0f;
		v.z = tc;
		break;

	case 3:
		v.x = sc;
		v.y = -1.0f;
		v.z = -tc;
		break;

	case 4:
		v.x = sc;
		v.y = -tc;
		v.z = 1.0f;
		break;

	case 5:
		v.x = -sc;
		v.y = -tc;
		v.z = -1.0f;
		break;
	}

	D3DXVec3Normalize(&v, &v);

	return v;
}

void VecToUChar(D3DXVECTOR3 const & v, unsigned char& x, unsigned char& y, unsigned char& z)
{
	x = static_cast<unsigned char>((v.x * 127 + 128));
	y = static_cast<unsigned char>((v.y * 127 + 128));
	z = static_cast<unsigned char>((v.z * 127 + 128));
}

TexturePtr CreateCubeMap(int cube_size, std::string const & cube_name)
{
	RenderFactory& render_factory(Context::Instance().RenderFactoryInstance());

	TexturePtr cube = render_factory.MakeTextureCube(cube_size, 1, PF_X8R8G8B8);

	for (int face = 0; face < 6; ++ face)
	{
		std::vector<uint8_t> data(cube_size * cube_size * 4);

		for (int y = 0; y < cube_size; ++ y)
		{
			for (int x = 0; x < cube_size; ++ x)
			{
				D3DXVECTOR3 v = CubeVector(face, cube_size, x, y);
				VecToUChar(v, data[(y * cube_size + x) * 4 + 2], data[(y * cube_size + x) * 4 + 1], data[(y * cube_size + x) * 4 + 0]);
				data[(y * cube_size + x) * 4 + 3] = 0xFF;
			}
		}

		cube->CopyMemoryToTextureCube(static_cast<Texture::CubeFaces>(face), 0, &data[0], PF_X8R8G8B8, cube_size, 0);
	}

	return cube;
}

int main(int argc, char* argv[])
{
	std::string cube_name("normalizer_cube.dds");
	int size = 128;

	if (argc > 1)
	{
		cube_name = argv[1];
	}
	if (argc > 2)
	{
		std::stringstream ss(argv[2]);
		ss >> size;
	}

	KlayGE::App3DFramework app;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	D3D9RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("NormalizerCubeMap", settings);

	TexturePtr cube = CreateCubeMap(size, cube_name);
	SaveToFile(cube, cube_name);

	cout << "Normalizer cube map is saved to " << cube_name << endl;
}
