// LightShaft.hpp
// KlayGE Light shaft类 实现文件
// Ver 4.2.0
// 版权所有(C) 龚敏敏, 2012
// Homepage: http://www.klayge.org
//
// 4.2.0
// 初次建立 (2012.9.23)
//
// 修改记录
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
	uint32_t const RADIAL_SAMPLE_NUM = 32;
	uint32_t const BLUR_ITERATE_NUM = 2;

	LightShaftPostProcess::LightShaftPostProcess()
		: PostProcess(L"LightShaft")
	{
		input_pins_.push_back(std::make_pair("depth_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("color_tex", TexturePtr()));

		params_.push_back(std::make_pair("light_pos", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("light_shaft_clr", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("light_intensity", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("shadow_intensity", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("depth_clip", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("cutoff_decay_weight", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("valid_sample_length", RenderEffectParameterPtr()));

		RenderFactory & rf = Context::Instance().RenderFactoryInstance();
		RenderEffectPtr effect = rf.LoadEffect("LightShaft.fxml");
		this->Technique(effect->TechniqueByName("ApplyLightShaft"));

		for (int i = 0; i < BLUR_ITERATE_NUM * 2; ++ i)
		{
			PostProcessPtr radial_blur_pp = LoadPostProcess(ResLoader::Instance().Open("LightShaft.ppml"), "LightShaftMaskRadialBlur");
			radial_blur_pp->SetParam(0, static_cast<int>(RADIAL_SAMPLE_NUM));
			radial_blur_pps_.push_back(radial_blur_pp);
		}

		apply_pp_ = LoadPostProcess(ResLoader::Instance().Open("LightShaft.ppml"), "ApplyLightShaftEffect");
	}

	void LightShaftPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		if (index < 2)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			uint32_t tex_width = (tex->Width(0) + 1) / 2;
			uint32_t tex_height = (tex->Height(0) + 1) / 2;

			TexturePtr blur_tex[2];
			blur_tex[0] = rf.MakeTexture2D(tex_width, tex_height, 1, 1, tex->Format(),
				1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			blur_tex[1] = rf.MakeTexture2D(tex_width, tex_height, 1, 1, tex->Format(),
				1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

			uint32_t start = index * BLUR_ITERATE_NUM;

			bool active_idx = false;
			for (uint32_t i = start; i < start + BLUR_ITERATE_NUM; ++ i)
			{
				radial_blur_pps_[i]->InputPin(0, blur_tex[active_idx]);
				radial_blur_pps_[i]->OutputPin(0, blur_tex[!active_idx]);
				active_idx = !active_idx;
			}

			radial_blur_pps_[start]->InputPin(0, tex);

			apply_pp_->InputPin(index, blur_tex[active_idx]);
			apply_pp_->SetParam(5, static_cast<float>(tex_width) / tex_height);
		}
	}

	void LightShaftPostProcess::Apply()
	{
		Camera& camera = Context::Instance().AppInstance().ActiveCamera();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		float4x4 const & proj_mat = camera.ProjMatrix();
		float4x4 const & view_mat = camera.ViewMatrix();

		float3 light_pos;
		params_[0].second->Value(light_pos);
		float4 proj_light_pos = MathLib::transform(light_pos, view_mat * proj_mat);

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

			float valid_sample_length;
			params_[6].second->Value(valid_sample_length);
		
			for (int i = 0; i < BLUR_ITERATE_NUM * 2; ++ i)
			{
				radial_blur_pps_[i]->SetParam(1, light_uv_pos);
				radial_blur_pps_[i]->SetParam(2, (1 - cutoff_decay_weight) / RADIAL_SAMPLE_NUM);
				radial_blur_pps_[i]->SetParam(3, valid_sample_length);
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
