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
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <iostream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/algorithm/string/case_conv.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <boost/algorithm/string/classification.hpp>

using namespace std;
using namespace KlayGE;

uint32_t const KFX_VERSION = 0x0140;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
struct PlatformDefinition
{
	std::string platform;
	uint8_t major_version;
	uint8_t minor_version;

	bool requires_flipping;
	uint32_t native_shader_fourcc;
	uint32_t native_shader_version;

	ShaderModel max_shader_model;

	uint32_t max_texture_depth;
	uint32_t max_texture_array_length;
	uint8_t max_pixel_texture_units;
	uint8_t max_simultaneous_rts;

	bool fp_color_support : 1;
	bool pack_to_rgba_required : 1;
	bool render_to_texture_array_support : 1;

	bool gs_support : 1;
	bool cs_support : 1;
	bool hs_support : 1;
	bool ds_support : 1;

	bool bc4_support : 1;
	bool bc5_support : 1;
	bool frag_depth_support : 1;
};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

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
		return std::string(attr->ValueString());
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

PlatformDefinition LoadPlatformConfig(std::string const & platform)
{
	ResIdentifierPtr plat = ResLoader::Instance().Open("PlatConf/" + platform + ".plat");

	KlayGE::XMLDocument doc;
	XMLNodePtr root = doc.Parse(plat);

	PlatformDefinition ret;

	ret.platform = RetrieveAttrValue(root, "name", "");
	ret.major_version = static_cast<uint8_t>(RetrieveAttrValue(root, "major_version", 0));
	ret.minor_version = static_cast<uint8_t>(RetrieveAttrValue(root, "minor_version", 0));

	ret.requires_flipping = RetrieveNodeValue(root, "requires_flipping", 0) ? true : false;
	std::string const fourcc_str = RetrieveNodeValue(root, "native_shader_fourcc", "");
	ret.native_shader_fourcc = (fourcc_str[0] << 0) + (fourcc_str[1] << 8) + (fourcc_str[2] << 16) + (fourcc_str[3] << 24);
	ret.native_shader_version = RetrieveNodeValue(root, "native_shader_version", 0);

	XMLNodePtr max_shader_model_node = root->FirstNode("max_shader_model");
	ret.max_shader_model = ShaderModel(static_cast<uint8_t>(RetrieveAttrValue(max_shader_model_node, "major", 0)),
		static_cast<uint8_t>(RetrieveAttrValue(max_shader_model_node, "minor", 0)));

	ret.max_texture_depth = RetrieveNodeValue(root, "max_texture_depth", 0);
	ret.max_texture_array_length = RetrieveNodeValue(root, "max_texture_array_length", 0);
	ret.max_pixel_texture_units = static_cast<uint8_t>(RetrieveNodeValue(root, "max_pixel_texture_units", 0));
	ret.max_simultaneous_rts = static_cast<uint8_t>(RetrieveNodeValue(root, "max_simultaneous_rts", 0));

	ret.fp_color_support = RetrieveNodeValue(root, "fp_color_support", 0) ? true : false;
	ret.pack_to_rgba_required = RetrieveNodeValue(root, "pack_to_rgba_required", 0) ? true : false;
	ret.render_to_texture_array_support = RetrieveNodeValue(root, "render_to_texture_array_support", 0) ? true : false;

	ret.gs_support = RetrieveNodeValue(root, "gs_support", 0) ? true : false;
	ret.cs_support = RetrieveNodeValue(root, "cs_support", 0) ? true : false;
	ret.hs_support = RetrieveNodeValue(root, "hs_support", 0) ? true : false;
	ret.ds_support = RetrieveNodeValue(root, "ds_support", 0) ? true : false;

	ret.bc4_support = RetrieveNodeValue(root, "bc4_support", 0) ? true : false;
	ret.bc5_support = RetrieveNodeValue(root, "bc5_support", 0) ? true : false;
	ret.frag_depth_support = RetrieveNodeValue(root, "frag_depth_support", 0) ? true : false;

	return ret;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: FXMLJIT d3d_12_1|d3d_12_0|d3d_11_1|d3d_11_0|gl_4_6|gl_4_5|gl_4_4|gl_4_3|gl_4_2|gl_4_1|gles_3_2|gles_3_1|gles_3_0 xxx.fxml [target folder]" << endl;
		return 1;
	}

	ResLoader::Instance().AddPath("../../Tools/media/PlatformDeployer");

	std::string platform = argv[1];

	boost::algorithm::to_lower(platform);

	filesystem::path target_folder;
	if (argc >= 4)
	{
		target_folder = argv[3];
	}

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.render_factory_name = "NullRender";
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	context_cfg.graphics_cfg.gamma = false;
	context_cfg.graphics_cfg.color_grading = false;
	Context::Instance().Config(context_cfg);

	PlatformDefinition plat = LoadPlatformConfig(platform);

	// We only care about some fields
	RenderDeviceCaps device_caps{};

	device_caps.max_shader_model = plat.max_shader_model;

	device_caps.max_texture_depth = plat.max_texture_depth;
	device_caps.max_texture_array_length = plat.max_texture_array_length;
	device_caps.max_pixel_texture_units = plat.max_pixel_texture_units;
	device_caps.max_simultaneous_rts = plat.max_simultaneous_rts;

	device_caps.fp_color_support = plat.fp_color_support;
	device_caps.pack_to_rgba_required = plat.pack_to_rgba_required;
	device_caps.render_to_texture_array_support = plat.render_to_texture_array_support;

	device_caps.gs_support = plat.gs_support;
	device_caps.cs_support = plat.cs_support;
	device_caps.hs_support = plat.hs_support;
	device_caps.ds_support = plat.ds_support;

	std::vector<ElementFormat> texture_format;
	texture_format.push_back(EF_R8);
	texture_format.push_back(EF_ABGR8);
	texture_format.push_back(EF_ARGB8);
	texture_format.push_back(EF_BC1);
	texture_format.push_back(EF_BC1_SRGB);
	texture_format.push_back(EF_BC2);
	texture_format.push_back(EF_BC2_SRGB);
	texture_format.push_back(EF_BC3);
	texture_format.push_back(EF_BC3_SRGB);
	if (plat.bc4_support)
	{
		texture_format.push_back(EF_BC4);
		texture_format.push_back(EF_BC4_SRGB);
	}
	if (plat.bc5_support)
	{
		texture_format.push_back(EF_BC5);
		texture_format.push_back(EF_BC5_SRGB);
	}

	std::vector<ElementFormat> uav_format;
	if (device_caps.max_shader_model >= ShaderModel(5, 1))
	{
		uav_format.push_back(EF_ABGR16F);
		uav_format.push_back(EF_B10G11R11F);
		uav_format.push_back(EF_ABGR8);
		uav_format.push_back(EF_R16UI);
		uav_format.push_back(EF_R32UI);
		uav_format.push_back(EF_R32F);
	}

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	int major_version = plat.major_version;
	int minor_version = plat.minor_version;
	bool frag_depth_support = plat.frag_depth_support;
	re.SetCustomAttrib("PLATFORM", &plat.platform);
	re.SetCustomAttrib("MAJOR_VERSION", &major_version);
	re.SetCustomAttrib("MINOR_VERSION", &minor_version);
	re.SetCustomAttrib("NATIVE_SHADER_FOURCC", &plat.native_shader_fourcc);
	re.SetCustomAttrib("NATIVE_SHADER_VERSION", &plat.native_shader_version);
	re.SetCustomAttrib("REQUIRES_FLIPPING", &plat.requires_flipping);
	re.SetCustomAttrib("DEVICE_CAPS", &device_caps);
	re.SetCustomAttrib("TEXTURE_FORMAT", &texture_format);
	re.SetCustomAttrib("UAV_FORMAT", &uav_format);
	re.SetCustomAttrib("FRAG_DEPTH_SUPPORT", &frag_depth_support);

	std::string fxml_name(argv[2]);
	filesystem::path fxml_path(fxml_name);
	std::string const base_name = fxml_path.stem().string();
	filesystem::path fxml_directory = fxml_path.parent_path();
	ResLoader::Instance().AddPath(fxml_directory.string());

	filesystem::path kfx_name(base_name + ".kfx");
	filesystem::path kfx_path = fxml_directory / kfx_name;
	bool skip_jit = false;
	if (filesystem::exists(fxml_path) && filesystem::exists(kfx_path))
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

			uint8_t shader_platform_name_len;
			kfx_source->read(&shader_platform_name_len, sizeof(shader_platform_name_len));
			std::string shader_platform_name(shader_platform_name_len, 0);
			kfx_source->read(&shader_platform_name[0], shader_platform_name_len);

			if ((re.NativeShaderFourCC() == shader_fourcc) && (re.NativeShaderVersion() == shader_ver)
				&& (re.NativeShaderPlatformName() == shader_platform_name))
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
		std::vector<string> fxml_names;
		if (ResLoader::Instance().Locate(fxml_name).empty())
		{
			std::vector<std::string> frags;
			boost::algorithm::split(frags, base_name, boost::is_any_of("+"));
			for (auto const & frag : frags)
			{
				fxml_names.push_back(frag + ".fxml");
			}

			fxml_names.back() = (fxml_directory / fxml_names.back()).string();
		}
		else
		{
			fxml_names.push_back(fxml_name);
		}

		RenderEffect effect;
		effect.Load(fxml_names);
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
