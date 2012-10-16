#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/SSRPostProcess.hpp>

namespace KlayGE
{
	SSRPostProcess::SSRPostProcess()
			: PostProcess(L"ScreenSpaceReflection")
	{
		input_pins_.push_back(std::make_pair("g_buffer_0_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("g_buffer_1_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("front_side_depth_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("front_side_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("foreground_depth_tex", TexturePtr()));

		params_.push_back(std::make_pair("min_samples", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("max_samples", RenderEffectParameterPtr()));
		params_.push_back(std::make_pair("ray_length_fadeout_factor", RenderEffectParameterPtr()));

		RenderFactory & rf = Context::Instance().RenderFactoryInstance();
		RenderEffectPtr effect = rf.LoadEffect("SSR.fxml");
		this->Technique(effect->TechniqueByName("ScreenSpaceReflectionPostProcess"));

		if (technique_ && technique_->Validate())
		{
			proj_param_ = effect->ParameterByName("proj");
			inv_proj_param_ = effect->ParameterByName("inv_proj");
			near_q_param_ = effect->ParameterByName("near_q");
			ray_length_param_ = effect->ParameterByName("ray_length");
		}

		this->SetParam(0, static_cast<int32_t>(20));
		this->SetParam(1, static_cast<int32_t>(30));
		this->SetParam(2, 0.6f);
	}

	void SSRPostProcess::Apply()
	{
		Camera& camera = Context::Instance().AppInstance().ActiveCamera();
		float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());

		*proj_param_ = camera.ProjMatrix();
		*inv_proj_param_ = camera.InverseProjMatrix();
		*near_q_param_ = float2(camera.NearPlane() * q, q);
		*ray_length_param_ = camera.FarPlane() - camera.NearPlane();

		this->Render();
	}
}
