/**
 * @file FXMLJIT.cpp
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
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>

#include <boost/algorithm/string/case_conv.hpp>

#include "OfflineRenderEffect.hpp"

using namespace std;
using namespace KlayGE;

uint32_t const KFX_VERSION = 0x0110;

int RetrieveAttrValue(XMLNodePtr node, std::string const & attr_name, int default_value)
{
	XMLAttributePtr attr = node->Attrib(attr_name);
	if (attr)
	{
		return attr->ValueInt();
	}

	return default_value;
}

std::string RetrieveAttrValue(XMLNodePtr node, std::string const & attr_name, std::string const & default_value)
{
	XMLAttributePtr attr = node->Attrib(attr_name);
	if (attr)
	{
		return attr->ValueString();
	}

	return default_value;
}

int RetrieveNodeValue(XMLNodePtr root, std::string const & node_name, int default_value)
{
	XMLNodePtr node = root->FirstNode(node_name);
	if (node)
	{
		return RetrieveAttrValue(node, "value", default_value);
	}

	return default_value;
}

std::string RetrieveNodeValue(XMLNodePtr root, std::string const & node_name, std::string const & default_value)
{
	XMLNodePtr node = root->FirstNode(node_name);
	if (node)
	{
		return RetrieveAttrValue(node, "value", default_value);
	}

	return default_value;
}

Offline::OfflineRenderDeviceCaps LoadPlatformConfig(std::string const & platform)
{
	ResIdentifierPtr plat = ResLoader::Instance().Open("PlatConf/" + platform + ".plat");

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(plat);

	Offline::OfflineRenderDeviceCaps caps;

	caps.platform = RetrieveAttrValue(root, "name", "");
	caps.major_version = static_cast<uint8_t>(RetrieveAttrValue(root, "major_version", 0));
	caps.minor_version = static_cast<uint8_t>(RetrieveAttrValue(root, "minor_version", 0));

	caps.requires_flipping = RetrieveNodeValue(root, "requires_flipping", 0) ? true : false;
	std::string const fourcc_str = RetrieveNodeValue(root, "native_shader_fourcc", "");
	caps.native_shader_fourcc = (fourcc_str[0] << 0) + (fourcc_str[1] << 8) + (fourcc_str[2] << 16) + (fourcc_str[3] << 24);
	caps.native_shader_version = RetrieveNodeValue(root, "native_shader_version", 0);

	XMLNodePtr max_shader_model_node = root->FirstNode("max_shader_model");
	caps.max_shader_model = ShaderModel(static_cast<uint8_t>(RetrieveAttrValue(max_shader_model_node, "major", 0)),
		static_cast<uint8_t>(RetrieveAttrValue(max_shader_model_node, "minor", 0)));

	caps.max_texture_depth = RetrieveNodeValue(root, "max_texture_depth", 0);
	caps.max_texture_array_length = RetrieveNodeValue(root, "max_texture_array_length", 0);
	caps.max_pixel_texture_units = static_cast<uint8_t>(RetrieveNodeValue(root, "max_pixel_texture_units", 0));
	caps.max_simultaneous_rts = static_cast<uint8_t>(RetrieveNodeValue(root, "max_simultaneous_rts", 0));

	caps.standard_derivatives_support = RetrieveNodeValue(root, "standard_derivatives_support", 0) ? true : false;
	caps.shader_texture_lod_support = RetrieveNodeValue(root, "shader_texture_lod_support", 0) ? true : false;
	caps.fp_color_support = RetrieveNodeValue(root, "fp_color_support", 0) ? true : false;
	caps.pack_to_rgba_required = RetrieveNodeValue(root, "pack_to_rgba_required", 0) ? true : false;
	caps.render_to_texture_array_support = RetrieveNodeValue(root, "render_to_texture_array_support", 0) ? true : false;

	caps.gs_support = RetrieveNodeValue(root, "gs_support", 0) ? true : false;
	caps.cs_support = RetrieveNodeValue(root, "cs_support", 0) ? true : false;
	caps.hs_support = RetrieveNodeValue(root, "hs_support", 0) ? true : false;
	caps.ds_support = RetrieveNodeValue(root, "ds_support", 0) ? true : false;

	caps.bc4_support = RetrieveNodeValue(root, "bc4_support", 0) ? true : false;
	caps.bc5_support = RetrieveNodeValue(root, "bc5_support", 0) ? true : false;
	caps.frag_depth_support = RetrieveNodeValue(root, "frag_depth_support", 0) ? true : false;
	caps.ubo_support = RetrieveNodeValue(root, "ubo_support", 0) ? true : false;

	return caps;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: FXMLJIT pc_dx11|pc_dx10|pc_dx9|win_tegra3|pc_gl4|pc_gl3|pc_gl2|android_tegra3|ios xxx.fxml [target folder]" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../Tools/media/PlatformDeployer");

	std::string platform = argv[1];

	if (("pc_dx11" == platform) || ("pc_dx10" == platform) || ("pc_dx9" == platform) || ("win_tegra3" == platform)
		|| ("pc_gl4" == platform) || ("pc_gl3" == platform) || ("pc_gl2" == platform)
		|| ("android_tegra3" == platform) || ("ios" == platform))
	{
		if ("pc_dx11" == platform)
		{
			platform = "d3d_11_0";
		}
		else if ("pc_dx10" == platform)
		{
			platform = "d3d_10_0";
		}
		else if ("pc_dx9" == platform)
		{
			platform = "d3d_9_3";
		}
		else if ("win_tegra3" == platform)
		{
			platform = "d3d_9_1";
		}
		else if ("pc_gl4" == platform)
		{
			platform = "gl_4_0";
		}
		else if ("pc_gl3" == platform)
		{
			platform = "gl_3_0";
		}
		else if ("pc_gl2" == platform)
		{
			platform = "gl_2_0";
		}
		else if ("android_tegra3" == platform)
		{
			platform = "gles_2_0";
		}
		else if ("ios" == platform)
		{
			platform = "gles_2_0";
		}
	}

	boost::algorithm::to_lower(platform);

	filesystem::path target_folder;
	if (argc >= 4)
	{
		target_folder = argv[3];
	}

	Offline::OfflineRenderDeviceCaps caps = LoadPlatformConfig(platform);

	std::string fxml_name(argv[2]);
	filesystem::path fxml_path(fxml_name);
	std::string const base_name = fxml_path.stem().string();
	filesystem::path fxml_directory = fxml_path.parent_path();
	ResLoader::Instance().AddPath(fxml_directory.string());

	filesystem::path kfx_name(base_name + ".kfx");
	filesystem::path kfx_path = fxml_directory / kfx_name;
	bool skip_jit = false;
	if (filesystem::exists(kfx_path))
	{
		ResIdentifierPtr source = ResLoader::Instance().Open(fxml_name);
		ResIdentifierPtr kfx_source = ResLoader::Instance().Open(kfx_path.string());

		uint64_t src_timestamp = source->Timestamp();

		uint32_t fourcc;
		kfx_source->read(&fourcc, sizeof(fourcc));
		fourcc = LE2Native(fourcc);

		uint32_t ver;
		kfx_source->read(&ver, sizeof(ver));
		ver = LE2Native(ver);

		if ((MakeFourCC<'K', 'F', 'X', ' '>::value == fourcc) && (KFX_VERSION == ver))
		{
			uint32_t shader_fourcc;
			kfx_source->read(&shader_fourcc, sizeof(shader_fourcc));
			shader_fourcc = LE2Native(shader_fourcc);

			uint32_t shader_ver;
			kfx_source->read(&shader_ver, sizeof(shader_ver));
			shader_ver = LE2Native(shader_ver);

			if ((caps.native_shader_fourcc == shader_fourcc) && (caps.native_shader_version == shader_ver))
			{
				uint64_t timestamp;
				kfx_source->read(&timestamp, sizeof(timestamp));
				timestamp = LE2Native(timestamp);
				if (src_timestamp <= timestamp)
				{
					skip_jit = true;
				}
			}
		}
	}

	if (!skip_jit)
	{
		Offline::RenderEffect effect(caps);
		effect.Load(fxml_name);
	}
	if (!target_folder.empty())
	{
		filesystem::copy_file(kfx_path, target_folder / kfx_name,
#if defined(KLAYGE_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(KLAYGE_TS_LIBRARY_FILESYSTEM_SUPPORT)
			filesystem::copy_options::overwrite_existing);
#else
			filesystem::copy_option::overwrite_if_exists);
#endif
		kfx_path = target_folder / kfx_name;
	}

	if (filesystem::exists(kfx_path))
	{
		cout << "Compiled kfx has been saved to " << kfx_path << "." << endl;
	}
	else
	{
		cout << "Couldn't find " << fxml_name << "." << endl;
	}

	Context::Destroy();

	return 0;
}
