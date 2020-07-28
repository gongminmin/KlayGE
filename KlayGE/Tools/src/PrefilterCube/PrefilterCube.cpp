#include <KlayGE/KlayGE.hpp>
#include <KFL/Half.hpp>
#include <KFL/Math.hpp>
#include <KFL/Timer.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/assert.hpp>

using namespace std;
using namespace KlayGE;

namespace
{
	void PrefilterCubeGPU(std::string const & in_file, std::string const & out_file)
	{
		TexturePtr in_tex = SyncLoadTexture(in_file, EAH_GPU_Read | EAH_Immutable);
		uint32_t in_width = in_tex->Width(0);

		uint32_t out_num_mipmaps = 1;
		{
			uint32_t w = in_width;
			while (w > 8)
			{
				++ out_num_mipmaps;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		auto in_srv = rf.MakeTextureSrv(in_tex);

		PostProcessPtr diff_pp = SyncLoadPostProcess("PrefilterCube.ppml", "PrefilterCubeDiffuse");
		PostProcessPtr spec_pp = SyncLoadPostProcess("PrefilterCube.ppml", "PrefilterCubeSpecular");
		diff_pp->InputPin(0, in_srv);
		spec_pp->InputPin(0, in_srv);

		TexturePtr out_tex = rf.MakeTextureCube(in_width, out_num_mipmaps, 1, EF_ABGR16F, 1, 0, EAH_GPU_Write);

		for (int face = 0; face < 6; ++ face)
		{
			in_tex->CopyToSubTextureCube(*out_tex, 0, static_cast<Texture::CubeFaces>(face), 0, 0, 0, in_width, in_width, 0,
				static_cast<Texture::CubeFaces>(face), 0, 0, 0, in_width, in_width, TextureFilter::Point);

			for (uint32_t level = 1; level < out_num_mipmaps - 1; ++ level)
			{
				float shininess = Glossiness2Shininess(static_cast<float>(out_num_mipmaps - 2 - level) / (out_num_mipmaps - 2));

				spec_pp->OutputPin(0, rf.Make2DRtv(out_tex, 0, static_cast<Texture::CubeFaces>(face), level));
				spec_pp->SetParam(0, face);
				spec_pp->SetParam(1, shininess);
				spec_pp->Apply();
			}

			{
				diff_pp->OutputPin(0, rf.Make2DRtv(out_tex, 0, static_cast<Texture::CubeFaces>(face), out_num_mipmaps - 1));
				diff_pp->SetParam(0, face);
				diff_pp->Apply();
			}
		}

		SaveTexture(out_tex, out_file);
	}
}

class PrefilterCubeApp : public KlayGE::App3DFramework
{
public:
	PrefilterCubeApp()
		: App3DFramework("PrefilterCube")
	{
		ResLoader::Instance().AddPath("../../Tools/media/PrefilterCube");
	}

	virtual void DoUpdateOverlay() override
	{
	}

	virtual uint32_t DoUpdate(uint32_t /*pass*/) override
	{
		return URV_Finished;
	}
};

int main(int argc, char* argv[])
{
	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.color_grading = false;
	context_cfg.graphics_cfg.gamma = false;
	Context::Instance().Config(context_cfg);

	PrefilterCubeApp app;
	app.Create();

	using namespace KlayGE;

	if (argc < 2)
	{
		cout << "Usage: PrefilterCube xxx.dds [xxx_filtered.dds]" << endl;
		return 1;
	}

	std::string input(argv[1]);
	std::string output;
	if (argc >= 3)
	{
		output = argv[2];
	}
	else
	{
		filesystem::path output_path(argv[1]);
		output = output_path.stem().string() + "_filtered.dds";
	}

	Timer timer;

	PrefilterCubeGPU(input, output);

	cout << timer.elapsed() << " s" << endl;
	cout << "Filtered cube map is saved into " << output << endl;

	return 0;
}
