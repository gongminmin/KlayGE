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
		TexturePtr in_tex = LoadSoftwareTexture(in_file);
		auto const in_type = in_tex->Type();
		auto const in_width = in_tex->Width(0);
		auto const in_height = in_tex->Height(0);
		auto const in_depth = in_tex->Depth(0);
		auto const in_num_mipmaps = in_tex->NumMipMaps();
		auto const in_array_size = in_tex->ArraySize();
		auto const in_format = in_tex->Format();
		auto const & in_data = checked_cast<SoftwareTexture*>(in_tex.get())->SubresourceData();

		if (IsSRGB(in_format))
		{
			cout << "This texture is already in sRGB format." << endl;
			if (in_file != out_file)
			{
				SaveTexture(in_tex, out_file);
			}
			return;
		}
		ElementFormat new_format = MakeSRGB(in_format);
		if (new_format == in_format)
		{
			cout << "This texture format don't have a sRGB counterpart." << endl;
			return;
		}

		TexturePtr out_tex = MakeSharedPtr<SoftwareTexture>(in_type, in_width, in_height, in_depth,
			in_num_mipmaps, in_array_size, new_format, true);
		out_tex->CreateHWResource(in_data, nullptr);
		SaveTexture(out_tex, out_file);
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
