#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>

#include <iostream>

#include <KlayGE/KlayTX/KlayTX.hpp>

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


	void LoadTGA(TGAHeader& tgaHeader, std::vector<U8>& tgaData, const std::string& fileName)
	{
		DiskFile file(fileName, VFile::OM_Read);
		file.Read(&tgaHeader, sizeof(tgaHeader));
		file.Seek(tgaHeader.infoLength, VFile::SM_Current);

		std::vector<U8> data(tgaHeader.width * tgaHeader.height * tgaHeader.pixelSize / 8);
		file.Read(&data[0], data.size());

		tgaData.clear();
		tgaData.reserve(tgaHeader.width * tgaHeader.height * 4);

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
				const size_t offset((line * tgaHeader.width + x) * (tgaHeader.pixelSize / 8));

				tgaData.push_back(data[offset + 0]);
				tgaData.push_back(data[offset + 1]);
				tgaData.push_back(data[offset + 2]);

				switch (tgaHeader.pixelSize)
				{
				case 24:
					tgaData.push_back(0xFF);
					break;

				case 32:
					tgaData.push_back(data[offset + 3]);
					break;
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc != 3)
	{
		cout << "使用方法: KlayTXConvert xxx.tga xxx.kltx" << endl;
		return 1;
	}

	TGAHeader tgaHeader;
	std::vector<U8> tgaData;
	LoadTGA(tgaHeader, tgaData, argv[1]);

	KlayTXHeader txHeader;
	txHeader.id = MakeFourCC<'K', 'l', 'T', 'X'>::value;
	txHeader.width = tgaHeader.width;
	txHeader.height = tgaHeader.height;
	switch (tgaHeader.pixelSize)
	{
	case 24:
		txHeader.format = PF_X8R8G8B8;
		break;

	case 32:
		txHeader.format = PF_A8R8G8B8;
		break;
	}
	txHeader.offset = 0;

	DiskFile file(argv[2], VFile::OM_Create);
	file.Write(&txHeader, sizeof(txHeader));
	file.Write(&tgaData[0], tgaData.size());

	return 0;
}
