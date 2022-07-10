#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include "CartoonPP.hpp"

using namespace KlayGE;

CartoonPostProcess::CartoonPostProcess()
		: PostProcess(L"Cartoon", false,
			MakeSpan<std::string>(),
			MakeSpan<std::string>({"normal_tex", "depth_tex", "color_tex"}),
			MakeSpan<std::string>({"output"}),
			RenderEffectPtr(), nullptr)
{
	auto effect = SyncLoadRenderEffect("CartoonPP.fxml");
	this->Technique(effect, effect->TechniqueByName("Cartoon"));
}

void CartoonPostProcess::InputPin(uint32_t index, ShaderResourceViewPtr const& srv)
{
	PostProcess::InputPin(index, srv);
	if ((0 == index) && srv)
	{
		auto const* tex = srv->TextureResource().get();
		*(effect_->ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
		*(effect_->ParameterByName("inv_far")) = 1.0f / Context::Instance().AppInstance().ActiveCamera().FarPlane();
	}
}
