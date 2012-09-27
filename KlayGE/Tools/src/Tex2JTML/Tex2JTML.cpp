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
#include <boost/enable_shared_from_this.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/math/common_factor_rt.hpp>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <KlayGE/JudaTexture.hpp>

#include "JTMLWriter.hpp"
#include <math.h>

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

typedef std::vector<std::string> StringArray;
typedef std::vector<KlayGE::TexturePtr> TextureArray;

typedef std::map<KlayGE::TexturePtr, std::string> TextureNameTable;

inline static TextureNameTable& GetTextureNameTable()
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

void GetCommandLineInfo(int argc, char* argv[], StringArray& tex_names, std::string& jtml)
{
	tex_names.clear();
	for (int i = 1; i < argc - 1; ++ i)
	{
		std::string arg = argv[i];
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
	jtml = argv[argc - 1];
}

void CalcTileNumAndSize(TextureArray& ta, int& tile_num, int& tile_size)
{
	tile_num = 4096;
	tile_size = 256;
	std::vector<int> num_array;
	for (size_t i = 0; i < ta.size(); ++ i)
	{
		num_array.push_back(ta[i]->Width(0));
		num_array.push_back(ta[i]->Height(0));
	}
	for (size_t i = 0; i < num_array.size(); ++ i)
	{
		for (size_t j = i; j < num_array.size(); ++ j)
		{
			int g = boost::math::gcd(num_array[i], num_array[j]);
			if (g < tile_size)
			{
				tile_size = g;
			}
		}
	}
}

// From http://www.blackpawn.com/texts/lightmaps/default.html
class TexPackNode : public boost::enable_shared_from_this<TexPackNode>
{
public:
	TexPackNode()
		: rect_(0, 0, 0, 0)
	{
	}
	
	boost::shared_ptr<TexPackNode> Insert(KlayGE::TexturePtr& tex);
	
	bool IsLeaf()
	{
		return !child_[0] && !child_[1];
	}
	
	bool IsFit(KlayGE::TexturePtr& tex)
	{
		return (rect_.Height() >= tex->Height(0)) && (rect_.Width() >= tex->Width(0));
	}
	
	bool CanInsert(KlayGE::TexturePtr& tex)
	{
		return this->IsFit(tex) && !this->Texture();
	}
	
	bool CanPerfectlyInsert(KlayGE::TexturePtr& tex)
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

boost::shared_ptr<TexPackNode> TexPackNode::Insert(KlayGE::TexturePtr& tex)
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

void CalcPackInfo(TextureArray& ta, int tile_num, int tile_size, boost::shared_ptr<TexPackNode>& root)
{
	root = MakeSharedPtr<TexPackNode>();
	int width = sqrt_16(tile_num) * tile_size;
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

void ConvertTreeToJTML(boost::shared_ptr<TexPackNode> const & node, JTMLImageRecordArray& jirs, int tile_size)
{
	if (node->Texture())
	{
		JTMLImageRecord jir;
		jir.u_ = TAM_Wrap;
		jir.v_ = TAM_Wrap;
		jir.x_ = node->Rect().left() / tile_size;
		jir.y_ = node->Rect().top() / tile_size;
		jir.w_ = node->Rect().Width() / tile_size;
		jir.h_ = node->Rect().Height() / tile_size;
		TextureNameTable& table = GetTextureNameTable();
		jir.name_ = table[node->Texture()];
		jirs.push_back(jir);
	}
	if (!node->IsLeaf())
	{
		ConvertTreeToJTML(node->Child(0), jirs, tile_size);
		ConvertTreeToJTML(node->Child(1), jirs, tile_size);
	}
}

void WriteJTML(boost::shared_ptr<TexPackNode> const & root, std::string const & jtml_name, int tile_size, int tile_num, ElementFormat fmt)
{
	JTMLWriter& writer = JTMLWriter::Instance();
	writer.SetElementFormat(fmt);
	writer.SetTileNum(tile_num);
	writer.SetTileSize(tile_size);
	ConvertTreeToJTML(root, writer.GetImageRecordArray(), tile_size);
	writer.WriteToJTML(jtml_name);
}

void Tex2JTML(StringArray& tex_names, std::string& jtml_name)
{
	TextureArray textures;
	TextureNameTable& table = GetTextureNameTable();
	textures.resize(tex_names.size());
	for (size_t i = 0; i < tex_names.size(); ++ i)
	{
		textures[i] = SyncLoadTexture(tex_names[i], EAH_CPU_Read);
		table.insert(std::make_pair(textures[i], tex_names[i]));
	}

	int tile_size, tile_num;
	CalcTileNumAndSize(textures, tile_num, tile_size);

	boost::shared_ptr<TexPackNode> root;
	CalcPackInfo(textures, tile_num, tile_size, root);

	WriteJTML(root, jtml_name, tile_size, tile_num, EF_ABGR8);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: Tex2JTML [tex] xxx.jtml" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../../bin");

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(context_cfg);

	EmptyApp app;
	app.Create();

	StringArray tex_names;
	std::string jtml_name;

	GetCommandLineInfo(argc, argv, tex_names, jtml_name);

	Tex2JTML(tex_names, jtml_name);

	return 0;
}
