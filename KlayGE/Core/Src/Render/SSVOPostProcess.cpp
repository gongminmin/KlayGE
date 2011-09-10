#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/SSVOPostProcess.hpp>

namespace KlayGE
{
	SSVOPostProcess::SSVOPostProcess()
			: PostProcess(L"SSAO")
	{
		input_pins_.push_back(std::make_pair("g_buffer_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("depth_tex", TexturePtr()));

		output_pins_.push_back(std::make_pair("out_tex", TexturePtr()));

		this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("SSVO.fxml")->TechniqueByName("SSVO"));

		depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
		proj_param_ = technique_->Effect().ParameterByName("proj");
		inv_proj_param_ = technique_->Effect().ParameterByName("inv_proj");
	}

	void SSVOPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		float4x4 const & proj = camera.ProjMatrix();
		*proj_param_ = proj;
		*inv_proj_param_ = MathLib::inverse(proj);
	}
}
