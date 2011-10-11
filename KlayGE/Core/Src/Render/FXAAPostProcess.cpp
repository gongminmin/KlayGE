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
		output_pins_.push_back(std::make_pair("output", TexturePtr()));

		RenderEffectPtr effect = Context::Instance().RenderFactoryInstance().LoadEffect("FXAA.fxml");
		fxaa_tech_ = effect->TechniqueByName("FXAA");
		show_edge_tech_ = effect->TechniqueByName("FXAAShowEdge");

		this->Technique(fxaa_tech_);
	}

	void FXAAPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		PostProcess::InputPin(index, tex);
		if (0 == index)
		{
			*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
		}
	}

	void FXAAPostProcess::ShowEdge(bool se)
	{
		if (se)
		{
			technique_ = show_edge_tech_;
		}
		else
		{
			technique_ = fxaa_tech_;
		}
	}
}
