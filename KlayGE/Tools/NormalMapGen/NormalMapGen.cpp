#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

#pragma pack(push, 1)

	struct TGAHeader
	{
		uint8_t	infoLength;
		uint8_t	colorMapType;
		uint8_t	imageTypeCode;

		int16_t	colorMapEntry;
		int16_t	colorMapLength;
		uint8_t	colorMapBits;

		int16_t	leftbottomX;
		int16_t	leftbottomY;

		int16_t	width;
		int16_t	height;

		uint8_t	pixelSize;
		uint8_t	imageDescriptor;
	};

#pragma pack(pop)


	void LoadHeightMap(TGAHeader& tgaHeader, std::vector<uint8_t>& tgaData, std::string const & fileName)
	{
		ifstream file(fileName.c_str(), std::ios_base::binary);
		file.read(reinterpret_cast<char*>(&tgaHeader), sizeof(tgaHeader));
		file.seekg(tgaHeader.infoLength, std::ios_base::cur);

		std::vector<uint8_t> data(tgaHeader.width * tgaHeader.height * tgaHeader.pixelSize / 8);
		file.read(reinterpret_cast<char*>(&data[0]), data.size());

		tgaData.clear();
		tgaData.reserve(tgaHeader.width * tgaHeader.height);

		for (short y = 0; y < tgaHeader.height; ++ y)
		{
			short line(y);
			if (0 == (tgaHeader.imageDescriptor & 0x20))
			{
				// 图像从下到上
				line = tgaHeader.height - y - 1;
			}

			for (short x = 0; x < tgaHeader.width; ++ x)
			{
				size_t const offset((line * tgaHeader.width + x) * (tgaHeader.pixelSize / 8));

				tgaData.push_back(data[offset]);
			}
		}
	}

	void SaveNormalMap(TGAHeader& tgaHeader, std::vector<uint8_t>& tgaData, std::string const & fileName)
	{
		std::ofstream file(fileName.c_str(), std::ios_base::binary);
		file.write(reinterpret_cast<char*>(&tgaHeader), sizeof(tgaHeader));
		file.seekp(tgaHeader.infoLength, std::ios_base::cur);

		file.write(reinterpret_cast<char*>(&tgaData[0]), tgaData.size());
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc != 3)
	{
		cout << "使用方法: NormalMapGen xxx.tga yyy.tga" << endl;
		return 1;
	}

	TGAHeader header;
	std::vector<uint8_t> heightmap;
	LoadHeightMap(header, heightmap, argv[1]);

	std::vector<char> dx;
	dx.resize(heightmap.size());
	for (int y = 0; y < header.height; ++ y)
	{
		for (int x = 0; x < header.width - 1; ++ x)
		{
			dx[y * header.width + x] = heightmap[y * header.width + (x + 1)] - heightmap[y * header.width + x];
		}
		dx[y * header.width + (header.width - 1)] = 0;
	}

	std::vector<char> dy;
	dy.resize(heightmap.size());
	for (int x = 0; x < header.width; ++ x)
	{
		for (int y = 0; y < header.height - 1; ++ y)
		{
			dy[y * header.width + x] = heightmap[(y + 1) * header.width + x] - heightmap[y * header.width + x];
		}
		dy[(header.height - 1) * header.width + x] = 0;
	}

	std::vector<uint8_t> normalmap;
	for (int y = 0; y < header.height; ++ y)
	{
		for (int x = 0; x < header.width; ++ x)
		{
			Vector3 normal;
			MathLib::Normalize(normal,
				Vector3(dx[y * header.width + x] / 255.0f, dy[y * header.width + x] / 255.0f, 1.0f));

			normal = normal * 0.5f + Vector3(0.5f, 0.5f, 0.5f);

			normalmap.push_back(static_cast<uint8_t>(normal.z() * 255));
			normalmap.push_back(static_cast<uint8_t>(normal.y() * 255));
			normalmap.push_back(static_cast<uint8_t>(normal.x() * 255));
		}
	}

	header.imageDescriptor |= 0x20;
	SaveNormalMap(header, normalmap, argv[2]);

	return 0;
}
