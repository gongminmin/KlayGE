#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/SSVOPostProcess.hpp>

namespace KlayGE
{
	SSVOPostProcess::SSVOPostProcess()
			: PostProcess(L"SSVO")
	{
		input_pins_.push_back(std::make_pair("g_buffer_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("depth_tex", TexturePtr()));

		output_pins_.push_back(std::make_pair("out_tex", TexturePtr()));

		this->Technique(SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVO"));

		proj_param_ = technique_->Effect().ParameterByName("proj");
		inv_proj_param_ = technique_->Effect().ParameterByName("inv_proj");
		far_plane_param_ = technique_->Effect().ParameterByName("far_plane");
	}

	void SSVOPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*proj_param_ = camera.ProjMatrix();
		*inv_proj_param_ = camera.InverseProjMatrix();
		*far_plane_param_ = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
	}
}
