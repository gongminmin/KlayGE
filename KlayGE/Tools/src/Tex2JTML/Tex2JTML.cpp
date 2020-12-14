/**
 * @file Tex2JTML.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Timer.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/Math.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>

#include <nonstd/scope.hpp>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

#include <KlayGE/JudaTexture.hpp>
#include <KlayGE/ToolCommon.hpp>

using namespace std;
using namespace KlayGE;

struct TextureDesc
{
	std::string name;
	uint32_t width;
	uint32_t height;
};
typedef std::shared_ptr<TextureDesc> TextureDescPtr;

// From http://www.blackpawn.com/texts/lightmaps/default.html
class TexPackNode : public std::enable_shared_from_this<TexPackNode>
{
public:
	TexPackNode()
		: rect_(0, 0, 0, 0)
	{
	}
	
	std::shared_ptr<TexPackNode> Insert(TextureDescPtr const & tex_desc)
	{
		if (this->IsLeaf())
		{
			// room don't fit or already have picture here
			if (!this->CanInsert(tex_desc))
			{
				return std::shared_ptr<TexPackNode>();
			}
			// result comes form perfecly fit
			if (this->CanPerfectlyInsert(tex_desc))
			{
				return this->shared_from_this();
			}
			// split this node and create some kids
			child_[0] = MakeSharedPtr<TexPackNode>();
			child_[1] = MakeSharedPtr<TexPackNode>();
			// calc the rect for each child
			int dw = rect_.Width() - tex_desc->width;
			int dh = rect_.Height() - tex_desc->height;
			if (dw > dh)
			{
				child_[0]->rect_ = rect_;
				child_[0]->rect_.right() = rect_.left() + tex_desc->width;
				child_[1]->rect_ = rect_;
				child_[1]->rect_.left() = rect_.left() + tex_desc->width;
			}
			else
			{
				child_[0]->rect_ = rect_;
				child_[0]->rect_.bottom() = rect_.top() + tex_desc->height;
				child_[1]->rect_ = rect_;
				child_[1]->rect_.top() = rect_.top() + tex_desc->height;
			}
			// insert first child we create
			return child_[0]->Insert(tex_desc);
		}
		else
		{
			// try first child
			std::shared_ptr<TexPackNode> new_node = child_[0]->Insert(tex_desc);
			if (new_node)
			{
				return new_node;
			}
			// no room, then insert second child
			return child_[1]->Insert(tex_desc);
		}
	}
	
	bool IsLeaf()
	{
		return !child_[0] && !child_[1];
	}
	
	bool IsFit(TextureDescPtr const & tex_desc)
	{
		return (rect_.Height() >= tex_desc->height) && (rect_.Width() >= tex_desc->width);
	}
	
	bool CanInsert(TextureDescPtr const & tex_desc)
	{
		return this->IsFit(tex_desc) && !this->TextureDesc();
	}
	
	bool CanPerfectlyInsert(TextureDescPtr const & tex_desc)
	{
		return (rect_.Width() == tex_desc->width) && (rect_.Height() == tex_desc->height);
	}

	std::shared_ptr<TexPackNode> Child(uint32_t index)
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

	void TextureDesc(TextureDescPtr const & tex_desc)
	{
		tex_desc_ = tex_desc;
	}
	TextureDescPtr const & TextureDesc() const
	{
		return tex_desc_;
	}

private:
	std::shared_ptr<TexPackNode> child_[2];
	KlayGE::Rect_T<uint32_t> rect_;
	TextureDescPtr tex_desc_;
};


void CalcPackInfo(std::vector<TextureDescPtr>& ta, int num_tiles, int tile_size, std::shared_ptr<TexPackNode>& root)
{
	root = MakeSharedPtr<TexPackNode>();
	int size = num_tiles * tile_size;
	root->Rect(KlayGE::Rect_T<uint32_t>(0, 0, size, size));
	// The insert function traverses the tree looking for a place to insert the lightmap.
	// It returns the pointer of the node the lightmap can go into or null to say it can't fit.
	for (size_t i = 0; i < ta.size(); ++ i)
	{
		std::shared_ptr<TexPackNode> node = root->Insert(ta[i]);
		if (node)
		{
			node->TextureDesc(ta[i]);
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

void ConvertTreeToJTML(std::shared_ptr<TexPackNode> const & node, std::vector<JTMLImageRecord>& jirs, int tile_size)
{
	if (node->TextureDesc())
	{
		JTMLImageRecord jir;
		jir.u = TAM_Wrap;
		jir.v = TAM_Wrap;
		jir.x = node->Rect().left() / tile_size;
		jir.y = node->Rect().top() / tile_size;
		jir.w = node->Rect().Width() / tile_size;
		jir.h = node->Rect().Height() / tile_size;
		jir.name = node->TextureDesc()->name;
		jirs.push_back(jir);
	}
	if (!node->IsLeaf())
	{
		ConvertTreeToJTML(node->Child(0), jirs, tile_size);
		ConvertTreeToJTML(node->Child(1), jirs, tile_size);
	}
}

void WriteJTML(std::shared_ptr<TexPackNode> const & root, std::string const & jtml_name, int num_tiles, int tile_size, ElementFormat fmt)
{
	std::vector<JTMLImageRecord> jirs;
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
		KFL_UNREACHABLE("Unsupported element format");
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
	std::vector<TextureDescPtr> tex_descs(tex_names.size());
	for (size_t i = 0; i < tex_names.size(); ++ i)
	{
		cout << "Adding " << tex_names[i] << " (" << i + 1 << " / " << tex_names.size() << ") ...";
		
		Texture::TextureType type;
		uint32_t width, height, depth;
		uint32_t num_mipmaps;
		uint32_t array_size;
		ElementFormat format;
		uint32_t row_pitch, slice_pitch;
		GetImageInfo(tex_names[i], type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
		
		tex_descs[i] = MakeSharedPtr<TextureDesc>();
		tex_descs[i]->name = tex_names[i];
		tex_descs[i]->width = width;
		tex_descs[i]->height = height;
		
		cout << " DONE" << endl;
	}

	std::shared_ptr<TexPackNode> root;
	CalcPackInfo(tex_descs, num_tiles, tile_size, root);

	WriteJTML(root, jtml_name, num_tiles, tile_size, EF_ABGR8);
}

int main(int argc, char* argv[])
{
	auto on_exit = nonstd::make_scope_exit([] { Context::Destroy(); });

	int num_tiles;
	int tile_size;
	std::vector<std::string> tex_names;
	std::string jtml_name;

	cxxopts::Options options("Tex2JTML", "KlayGE Tex2JTML Generator");
	// clang-format off
	options.add_options()
		("H,help", "Produce help message.")
		("I,input-name", "Input textures names.", cxxopts::value<std::string>())
		("O,output-name", "Output jtml name.", cxxopts::value<std::string>())
		("N,num-tiles", "Number of tiles.", cxxopts::value<int>(num_tiles)->default_value("2048"))
		("T,tile-size", "Tile size.", cxxopts::value<int>(tile_size)->default_value("128"))
		("v,version", "Version.");
	// clang-format on

	int const argc_backup = argc;
	auto vm = options.parse(argc, argv);

	if ((argc_backup <= 1) || (vm.count("help") > 0))
	{
		cout << options.help() << endl;
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

		std::vector<std::string_view> tokens = StringUtil::Split(input_name_str, StringUtil::IsAnyOf(",;"));
		for (auto& arg : tokens)
		{
			arg = StringUtil::Trim(arg);
			if ((std::string::npos == arg.find('*')) && (std::string::npos == arg.find('?')))
			{
				tex_names.push_back(std::string(arg));
			}
			else
			{
				FILESYSTEM_NS::path arg_path(arg.begin(), arg.end());
				auto const parent = arg_path.parent_path();
				auto const file_name = arg_path.filename();

				std::regex const filter(DosWildcardToRegex(file_name.string()));

				FILESYSTEM_NS::directory_iterator end_itr;
				for (FILESYSTEM_NS::directory_iterator i(parent); i != end_itr; ++i)
				{
					if (FILESYSTEM_NS::is_regular_file(i->status()))
					{
						std::smatch what;
						std::string const name = i->path().filename().string();
						if (std::regex_match(name, what, filter))
						{
							tex_names.push_back((parent / name).string());
						}
					}
				}
			}
		}
	}
	else
	{
		cout << "Need input textures names." << endl;
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("output-name") > 0)
	{
		jtml_name = vm["output-name"].as<std::string>();
	}
	else
	{
		cout << "Need output jtml name." << endl;
		return 1;
	}

	Tex2JTML(tex_names, num_tiles, tile_size, jtml_name);

	return 0;
}
