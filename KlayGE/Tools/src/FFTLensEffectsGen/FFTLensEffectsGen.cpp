#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/FFT.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace KlayGE;

class FFTLensEffectsGenApp : public KlayGE::App3DFramework
{
public:
	FFTLensEffectsGenApp()
		: App3DFramework("FFTLensEffectsGen")
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
		cout << "Usage: FFTLensEffectsGen xxx.dds" << endl;
		return 1;
	}

	std::string src_name = argv[1];

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	Context::Instance().Config(context_cfg);
	
	FFTLensEffectsGenApp app;
	app.Create();

	
	int const WIDTH = 512;
	int const HEIGHT = 512;

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	TexturePtr empty_tex;
	{
		std::vector<uint8_t> zero_data(WIDTH * HEIGHT, 0);
		ElementInitData resized_data;
		resized_data.data = &zero_data[0];
		resized_data.row_pitch = WIDTH * sizeof(uint8_t);
		resized_data.slice_pitch = WIDTH * HEIGHT * sizeof(uint8_t);
		empty_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_R8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, MakeSpan<1>(resized_data));
	}

	TexturePtr pattern_raw = SyncLoadTexture(src_name, EAH_CPU_Read);
	int width = static_cast<int>(pattern_raw->Width(0));
	int height = static_cast<int>(pattern_raw->Height(0));
	if (pattern_raw->Format() != EF_ABGR8)
	{
		TexturePtr pattern_refmt = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read);
		pattern_raw->CopyToSubTexture2D(*pattern_refmt, 0, 0, 0, 0, width, height, 0, 0, 0, 0, width, height);
		pattern_raw = pattern_refmt;
	}

	std::vector<float4> pattern_real(WIDTH * HEIGHT);
	
	{
		float3 LUM_WEIGHT(0.2126f, 0.7152f, 0.0722f);

		Texture::Mapper mapper(*pattern_raw, 0, 0, TMA_Read_Only, 0, 0, width, height);

		float sum_lum = 0;
		uint8_t const * p = mapper.Pointer<uint8_t>();
		uint32_t const pitch = mapper.RowPitch() / sizeof(uint8_t);
		for (int y = 0; y < height; ++ y)
		{
			int start_y;
			if (y < height / 2)
			{
				start_y = HEIGHT - height / 2;
			}
			else
			{
				start_y = -height / 2;
			}

			for (int x = 0; x < width; ++ x)
			{
				int start_x;
				if (x < width / 2)
				{
					start_x = WIDTH - width / 2;
				}
				else
				{
					start_x = -width / 2;
				}

				float r = p[y * pitch + x * 4 + 0] / 255.0f;
				float g = p[y * pitch + x * 4 + 1] / 255.0f;
				float b = p[y * pitch + x * 4 + 2] / 255.0f;
				uint8_t a = p[y * pitch + x * 4 + 3];
				if (0 == a)
				{
					r = g = b = 0;
				}

				sum_lum += MathLib::dot(float3(r, g, b), LUM_WEIGHT);

				pattern_real[(y + start_y) * WIDTH + (x + start_x)] = float4(r, g, b, 1);
			}
		}

		float scale = 1.0f / sum_lum;
		for (int y = 0; y < HEIGHT; ++ y)
		{
			for (int x = 0; x < WIDTH; ++ x)
			{
				pattern_real[y * WIDTH + x] *= scale;
			}
		}
	}

	ElementInitData pattern_real_data;
	pattern_real_data.data = &pattern_real[0];
	pattern_real_data.row_pitch = WIDTH * sizeof(float4);
	pattern_real_data.slice_pitch = WIDTH * HEIGHT * sizeof(float4);
	TexturePtr real_tex =
		rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, MakeSpan<1>(pattern_real_data));

	TexturePtr pattern_real_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	TexturePtr pattern_imag_tex = rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

	GpuFftPS fft(WIDTH, HEIGHT, true);
	fft.Execute(pattern_real_tex, pattern_imag_tex, rf.MakeTextureSrv(real_tex), rf.MakeTextureSrv(empty_tex));

	SaveTexture(pattern_real_tex, "lens_effects_real.dds");
	SaveTexture(pattern_imag_tex, "lens_effects_imag.dds");

	Context::Destroy();
}
