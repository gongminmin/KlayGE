// LightShaft.hpp
// KlayGE Light shaft�� ʵ���ļ�
// Ver 4.2.0
// ��Ȩ����(C) ������, 2012
// Homepage: http://www.klayge.org
//
// 4.2.0
// ���ν��� (2012.9.23)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderEngine.hpp>

#include <KlayGE/LightShaft.hpp>

namespace KlayGE
{
	uint32_t const RADIAL_SAMPLE_NUM = 16;
	uint32_t const BLUR_ITERATE_NUM = 2;

	LightShaftPostProcess::LightShaftPostProcess()
		: PostProcess(L"LightShaft")
	{
		input_pins_.emplace_back("color_tex", TexturePtr());
		input_pins_.emplace_back("depth_tex", TexturePtr());

		params_.emplace_back("light_pos", RenderEffectParameterPtr());
		params_.emplace_back("light_shaft_clr", RenderEffectParameterPtr());
		params_.emplace_back("light_intensity", RenderEffectParameterPtr());
		params_.emplace_back("shadow_intensity", RenderEffectParameterPtr());
		params_.emplace_back("depth_clip", RenderEffectParameterPtr());
		params_.emplace_back("cutoff_decay_weight", RenderEffectParameterPtr());

		RenderEffectPtr effect = SyncLoadRenderEffect("LightShaft.fxml");
		this->Technique(effect->TechniqueByName("ApplyLightShaft"));

		for (uint32_t i = 0; i < BLUR_ITERATE_NUM; ++ i)
		{
			PostProcessPtr radial_blur_pp = SyncLoadPostProcess("LightShaft.ppml", 0 == i ? "LightShaftRadialBlurCombine" : "LightShaftRadialBlur");
			radial_blur_pp->SetParam(0, float2(static_cast<float>(RADIAL_SAMPLE_NUM), 1.0f / RADIAL_SAMPLE_NUM));
			radial_blur_pps_.push_back(radial_blur_pp);
		}

		apply_pp_ = SyncLoadPostProcess("LightShaft.ppml", "ApplyLightShaftEffect");
	}

	void LightShaftPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		if (index < 2)
		{
			radial_blur_pps_[0]->InputPin(index, tex);

			if (0 == index)
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();

				uint32_t const tex_width = tex->Width(0) / 4;
				uint32_t const tex_height = tex->Height(0) / 4;

				TexturePtr blur_tex[2];
				blur_tex[0] = rf.MakeTexture2D(tex_width, tex_height, 1, 1, EF_ABGR16F,
					1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				blur_tex[1] = rf.MakeTexture2D(tex_width, tex_height, 1, 1, EF_ABGR16F,
					1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

				bool active_idx = false;
				radial_blur_pps_[0]->OutputPin(0, blur_tex[active_idx]);
				for (uint32_t i = 1; i < BLUR_ITERATE_NUM; ++ i)
				{
					radial_blur_pps_[i]->InputPin(0, blur_tex[active_idx]);
					radial_blur_pps_[i]->OutputPin(0, blur_tex[!active_idx]);
					active_idx = !active_idx;
				}

				apply_pp_->InputPin(0, blur_tex[active_idx]);
				apply_pp_->SetParam(5, static_cast<float>(tex_width) / tex_height);
			}
		}
	}

	void LightShaftPostProcess::Apply()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		float3 light_pos;
		params_[0].second->Value(light_pos);
		float4 proj_light_pos = MathLib::transform(light_pos, camera.ViewProjMatrix());

		if (proj_light_pos.w() >= 0)
		{
			proj_light_pos /= proj_light_pos.w();
			float2 light_uv_pos = float2(proj_light_pos.x(), proj_light_pos.y());
			light_uv_pos *= 0.5f;
			light_uv_pos.y() *= re.RequiresFlipping() ? -1 : +1;
			light_uv_pos += 0.5f;
			apply_pp_->SetParam(4, light_uv_pos);

			float cutoff_decay_weight;
			params_[5].second->Value(cutoff_decay_weight);

			for (size_t i = 0; i < radial_blur_pps_.size(); ++ i)
			{
				radial_blur_pps_[i]->SetParam(1, light_uv_pos);
				radial_blur_pps_[i]->SetParam(2, (1 - cutoff_decay_weight) / RADIAL_SAMPLE_NUM);
				radial_blur_pps_[i]->SetParam(3, float2(camera.FarPlane(), 1 / camera.FarPlane()));
				radial_blur_pps_[i]->Apply();
			}

			float depth_clip;
			params_[4].second->Value(depth_clip);
			float inv_frustum_depth = 1 / (camera.FarPlane() * depth_clip);
			apply_pp_->SetParam(0, inv_frustum_depth);

			float4 light_shaft_clr;
			params_[1].second->Value(light_shaft_clr);
			apply_pp_->SetParam(1, light_shaft_clr);

			float light_intensity;
			params_[2].second->Value(light_intensity);
			apply_pp_->SetParam(2, light_intensity);

			float shadow_intensity;
			params_[3].second->Value(shadow_intensity);
			apply_pp_->SetParam(3, shadow_intensity);

			apply_pp_->Apply();
		}
	}
}
