#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>

#include <iostream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

#pragma pack(push, 1)

	struct TGAHeader
	{
		U8		infoLength;
		U8		colorMapType;
		U8		imageTypeCode;

		short	colorMapEntry;
		short	colorMapLength;
		U8		colorMapBits;

		short	leftbottomX;
		short	leftbottomY;

		short	width;
		short	height;

		U8		pixelSize;
		U8		imageDescriptor;
	};

#pragma pack(pop)


	void LoadHeightMap(TGAHeader& tgaHeader, std::vector<U8>& tgaData, std::string const & fileName)
	{
		DiskFile file(fileName, VFile::OM_Read);
		file.Read(&tgaHeader, sizeof(tgaHeader));
		file.Seek(tgaHeader.infoLength, VFile::SM_Current);

		std::vector<U8> data(tgaHeader.width * tgaHeader.height * tgaHeader.pixelSize / 8);
		file.Read(&data[0], data.size());

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

	void SaveNormalMap(TGAHeader& tgaHeader, std::vector<U8>& tgaData, std::string const & fileName)
	{
		DiskFile file(fileName, VFile::OM_Write);
		file.Write(&tgaHeader, sizeof(tgaHeader));
		file.Seek(tgaHeader.infoLength, VFile::SM_Current);

		file.Write(&tgaData[0], tgaData.size());
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
	std::vector<U8> heightmap;
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

	std::vector<U8> normalmap;
	for (int y = 0; y < header.height; ++ y)
	{
		for (int x = 0; x < header.width; ++ x)
		{
			Vector3 normal;
			MathLib::Normalize(normal,
				Vector3(dx[y * header.width + x] / 255.0f, dy[y * header.width + x] / 255.0f, 1.0f));

			normal = normal * 0.5f + Vector3(0.5f, 0.5f, 0.5f);

			normalmap.push_back(static_cast<U8>(normal.z() * 255));
			normalmap.push_back(static_cast<U8>(normal.y() * 255));
			normalmap.push_back(static_cast<U8>(normal.x() * 255));
		}
	}

	header.imageDescriptor |= 0x20;
	SaveNormalMap(header, normalmap, argv[2]);

	return 0;
}
