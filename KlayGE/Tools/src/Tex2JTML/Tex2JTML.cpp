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
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4251 4275 4273 4512 4701 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 6328)
#endif
#include <boost/tokenizer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

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

typedef std::map<KlayGE::TexturePtr, std::string> TextureNameTable;

static TextureNameTable& GetTextureNameTable()
{
	static TextureNameTable table;
	return table;
}

std::string DosWildcardToRegex(std::string const & wildcard)
{
	std::string ret;
	for (size_t i = 0; i < wildcard.size(); ++ i)
	{
		switch (wildcard[i])
		{
		case '*':
			ret.append(".*");
			break;

		case '?':
			ret.append(".");
			break;

		case '+':
		case '(':
		case ')':
		case '^':
		case '$':
		case '.':
		case '{':
		case '}':
		case '[':
		case ']':
		case '|':
		case '\\':
			ret.push_back('\\');
			ret.push_back(wildcard[i]);
			break;

		default:
			ret.push_back(wildcard[i]);
			break;
		}
	}

	return ret;
}

// From http://www.blackpawn.com/texts/lightmaps/default.html
class TexPackNode : public boost::enable_shared_from_this<TexPackNode>
{
public:
	TexPackNode()
		: rect_(0, 0, 0, 0)
	{
	}
	
	boost::shared_ptr<TexPackNode> Insert(KlayGE::TexturePtr const & tex)
	{
		if (!this->IsLeaf())
		{
			// try first child
			boost::shared_ptr<TexPackNode> new_node = child_[0]->Insert(tex);
			if (new_node)
			{
				return new_node;
			}
			// no room, then insert second child
			return child_[1]->Insert(tex);
		}
		else
		{
			// room don't fit or already have picture here
			if (!this->CanInsert(tex))
			{
				return boost::shared_ptr<TexPackNode>();
			}
			// result comes form perfecly fit
			if (this->CanPerfectlyInsert(tex))
			{
				return this->shared_from_this();
			}
			// split this node and create some kids
			child_[0] = MakeSharedPtr<TexPackNode>();
			child_[1] = MakeSharedPtr<TexPackNode>();
			// calc the rect for each child
			int dw = rect_.Width() - tex->Width(0);
			int dh = rect_.Height() - tex->Height(0);
			if (dw > dh)
			{
				child_[0]->rect_ = rect_;
				child_[0]->rect_.right() = rect_.left() + tex->Width(0);
				child_[1]->rect_ = rect_;
				child_[1]->rect_.left() = rect_.left() + tex->Width(0);
			}
			else
			{
				child_[0]->rect_ = rect_;
				child_[0]->rect_.bottom() = rect_.top() + tex->Height(0);
				child_[1]->rect_ = rect_;
				child_[1]->rect_.top() = rect_.top() + tex->Height(0);
			}
			// insert first child we create
			return child_[0]->Insert(tex);
		}
	}
	
	bool IsLeaf()
	{
		return !child_[0] && !child_[1];
	}
	
	bool IsFit(KlayGE::TexturePtr const & tex)
	{
		return (rect_.Height() >= tex->Height(0)) && (rect_.Width() >= tex->Width(0));
	}
	
	bool CanInsert(KlayGE::TexturePtr const & tex)
	{
		return this->IsFit(tex) && !this->Texture();
	}
	
	bool CanPerfectlyInsert(KlayGE::TexturePtr const & tex)
	{
		return (rect_.Width() == tex->Width(0)) && (rect_.Height() == tex->Height(0));
	}

	boost::shared_ptr<TexPackNode> Child(uint32_t index)
	{
		BOOST_ASSERT(index < 2);
		return child_[index];
	}

	void Rect(KlayGE::Rect_T<uint32_t> const & rc)
	{
		rect_ = rc;
	}
	KlayGE::Rect_T<uint32_t> const & Rect() const
	{
		return rect_;
	}

	void Texture(KlayGE::TexturePtr const & tex)
	{
		texture_ = tex;
	}
	KlayGE::TexturePtr const & Texture() const
	{
		return texture_;
	}

private:
	boost::shared_ptr<TexPackNode> child_[2];
	KlayGE::Rect_T<uint32_t> rect_;
	KlayGE::TexturePtr texture_;
};


// Fast sqrt for integer
unsigned int sqrt_16(unsigned long M) 
{ 
	unsigned int N, i; 
	unsigned long tmp, ttp;
	if (0 == M)
	{
		return 0;
	}
	N = 0;
	tmp = (M >> 30);
	M <<= 2; 
	if (tmp > 1)
	{ 
		++ N;
		tmp -= N; 
	}
	for (i = 15; i > 0; -- i)
	{ 
		N <<= 1;
		tmp <<= 2; 
		tmp += (M >> 30);
		ttp = N; 
		ttp = (ttp << 1) + 1;
		M <<= 2; 
		if (tmp >= ttp)
		{ 
			tmp -= ttp; 
			++ N; 
		}
	}
	return N; 
}

void CalcPackInfo(std::vector<KlayGE::TexturePtr>& ta, int num_tiles, int tile_size, boost::shared_ptr<TexPackNode>& root)
{
	root = MakeSharedPtr<TexPackNode>();
	int width = sqrt_16(num_tiles) * tile_size;
	root->Rect(KlayGE::Rect_T<uint32_t>(0, 0, width, width));
	// The insert function traverses the tree looking for a place to insert the lightmap.
	// It returns the pointer of the node the lightmap can go into or null to say it can't fit.
	for (size_t i = 0; i < ta.size(); ++ i)
	{
		boost::shared_ptr<TexPackNode> node = root->Insert(ta[i]);
		if (node)
		{
			node->Texture(ta[i]);
		}
	}
}

