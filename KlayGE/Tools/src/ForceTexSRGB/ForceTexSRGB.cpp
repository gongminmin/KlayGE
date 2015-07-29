#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace KlayGE;

namespace
{
	void ForceTexSRGB(std::string const & in_file, std::string const & out_file)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		if (IsSRGB(in_format))
		{
			cout << "This texture is already in sRGB format." << endl;
			if (in_file != out_file)
			{
				SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data);
			}
			return;
		}
		ElementFormat new_format = MakeSRGB(in_format);
		if (new_format == in_format)
		{
			cout << "This texture format don't have a sRGB counterpart." << endl;
			return;
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, new_format, in_data);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: ForceTexSRGB xxx.dds [yyy.dds]" << endl;
		return 1;
	}

	std::string in_file = ResLoader::Instance().Locate(argv[1]);
	if (in_file.empty())
	{
		cout << "Couldn't locate " << in_file << endl;
		Context::Destroy();
		return 1;
	}

	std::string out_file;
	if (argc >= 3)
	{
		out_file = argv[2];
	}
	else
	{
		out_file = argv[1];
	}

	ForceTexSRGB(in_file, out_file);

	cout << "sRGB texture is saved to " << out_file << endl;

	Context::Destroy();

	return 0;
}
