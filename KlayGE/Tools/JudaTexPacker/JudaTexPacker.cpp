#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/XMLDom.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/typeof/typeof.hpp>

#include <KlayGE/JudaTexture.hpp>

using namespace std;
using namespace KlayGE;

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp()
		: App3DFramework("JudaTexPacker")
	{
	}

	void DoUpdateOverlay()
	{
	}

	uint32_t DoUpdate(uint32_t /*pass*/)
	{
		return URV_Finished;
	}
};

void PackJTML(std::string const & jtml_name)
{
	Timer timer;

	ResIdentifierPtr jtml = ResLoader::Instance().Load(jtml_name);

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(jtml);

	uint32_t num_tiles = root->AttribInt("num_tiles", 2048);
	uint32_t tile_size = root->AttribInt("tile_size", 256);
	std::string fmt_str = root->AttribString("format", "");
	ElementFormat format = EF_ARGB8;
	if ("ARGB8" == fmt_str)
	{
		format = EF_ARGB8;
	}
	else if ("ABGR8" == fmt_str)
	{
		format = EF_ABGR8;
	}
	uint32_t pixel_size = NumFormatBytes(format);

	JudaTexturePtr juda_tex(new JudaTexture(num_tiles, tile_size, format));

	uint32_t level = juda_tex->TreeLevels() - 1;

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	for (XMLNodePtr node = root->FirstNode("image"); node; node = node->NextSibling("image"))
	{
		timer.restart();

		std::string name = node->AttribString("name", "");
		uint32_t x = node->AttribInt("x", 0);
		uint32_t y = node->AttribInt("y", 0);

		cout << "Processing " << name << "... ";

		TexturePtr src_texture = LoadTexture(name, EAH_CPU_Read | EAH_CPU_Write)();
		if (src_texture->Type() != Texture::TT_2D)
		{
			cout << "Texture " << name << "is not 2D texture. Skipped." << endl;
			continue;
		}

		uint32_t in_width = src_texture->Width(0);
		uint32_t in_height = src_texture->Height(0);

		TexturePtr texture = rf.MakeTexture2D(in_width, in_height, 1, 1, format, 1, 0, EAH_CPU_Read | EAH_CPU_Write, NULL);
		src_texture->CopyToTexture(*texture);

		Texture::Mapper mapper(*texture, 0, 0, TMA_Read_Only, 0, 0, in_width, in_height);
		uint8_t const * in_data_p = mapper.Pointer<uint8_t>();

		uint32_t in_num_tiles_x = std::min((in_width + tile_size - 1) / tile_size, num_tiles);
		uint32_t in_num_tiles_y = std::min((in_height + tile_size - 1) / tile_size, num_tiles);

		std::vector<std::vector<uint8_t> > tiles(in_num_tiles_x);
		std::vector<uint32_t> tile_ids(tiles.size());
		for (size_t by = 0; by < in_num_tiles_y; ++ by)
		{
			for (size_t bx = 0; bx < in_num_tiles_x; ++ bx)
			{
				tiles[bx].assign(tile_size * tile_size * pixel_size, 0);
				tile_ids[bx] = juda_tex->EncodeTileID(level, bx + x, by + y);
				for (size_t dy = 0; dy < tile_size; ++ dy)
				{
					if (dy + by * tile_size < in_height)
					{
						for (size_t dx = 0; dx < tile_size; ++ dx)
						{
							if (dx + bx * tile_size < in_width)
							{
								memcpy(&tiles[bx][(dy * tile_size + dx) * pixel_size],
									&in_data_p[(dy + by * tile_size) * mapper.RowPitch() + (dx + bx * tile_size) * pixel_size],
									pixel_size);
							}
						}
					}
				}
			}

			juda_tex->CommitTiles(tiles, tile_ids);
		}

		cout << "Takes " << timer.elapsed() << "s" << endl;
	}

	cout << "Total tiles: " << juda_tex->NumTiles() * juda_tex->NumTiles() << endl;
	cout << "Non empty tiles: " << juda_tex->NumNonEmptyNodes() << endl;
	cout << "Tree depth: " << juda_tex->TreeLevels() << endl;

	std::string base_name = jtml_name.substr(0, jtml_name.find_last_of('.'));

	cout << "Saving... ";
	timer.restart();
	SaveJudaTexture(juda_tex, base_name + ".jdt");
	cout << "Takes " << timer.elapsed() << "s" << endl << endl;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: JudaTexPacker xxx.jtml" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	Context::Instance().LoadCfg("KlayGE.cfg");
	
	EmptyApp app;
	app.Create();

	PackJTML(argv[1]);

	return 0;
}