char const * GetTextureAddressTypeName(TexAddressingMode type)
{
	switch(type)
	{
	case TAM_Wrap:
		return "wrap";

	case TAM_Mirror:
		return "mirror";

	case TAM_Clamp:
		return "clamp";

	case TAM_Border:
	default:
		return "broder";
	}
}

struct JTMLImageRecord
{
	std::string name;
	uint32_t x, y, h, w;
	TexAddressingMode u, v;
};
typedef std::vector<JTMLImageRecord> JTMLImageRecordArray;

void ConvertTreeToJTML(boost::shared_ptr<TexPackNode> const & node, JTMLImageRecordArray& jirs, int tile_size)
{
	if (node->Texture())
	{
		JTMLImageRecord jir;
		jir.u = TAM_Wrap;
		jir.v = TAM_Wrap;
		jir.x = node->Rect().left() / tile_size;
		jir.y = node->Rect().top() / tile_size;
		jir.w = node->Rect().Width() / tile_size;
		jir.h = node->Rect().Height() / tile_size;
		TextureNameTable const & table = GetTextureNameTable();
		jir.name = table.find(node->Texture())->second;
		jirs.push_back(jir);
	}
	if (!node->IsLeaf())
	{
		ConvertTreeToJTML(node->Child(0), jirs, tile_size);
		ConvertTreeToJTML(node->Child(1), jirs, tile_size);
	}
}

void WriteJTML(boost::shared_ptr<TexPackNode> const & root, std::string const & jtml_name, int num_tiles, int tile_size, ElementFormat fmt)
{
	JTMLImageRecordArray jirs;
	ConvertTreeToJTML(root, jirs, tile_size);

	std::string fmt_str;
	switch (fmt)
	{
	case EF_ABGR8:
		fmt_str = "ABGR8";
		break;

	case EF_ARGB8:
		fmt_str = "ARGB8";
		break;

	default:
		BOOST_ASSERT(false);
		break;
	}

	std::ofstream os(jtml_name.c_str());
	os << "<?xml version='1.0'?>" << endl;
	os << endl;
	os << "<juda_tex num_tiles=\"" << num_tiles << "\" tile_size=\"" << tile_size << "\" format=\"" << fmt_str << "\">" << endl;
	for (size_t i = 0; i < jirs.size(); ++ i)
	{
		JTMLImageRecord const & jir = jirs[i];
		os << "\t<image name=\"" << jir.name
				<< "\" x=\"" << jir.x << "\" y=\"" << jir.y << "\" w=\"" << jir.w << "\" h=\"" << jir.h
				<<"\" address_u=\"" << GetTextureAddressTypeName(jir.u)
				<<"\" address_v=\"" << GetTextureAddressTypeName(jir.v) << "\"/>\n";
	}
	os << "</juda_tex>" << endl;
}

void Tex2JTML(std::vector<std::string>& tex_names, uint32_t num_tiles, uint32_t tile_size, std::string& jtml_name)
{
	std::vector<KlayGE::TexturePtr> textures;
	TextureNameTable& table = GetTextureNameTable();
	textures.resize(tex_names.size());
	for (size_t i = 0; i < tex_names.size(); ++ i)
	{
		cout << "Adding " << tex_names[i] << " (" << i + 1 << " / " << tex_names.size() << ") ...";
		textures[i] = SyncLoadTexture(tex_names[i], EAH_CPU_Read);
		table.insert(std::make_pair(textures[i], tex_names[i]));
		cout << " DONE" << endl;
	}

	boost::shared_ptr<TexPackNode> root;
	CalcPackInfo(textures, num_tiles, tile_size, root);

	WriteJTML(root, jtml_name, num_tiles, tile_size, EF_ABGR8);
}

int main(int argc, char* argv[])
{
	int num_tiles;
	int tile_size;
	std::vector<std::string> tex_names;
	std::string jtml_name;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-name,I", boost::program_options::value<std::string>(), "Input font name.")
		("output-name,O", boost::program_options::value<std::string>(), "Output font name.")
		("num-tiles,N", boost::program_options::value<int>(&num_tiles)->default_value(4096), "Number of tiles. Default is 4096.")
		("tile-size,T", boost::program_options::value<int>(&tile_size)->default_value(128), "Tile size. Default is 128.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if ((argc <= 1) || (vm.count("help") > 0))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Tex2JTML Generator, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-name") > 0)
	{
		std::string input_name_str = vm["input-name"].as<std::string>();

		boost::char_separator<char> sep("", ",;");
		boost::tokenizer<boost::char_separator<char> > tok(input_name_str, sep);
		for (BOOST_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
		{
			std::string arg = *beg;
			if ((std::string::npos == arg.find("*")) && (std::string::npos == arg.find("?")))
			{
				tex_names.push_back(arg);
			}
			else
			{
				boost::regex const filter(DosWildcardToRegex(arg));

				boost::filesystem::directory_iterator end_itr;
				for (boost::filesystem::directory_iterator i("."); i != end_itr; ++ i)
				{
					if (boost::filesystem::is_regular_file(i->status()))
					{
						boost::smatch what;
						std::string const name = i->path().filename().string();
						if (boost::regex_match(name, what, filter))
						{
							tex_names.push_back(name);
						}
					}
				}
			}
		}
	}
	else
	{
		cout << "Input texture name was not set." << endl;
		return 1;
	}
	if (vm.count("output-name") > 0)
	{
		jtml_name = vm["output-name"].as<std::string>();
	}
	else
	{
		cout << "Output jtml name was not set." << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(context_cfg);

	EmptyApp app;
	app.Create();

	Tex2JTML(tex_names, num_tiles, tile_size, jtml_name);

	return 0;
}
