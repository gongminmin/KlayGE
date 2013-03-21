#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>

#include "NightVisionPP.hpp"

using namespace KlayGE;

NightVisionPostProcess::NightVisionPostProcess()
	: PostProcess(L"NightVision",
			std::vector<std::string>(),
			std::vector<std::string>(1, "src_tex"),
			std::vector<std::string>(1, "output"),
			SyncLoadRenderEffect("NightVisionPP.fxml")->TechniqueByName("NightVision"))
{
}

void NightVisionPostProcess::OnRenderBegin()
{
	PostProcess::OnRenderBegin();

	float elapsed_time = Context::Instance().AppInstance().FrameTime();

	float2 sc;
	MathLib::sincos(elapsed_time * 50000.0f, sc.x(), sc.y());
	*(technique_->Effect().ParameterByName("noise_offset")) = 0.2f * sc;
}
