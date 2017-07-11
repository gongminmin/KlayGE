#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include "CartoonPP.hpp"

using namespace KlayGE;

CartoonPostProcess::CartoonPostProcess()
		: PostProcess(L"Cartoon", false,
			{},
			{ "normal_tex", "depth_tex", "color_tex" },
			{ "output" },
			RenderEffectPtr(), nullptr)
{
	auto effect = SyncLoadRenderEffect("CartoonPP.fxml");
	this->Technique(effect, effect->TechniqueByName("Cartoon"));
}

void CartoonPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
{
	PostProcess::InputPin(index, tex);
	if ((0 == index) && tex)
	{
		*(effect_->ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
		*(effect_->ParameterByName("inv_far")) = 1.0f / Context::Instance().AppInstance().ActiveCamera().FarPlane();
	}
}
