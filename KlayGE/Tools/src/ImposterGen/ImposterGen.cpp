/**
 * @file ImposterGen.cpp
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Camera.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <KlayGE/RenderFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

using namespace std;
using namespace KlayGE;

class ImposterGenApp : public KlayGE::App3DFramework
{
public:
	ImposterGenApp()
		: App3DFramework("ImposterGen")
	{
	}

private:
	void OnCreate() override
	{
	}

	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height) override
	{
		App3DFramework::OnResize(width, height);
	}

	void DoUpdateOverlay() override
	{
	}

	KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass) override
	{
		KFL_UNUSED(pass);
		return App3DFramework::URV_Finished;
	}
};

void GeneratesImposters(std::string const & model_name, std::string const & target_folder, std::string const & output_name,
	uint32_t num_azimuth, uint32_t num_elevation, uint32_t size)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	TexturePtr impostors_g_buffer_rt0 = rf.MakeTexture2D(size * num_azimuth, size * num_elevation,
		0, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
	TexturePtr impostors_g_buffer_rt1 = rf.MakeTexture2D(size * num_azimuth, size * num_elevation,
		0, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);

	FrameBufferPtr imposter_fb = rf.MakeFrameBuffer();
	imposter_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(impostors_g_buffer_rt0, 0, 1, 0));
	imposter_fb->Attach(FrameBuffer::Attachment::Color1, rf.Make2DRtv(impostors_g_buffer_rt1, 0, 1, 0));
	imposter_fb->Attach(rf.Make2DDsv(size * num_azimuth, size * num_elevation, EF_D24S8, 1, 0));
	auto const& imposter_camera = imposter_fb->Viewport()->Camera();
	imposter_fb->Viewport()->Width(size);
	imposter_fb->Viewport()->Height(size);

	imposter_fb->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
		Color(0, 0, 0, 0), 1, 0);

	auto scene_model = SyncLoadModel(model_name, EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr);

	auto const & aabbox = scene_model->RootNode()->PosBoundOS();
	float3 const dimensions = aabbox.Max() - aabbox.Min();
	float const diag_length = MathLib::length(dimensions);

	float const azimuth_angle_step = PI * 2 / num_azimuth;
	float const elevation_angle_step = PI * 2 / num_elevation;
	for (size_t azimuth_index = 0; azimuth_index < num_azimuth; ++ azimuth_index)
	{
		float const azimuth = azimuth_index * azimuth_angle_step;

		imposter_fb->Viewport()->Left(static_cast<int>(azimuth_index * size));

		for (size_t elevation_index = 0; elevation_index < num_elevation; ++ elevation_index)
		{
			float const elevation = elevation_index * elevation_angle_step;

			float y = sin(elevation);
			float hyp = cos(elevation);
			float x = hyp * sin(azimuth);
			float z = hyp * cos(azimuth);
			float3 eye_vec(x, y, z);
			float3 up_vec;
			if (MathLib::abs(eye_vec.y()) < 1 - 1e-6f)
			{
				up_vec = float3(0, 1, 0);
			}
			else
			{
				up_vec = float3(0, 0, 1);
			}

			float3 imposter_eye_pos = aabbox.Center() + eye_vec * diag_length * 0.5f;
			imposter_camera->LookAtDist(MathLib::length(aabbox.Center() - imposter_eye_pos));
			imposter_camera->BoundSceneNode()->TransformToWorld(
				MathLib::inverse(MathLib::look_at_lh(imposter_eye_pos, aabbox.Center(), up_vec)));

			AABBox const aabb_es = MathLib::transform_aabb(aabbox, imposter_camera->ViewMatrix());
			imposter_camera->ProjOrthoOffCenterParams(aabb_es.Min().x(), aabb_es.Max().y(), aabb_es.Max().x(), aabb_es.Min().y(),
				0.01f, diag_length);

			imposter_fb->Viewport()->Top(static_cast<int>(elevation_index * size));

			re.BindFrameBuffer(imposter_fb);

			for (uint32_t i = 0; i < scene_model->NumMeshes(); ++ i)
			{
				auto mesh = scene_model->Mesh(i).get();

				while (!mesh->AllHWResourceReady());
				mesh->Pass(PT_OpaqueGBuffer);
				mesh->Render();
			}
		}
	}

	impostors_g_buffer_rt0->BuildMipSubLevels(TextureFilter::Linear);
	impostors_g_buffer_rt1->BuildMipSubLevels(TextureFilter::Linear);

	std::string rt0_name = output_name + ".imposter.rt0.dds";
	std::string rt1_name = output_name + ".imposter.rt1.dds";
	SaveTexture(impostors_g_buffer_rt0, target_folder + "/" + rt0_name);
	SaveTexture(impostors_g_buffer_rt1, target_folder + "/" + rt1_name);

	std::ofstream ofs((target_folder + "/" + output_name + ".impml").c_str());
	ofs << "<?xml version='1.0'?>" << std::endl << std::endl;
	ofs << "<imposter>" << std::endl;
	ofs << "\t<azimuth value=\"" << num_azimuth << "\"/>" << std::endl;
	ofs << "\t<elevation value=\"" << num_elevation << "\"/>" << std::endl;
	ofs << "\t<size value=\"" << size << "\"/>" << std::endl;
	ofs << "\t<rt0 name=\"" << rt0_name << "\"/>" << std::endl;
	ofs << "\t<rt1 name=\"" << rt1_name << "\"/>" << std::endl;
	ofs << "</imposter>" << std::endl;
}

int main(int argc, char* argv[])
{
	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.deferred_rendering = true;
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.color_grading = false;
	context_cfg.graphics_cfg.gamma = false;
	Context::Instance().Config(context_cfg);

	ImposterGenApp app;
	app.Create();

	std::string input_name;
	FILESYSTEM_NS::path target_folder;
	uint32_t azimuth;
	uint32_t elevation;
	uint32_t size;
	bool quiet = false;

	cxxopts::Options options("ImposterGen", "KlayGE Imposter Generator");
	// clang-format off
	options.add_options()
		("H,help", "Produce help message")
		("I,input-name", "Input model name.", cxxopts::value<std::string>())
		("T,target-folder", "Target folder.", cxxopts::value<std::string>())
		("A,azimuth", "Num of view angles in XoZ plane.", cxxopts::value<uint32_t>(azimuth)->default_value("8"))
		("E,elevation", "Num of view angles in XoY plane.", cxxopts::value<uint32_t>(elevation)->default_value("8"))
		("S,size", "Size of each imposter.", cxxopts::value<uint32_t>(size)->default_value("256"))
		("q,quiet", "Quiet mode.", cxxopts::value<bool>()->implicit_value("true"))
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
		cout << "KlayGE Imposter Generator, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-name") > 0)
	{
		input_name = vm["input-name"].as<std::string>();
	}
	else
	{
		cout << "Need input model name." << endl;
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("target-folder") > 0)
	{
		target_folder = vm["target-folder"].as<std::string>();
	}

	std::string model_name = ResLoader::Instance().Locate(input_name);

	std::string file_name;
	FILESYSTEM_NS::path model_path(model_name);
	if (target_folder.empty())
	{
		target_folder = model_path.parent_path();
	}
	file_name = model_path.stem().string();

	GeneratesImposters(model_name, target_folder.string(), file_name, azimuth, elevation, size);

	if (!quiet)
	{
		cout << "Imposter has been saved to " << target_folder.string() << "/" << file_name << ".impml" << endl;
	}

	return 0;
}
