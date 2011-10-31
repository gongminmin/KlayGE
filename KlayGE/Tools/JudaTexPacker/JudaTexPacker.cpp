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

struct address_calculator
{
	virtual int32_t operator()(int32_t x, uint32_t h) = 0;
};

struct calc_wrap : public address_calculator
{
	int32_t operator()(int32_t x, uint32_t h)
	{
		return (x + h) % h;
	}
};

struct calc_clamp : public address_calculator
{
	int32_t operator()(int32_t x, uint32_t h)
	{
		return (x > static_cast<int32_t>(h - 1)) ? h - 1 : ((x < 0) ? 0 : x);
	}
};

struct calc_mirror : public address_calculator
{
	int32_t operator()(int32_t x, uint32_t h)
	{
		int32_t selection_coord = static_cast<int32_t>(floor(static_cast<float>(x) / h));
		return (selection_coord & 1) ? h + selection_coord * h - x : x - selection_coord * h;
	}
};

struct calc_border : public address_calculator
{
	int32_t operator()(int32_t x, uint32_t h)
	{
		return (x > static_cast<int32_t>(h - 1)) ? -1 : ((x < 0) ? -1 : x);
	}
};

boost::shared_ptr<address_calculator> address_calculators[4] = 
{
	boost::shared_ptr<address_calculator>(new calc_wrap),
	boost::shared_ptr<address_calculator>(new calc_mirror),
	boost::shared_ptr<address_calculator>(new calc_clamp),
	boost::shared_ptr<address_calculator>(new calc_border)
};

void PackJTML(std::string const & jtml_name)
{
	Timer timer;

	ResIdentifierPtr jtml = ResLoader::Instance().Open(jtml_name);

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(jtml);

	uint32_t n = root->AttribInt("num_tiles", 2048);
	uint32_t num_tiles = 1;
	while (num_tiles * 2 <= n)
	{
		num_tiles *= 2;
	}

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
		int32_t x = node->AttribInt("x", 0);
		int32_t y = node->AttribInt("y", 0);
		std::string address_u_str = node->AttribString("address_u", "wrap");
		std::string address_v_str = node->AttribString("address_v", "wrap");
		Color border_clr;
		border_clr.r() = node->AttribFloat("border_r", 0.0f);
		border_clr.g() = node->AttribFloat("border_g", 0.0f);
		border_clr.b() = node->AttribFloat("border_b", 0.0f);
		border_clr.a() = node->AttribFloat("border_a", 0.0f);
		uint32_t border_clr_u8;
		switch (format)
		{
		case EF_ARGB8:
			border_clr_u8 = border_clr.ARGB();
			break;

		case EF_ABGR8:
			border_clr_u8 = border_clr.ABGR();
			break;

		default:
			border_clr_u8 = 0;
			break;
		}

		boost::shared_ptr<address_calculator> calc_u, calc_v;
		if ("wrap" == address_u_str)
		{
			calc_u = address_calculators[TAM_Wrap];
		}
		else
		{
			if ("mirror" == address_u_str)
			{
				calc_u = address_calculators[TAM_Mirror];
			}
			else
			{
				if ("clamp" == address_u_str)
				{
					calc_u = address_calculators[TAM_Clamp];
				}
				else
				{
					if ("border" == address_u_str)
					{
						calc_u = address_calculators[TAM_Border];
					}
					else
					{
						calc_u = address_calculators[TAM_Wrap];
					}
				}
			}
		}
		if ("wrap" == address_v_str)
		{
			calc_v = address_calculators[TAM_Wrap];
		}
		else
		{
			if ("mirror" == address_v_str)
			{
				calc_v = address_calculators[TAM_Mirror];
			}
			else
			{
				if ("clamp" == address_v_str)
				{
					calc_v = address_calculators[TAM_Clamp];
				}
				else
				{
					if ("border" == address_v_str)
					{
						calc_v = address_calculators[TAM_Border];
					}
					else
					{
						calc_v = address_calculators[TAM_Wrap];
					}
				}
			}
		}

		cout << "Processing " << name << "... ";

		TexturePtr src_texture = SyncLoadTexture(name, EAH_CPU_Read | EAH_CPU_Write);
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

		int32_t in_num_tiles_x = std::min((in_width + tile_size - 1) / tile_size, num_tiles);
		int32_t in_num_tiles_y = std::min((in_height + tile_size - 1) / tile_size, num_tiles);

		int32_t beg_tile_x = std::max(x - 1, 0) - x;
		int32_t end_tile_x = std::min(static_cast<int32_t>(x + in_num_tiles_x + 1), static_cast<int32_t>(num_tiles)) - x;
		int32_t beg_tile_y = std::max(y - 1, 0) - y;
		int32_t end_tile_y = std::min(static_cast<int32_t>(y + in_num_tiles_y + 1), static_cast<int32_t>(num_tiles)) - y;

		std::vector<std::vector<uint8_t> > tiles;
		std::vector<uint32_t> tile_ids;
		for (int32_t by = beg_tile_y; by < end_tile_y; ++ by)
		{
			tiles.clear();
			tile_ids.clear();
			for (int32_t bx = beg_tile_x; bx < end_tile_x; ++ bx)
			{
				uint32_t xindex = bx - beg_tile_x;

				tiles.push_back(std::vector<uint8_t>(tile_size * tile_size * pixel_size, 0));
				tile_ids.push_back(juda_tex->EncodeTileID(level, bx + x, by + y));
				for (size_t dy = 0; dy < tile_size; ++ dy)
				{
					int32_t tex_y = (*calc_v)(static_cast<int32_t>(by * tile_size + dy), in_height);
					if (tex_y >= 0)
					{
						for (size_t dx = 0; dx < tile_size; ++ dx)
						{
							int32_t tex_x = (*calc_u)(static_cast<int32_t>(bx * tile_size + dx), in_width);
							if (tex_x >= 0)
							{
								memcpy(&tiles[xindex][(dy * tile_size + dx) * pixel_size],
									&in_data_p[tex_y * mapper.RowPitch() + tex_x * pixel_size],
									pixel_size);
							}
							else
							{
								memcpy(&tiles[xindex][(dy * tile_size + dx) * pixel_size],
									&border_clr_u8,
									pixel_size);
							}
						}
					}
					else
					{
						for (size_t dx = 0; dx < tile_size; ++ dx)
						{
							memcpy(&tiles[xindex][(dy * tile_size + dx) * pixel_size],
								&border_clr_u8,
								pixel_size);
						}
					}
				}
			}

			juda_tex->CommitTiles(tiles, tile_ids);
		}

		cout << "Takes " << timer.elapsed() << "s" << endl;
	}

	cout << "Total tiles: " << ((1UL << (juda_tex->TreeLevels() * 2)) - 1) / (4 - 1) << endl;
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
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(context_cfg);
	
	EmptyApp app;
	app.Create();

	PackJTML(argv[1]);

	return 0;
}
