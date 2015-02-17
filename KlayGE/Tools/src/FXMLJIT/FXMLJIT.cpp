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

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#if defined(KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT) || defined(KLAYGE_TR2_LIBRARY_FILESYSTEM_V3_SUPPORT)
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

#include "OfflineRenderEffect.hpp"

using namespace std;
using namespace KlayGE;

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) KlayGE::uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

uint32_t const KFX_VERSION = 0x0106;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: FXMLJIT pc_dx11|pc_dx10|pc_dx9|win_tegra3|pc_gl4|pc_gl3|pc_gl2|android_tegra3|ios xxx.fxml [target folder]" << endl;
		return 1;
	}

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

	filesystem::path target_folder;
	if (argc >= 4)
	{
		target_folder = argv[3];
	}

	Offline::OfflineRenderDeviceCaps caps;

	caps.platform = platform;

	if (0 == platform.find("d3d_"))
	{
		caps.requires_flipping = true;
		caps.native_shader_fourcc = MakeFourCC<'D', 'X', 'B', 'C'>::value;
		caps.native_shader_version = 5;

		caps.frag_depth_support = true;
		caps.ubo_support = false;

		if ("d3d_11_1" == platform)
		{
			caps.major_version = 11;
			caps.minor_version = 1;

			caps.max_shader_model = 5;

			caps.max_texture_depth = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			caps.max_pixel_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps.max_simultaneous_rts = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = true;
			caps.cs_support = true;
			caps.hs_support = true;
			caps.ds_support = true;

			caps.bc4_support = true;
			caps.bc5_support = true;
		}
		else if ("d3d_11_0" == platform)
		{
			caps.major_version = 11;
			caps.minor_version = 0;

			caps.max_shader_model = 5;

			caps.max_texture_depth = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			caps.max_pixel_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps.max_simultaneous_rts = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = true;
			caps.cs_support = true;
			caps.hs_support = true;
			caps.ds_support = true;

			caps.bc4_support = true;
			caps.bc5_support = true;
		}
		else if ("d3d_10_1" == platform)
		{
			caps.major_version = 10;
			caps.minor_version = 1;

			caps.max_shader_model = 4;

			caps.max_texture_depth = D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			caps.max_pixel_texture_units = D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps.max_simultaneous_rts = D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = true;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = true;
			caps.bc5_support = true;
		}
		else if ("d3d_10_0" == platform)
		{
			caps.major_version = 10;
			caps.minor_version = 0;

			caps.max_shader_model = 4;

			caps.max_texture_depth = D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			caps.max_pixel_texture_units = D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps.max_simultaneous_rts = D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = true;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = true;
			caps.bc5_support = true;
		}
		else if ("d3d_9_3" == platform)
		{
			caps.major_version = 9;
			caps.minor_version = 3;

			caps.max_shader_model = 2;

			caps.max_texture_depth = D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = 1;
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = false;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = true;

			caps.gs_support = false;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = false;
			caps.bc5_support = false;
		}
		else if ("d3d_9_2" == platform)
		{
			caps.major_version = 9;
			caps.minor_version = 2;

			caps.max_shader_model = 2;

			caps.max_texture_depth = D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = 1;
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = false;
			caps.shader_texture_lod_support = false;
			caps.fp_color_support = false;
			caps.pack_to_rgba_required = true;

			caps.gs_support = false;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = false;
			caps.bc5_support = false;
		}
		else if ("d3d_9_1" == platform)
		{
			caps.major_version = 9;
			caps.minor_version = 1;

			caps.max_shader_model = 2;

			caps.max_texture_depth = D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps.max_texture_array_length = 1;
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT;

			caps.standard_derivatives_support = false;
			caps.shader_texture_lod_support = false;
			caps.fp_color_support = false;
			caps.pack_to_rgba_required = true;

			caps.gs_support = false;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = false;
			caps.bc5_support = false;
		}
	}
	else if (0 == platform.find("gl_"))
	{
		caps.requires_flipping = false;
		caps.native_shader_fourcc = MakeFourCC<'G', 'L', 'S', 'L'>::value;
		caps.native_shader_version = 3;

		caps.frag_depth_support = true;
		caps.ubo_support = true;

		if (0 == platform.find("gl_4_"))
		{
			caps.major_version = 4;
			if ("gl_4_5" == platform)
			{
				caps.minor_version = 5;
			}
			else if ("gl_4_4" == platform)
			{
				caps.minor_version = 4;
			}
			else if ("gl_4_3" == platform)
			{
				caps.minor_version = 3;
			}
			else if ("gl_4_2" == platform)
			{
				caps.minor_version = 2;
			}
			else if ("gl_4_1" == platform)
			{
				caps.minor_version = 1;
			}
			else if ("gl_4_0" == platform)
			{
				caps.minor_version = 0;
			}

			caps.max_shader_model = 4; // TODO

			caps.max_texture_depth = 2048;
			caps.max_texture_array_length = 1; // TODO
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = 8;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = true;
			caps.cs_support = false; // TODO
			caps.hs_support = false; // TODO
			caps.ds_support = false; // TODO

			caps.bc4_support = true;
			caps.bc5_support = true;
		}
		else if (0 == platform.find("gl_3_"))
		{
			caps.major_version = 3;
			if ("gl_3_3" == platform)
			{
				caps.minor_version = 3;
			}
			else if ("gl_3_2" == platform)
			{
				caps.minor_version = 2;
			}
			else if ("gl_3_1" == platform)
			{
				caps.minor_version = 1;
			}
			else if ("gl_3_0" == platform)
			{
				caps.minor_version = 0;
			}

			caps.max_shader_model = 4;

			caps.max_texture_depth = 2048;
			caps.max_texture_array_length = 1; // TODO
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = 8;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = true;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = true;
			caps.bc5_support = true;
		}
		else if (0 == platform.find("gl_2_"))
		{
			caps.major_version = 2;
			if ("gl_2_1" == platform)
			{
				caps.minor_version = 1;
			}
			else if ("gl_2_0" == platform)
			{
				caps.minor_version = 0;
			}

			caps.max_shader_model = 2;

			caps.max_texture_depth = 256;
			caps.max_texture_array_length = 1;
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = 4;

			caps.standard_derivatives_support = false;
			caps.shader_texture_lod_support = false;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = true;

			caps.gs_support = false;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = false;
			caps.bc5_support = false;
		}
	}
	else if (0 == platform.find("gles_"))
	{
		caps.requires_flipping = false;
		caps.native_shader_fourcc = MakeFourCC<'E', 'S', 'S', 'L'>::value;
		caps.native_shader_version = 3;

		caps.frag_depth_support = false;

		if (0 == platform.find("gles_3_"))
		{
			caps.major_version = 3;
			if ("gles_3_1" == platform)
			{
				caps.minor_version = 1;
			}
			else if ("gles_3_0" == platform)
			{
				caps.minor_version = 0;
			}

			caps.max_shader_model = 4;

			caps.max_texture_depth = 2048;
			caps.max_texture_array_length = 1; // TODO
			caps.max_pixel_texture_units = 16;
			caps.max_simultaneous_rts = 8;

			caps.standard_derivatives_support = true;
			caps.shader_texture_lod_support = true;
			caps.fp_color_support = true;
			caps.pack_to_rgba_required = false;

			caps.gs_support = false; // TODO
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = true;
			caps.bc5_support = true;

			caps.ubo_support = true;
		}
		else if (0 == platform.find("gles_2_"))
		{
			caps.major_version = 2;
			caps.minor_version = 0;

			caps.max_shader_model = 2;

			caps.max_texture_depth = 1;
			caps.max_texture_array_length = 1;
			caps.max_pixel_texture_units = 8;
			caps.max_simultaneous_rts = 1;

			caps.standard_derivatives_support = false;
			caps.shader_texture_lod_support = false;
			caps.fp_color_support = false;
			caps.pack_to_rgba_required = true;

			caps.gs_support = false;
			caps.cs_support = false;
			caps.hs_support = false;
			caps.ds_support = false;

			caps.bc4_support = false;
			caps.bc5_support = false;

			caps.ubo_support = false;
		}
	}

	std::string fxml_name(argv[2]);
	filesystem::path fxml_path(fxml_name);
#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V2_SUPPORT
	std::string const base_name = fxml_path.stem();
#else
	std::string const base_name = fxml_path.stem().string();
#endif
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
#ifdef KLAYGE_TR2_LIBRARY_FILESYSTEM_V3_SUPPORT
			filesystem::copy_options::overwrite_existing);
#else
			filesystem::copy_option::overwrite_if_exists);
#endif
		kfx_path = target_folder / kfx_name;
	}

	cout << "Compiled kfx has been saved to " << kfx_path << "." << endl;

	Context::Destroy();

	return 0;
}
