#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>

#include "NightVisionPP.hpp"

using namespace KlayGE;

NightVisionPostProcess::NightVisionPostProcess()
	: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("NightVisionPP.fxml")->TechniqueByName("NightVision"))
{
}

void NightVisionPostProcess::OnRenderBegin()
{
	PostProcess::OnRenderBegin();

	float elapsed_time = static_cast<float>(timer_.elapsed());
	timer_.restart();

	float2 sc;
	MathLib::sin_cos(elapsed_time * 50000.0f, sc.x(), sc.y());
	*(technique_->Effect().ParameterByName("noise_offset")) = 0.2f * sc;
}
