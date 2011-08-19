#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/SSVOPostProcess.hpp>

namespace KlayGE
{
	SSVOPostProcess::SSVOPostProcess()
			: PostProcess(L"SSAO",
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "out_tex"),
					Context::Instance().RenderFactoryInstance().LoadEffect("SSVO.fxml")->TechniqueByName("SSVO"))
	{
		depth_near_far_invfar_param_ = technique_->Effect().ParameterByName("depth_near_far_invfar");
		proj_param_ = technique_->Effect().ParameterByName("proj");
		inv_proj_param_ = technique_->Effect().ParameterByName("inv_proj");
	}

	void SSVOPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*depth_near_far_invfar_param_ = float3(camera.NearPlane(), camera.FarPlane(), 1 / camera.FarPlane());

		float4x4 const & proj = camera.ProjMatrix();
		*proj_param_ = proj;
		*inv_proj_param_ = MathLib::inverse(proj);
	}
}
