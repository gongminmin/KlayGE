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
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
#include <filesystem>
namespace KlayGE
{
	namespace filesystem = std::tr2::sys;
}
#else
#include <boost/filesystem.hpp>
namespace KlayGE
{
	namespace filesystem = boost::filesystem;
}
#endif

using namespace std;
using namespace KlayGE;

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) KlayGE::uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

class FXMLJITApp : public KlayGE::App3DFramework
{
public:
	FXMLJITApp()
		: App3DFramework("FXMLJIT")
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

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: FXMLJIT pc_dx11|pc_dx10|pc_dx9|pc_gl4|pc_gl3|pc_gl2|android_tegra3 xxx.fxml [target folder]" << endl;
		return 1;
	}

	std::string platform = argv[1];

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	if (("pc_dx11" == platform) || ("pc_dx10" == platform) || ("pc_dx9" == platform))
	{
		context_cfg.render_factory_name = "D3D11";
		if ("pc_dx11" == platform)
		{
			context_cfg.graphics_cfg.options = "level:11_0";
		}
		else if ("pc_dx10" == platform)
		{
			context_cfg.graphics_cfg.options = "level:10_0";
		}
		else if ("pc_dx9" == platform)
		{
			context_cfg.graphics_cfg.options = "level:9_3";
		}
	}
	else if (("pc_gl4" == platform) || ("pc_gl3" == platform) || ("pc_gl2" == platform))
	{
		context_cfg.render_factory_name = "OpenGL";
	}
	else if ("android_tegra3" == platform)
	{
		context_cfg.render_factory_name = "OpenGLES";
	}
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	context_cfg.graphics_cfg.gamma = false;
	context_cfg.graphics_cfg.color_grading = false;
	Context::Instance().Config(context_cfg);

	FXMLJITApp app;
	app.Create();

	filesystem::path target_folder;
	if (argc >= 4)
	{
		target_folder = argv[3];
	}

	std::string file_name(argv[2]);
	filesystem::path file_path(file_name);
	std::string const base_name = filesystem::basename(file_path);
	ResLoader::Instance().AddPath(file_path.parent_path().string());

	filesystem::path kfx_path(base_name + ".kfx");
	filesystem::remove(kfx_path);
	SyncLoadRenderEffect(file_name);
	if (!target_folder.empty())
	{
		filesystem::copy_file(kfx_path, target_folder / kfx_path,
			filesystem::copy_option::overwrite_if_exists);
	}

	return 0;
}
