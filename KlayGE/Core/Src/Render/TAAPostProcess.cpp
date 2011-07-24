#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/TAAPostProcess.hpp>

namespace KlayGE
{
	TAAPostProcess::TAAPostProcess()
		: PostProcess(L"TAA"),
			cur_jitter_index_(0)
	{
		input_pins_.push_back(std::make_pair("pre_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("cur_tex", TexturePtr()));

		this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("TAA.fxml")->TechniqueByName("TemporalAA"));
	}

	void TAAPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		if (0 == index)
		{
			if (!temporal_tex_[0])
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				uint32_t width = tex->Width(0);
				uint32_t height = tex->Height(0);
				temporal_tex_[0] = rf.MakeTexture2D(width, height, 1, 1, tex->Format(), 1, 0, EAH_GPU_Read, NULL);
				tex->CopyToSubTexture2D(*temporal_tex_[0], 0, 0, 0, 0, width, tex->Height(0), 0, 0, 0, 0, width, height);
			}
			temporal_tex_[1] = tex;
		}
	}

	void TAAPostProcess::Apply()
	{
		PostProcess::InputPin(0, temporal_tex_[0]);
		PostProcess::InputPin(1, temporal_tex_[1]);

		PostProcess::Apply();

		uint32_t width = temporal_tex_[0]->Width(0);
		uint32_t height = temporal_tex_[0]->Height(0);
		temporal_tex_[1]->CopyToSubTexture2D(*temporal_tex_[0], 0, 0, 0, 0, width, height, 0, 0, 0, 0, width, height);

		cur_jitter_index_ = (cur_jitter_index_ + 1) & 1;
	}

	void TAAPostProcess::UpdateJitterProj(Camera const & camera, uint32_t win_width, uint32_t win_height)
	{
		float top = camera.NearPlane() * tanf(camera.FOV() / 2);
		float bottom = -top;
		float right = top * camera.Aspect();
		float left = -right;
		float width = right - left;
		float height = top - bottom;

		float const pixel_dx[2] = { -0.25f, +0.25f };
		float const pixel_dy[2] = { +0.25f, -0.25f };
		float dx = -pixel_dx[cur_jitter_index_] * width / static_cast<float>(win_width);
		float dy = -pixel_dy[cur_jitter_index_] * height / static_cast<float>(win_height);
		jitter_proj_ = MathLib::perspective_off_center_lh(left + dx, right + dx, bottom + dy, top + dy,
			                                             camera.NearPlane(), camera.FarPlane());
	}
}
