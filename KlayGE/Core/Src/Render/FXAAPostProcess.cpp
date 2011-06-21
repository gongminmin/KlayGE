#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/FXAAPostProcess.hpp>

namespace KlayGE
{
	FXAAPostProcess::FXAAPostProcess()
		: PostProcess(L"FXAA")
	{
		input_pins_.push_back(std::make_pair("color_tex", TexturePtr()));

		this->Technique(Context::Instance().RenderFactoryInstance().LoadEffect("FXAA.fxml")->TechniqueByName("FXAA"));
	}

	void FXAAPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		PostProcess::InputPin(index, tex);
		if (0 == index)
		{
			*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
		}
	}
}
