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
#include <KFL/StringUtil.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <iostream>

#include <KlayGE/ToolCommon.hpp>
#include <KlayGE/DevHelper/PlatformDefinition.hpp>

using namespace std;
using namespace KlayGE;

uint32_t const KFX_VERSION = 0x0150;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: FXMLJIT d3d_12_1|d3d_12_0|d3d_11_1|d3d_11_0|gl_4_6|gl_4_5|gl_4_4|gl_4_3|gl_4_2|gl_4_1|gles_3_2|gles_3_1|gles_3_0 xxx.fxml [target folder]" << endl;
		return 1;
	}

	std::string platform = argv[1];

	StringUtil::ToLower(platform);

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

	PlatformDefinition platform_def(platform + ".plat");

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	int major_version = platform_def.major_version;
	int minor_version = platform_def.minor_version;
	bool frag_depth_support = platform_def.frag_depth_support;
	re.SetCustomAttrib("PLATFORM", &platform_def.platform);
	re.SetCustomAttrib("MAJOR_VERSION", &major_version);
	re.SetCustomAttrib("MINOR_VERSION", &minor_version);
	re.SetCustomAttrib("NATIVE_SHADER_FOURCC", &platform_def.native_shader_fourcc);
	re.SetCustomAttrib("NATIVE_SHADER_VERSION", &platform_def.native_shader_version);
	re.SetCustomAttrib("REQUIRES_FLIPPING", &platform_def.requires_flipping);
	re.SetCustomAttrib("DEVICE_CAPS", &platform_def.device_caps);
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
			std::vector<std::string_view> frags = StringUtil::Split(base_name, StringUtil::EqualTo('+'));
			for (auto const & frag : frags)
			{
				fxml_names.push_back(ResLoader::Instance().Locate(std::string(frag) + ".fxml"));
			}
		}
		else
		{
			fxml_names.push_back(fxml_name);
		}

		RenderEffect effect;
		effect.Load(fxml_names);
		effect.CompileShaders();
	}
	if (!target_folder.empty())
	{
		filesystem::copy_file(kfx_path, target_folder / kfx_name, filesystem::copy_options::overwrite_existing);
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
