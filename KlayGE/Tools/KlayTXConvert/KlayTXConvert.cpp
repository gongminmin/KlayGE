#include <KlayGE/KlayGE.hpp>
#include <KlayGE/CommFuncs.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>

#include <KlayGE/KlayTX/KlayTX.hpp>

namespace
{
	using namespace KlayGE;

#pragma pack(push, 1)

	struct TGAHeader
	{
		U8		infoLength;
		U8		colorMapType;
		U8		type;

		short	colorMapEntry;
		short	colorMapLength;
		U8		colorMapBits;

		short	leftbottomX;
		short	leftbottomY;

		short	width;
		short	height;
		U8		bits;
		U8		descript;
	};

#pragma pack(pop)


	void LoadTGA(TGAHeader& tgaHeader, std::vector<U8>& tgaData, const KlayGE::WString& fileName)
	{
		DiskFile file(fileName, VFile::OM_Read);
		file.Read(&tgaHeader, sizeof(tgaHeader));
		file.Seek(tgaHeader.infoLength, VFile::SM_Current);

		std::vector<U8> data(tgaHeader.width * tgaHeader.height * tgaHeader.bits / 8);
		file.Read(&data[0], data.size());

		tgaData.clear();
		tgaData.reserve(tgaHeader.width * tgaHeader.height * 4);

		for (short y = 0; y < tgaHeader.height; ++ y)
		{
			for (short x = 0; x < tgaHeader.width; ++ x)
			{
				size_t offset;
				if (tgaHeader.descript & 0x20)
				{
					// 图像从上到下
					offset = y * tgaHeader.width + x;
				}
				else
				{
					// 图像从下到上
					offset = (tgaHeader.height - y - 1) * tgaHeader.width + x;
				}

				tgaData.push_back(data[offset * (tgaHeader.bits / 8) + 0]);
				tgaData.push_back(data[offset * (tgaHeader.bits / 8) + 1]);
				tgaData.push_back(data[offset * (tgaHeader.bits / 8) + 2]);

				switch (tgaHeader.bits)
				{
				case 24:
					tgaData.push_back(0xFF);
					break;

				case 32:
					tgaData.push_back(offset * (tgaHeader.bits / 8) + 3);
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
		return 1;
	}

	WString tgaFileName;
	Convert(tgaFileName, argv[1]);

	TGAHeader tgaHeader;
	std::vector<U8> tgaData;
	LoadTGA(tgaHeader, tgaData, tgaFileName);

	KlayTXHeader txHeader;
	txHeader.id = MakeFourCC<'K', 'l', 'T', 'X'>::value;
	txHeader.width = tgaHeader.width;
	txHeader.height = tgaHeader.height;
	switch (tgaHeader.bits)
	{
	case 24:
		txHeader.format = PF_X8R8G8B8;
		break;

	case 32:
		txHeader.format = PF_A8R8G8B8;
		break;
	}
	txHeader.offset = 0;

	WString txFileName;
	Convert(txFileName, argv[2]);
	DiskFile file(txFileName, VFile::OM_Create);
	file.Write(&txHeader, sizeof(txHeader));
	file.Write(&tgaData[0], tgaData.size());

	return 0;
}
