#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/PostProcess.hpp>

class SSSBlurPP : public KlayGE::PostProcess
{
public:
	SSSBlurPP();

	virtual void InputPin(uint32_t index, KlayGE::TexturePtr const & tex) KLAYGE_OVERRIDE;
	using PostProcess::InputPin;

	virtual void Apply() KLAYGE_OVERRIDE;

private:
	KlayGE::FrameBufferPtr temp_fb_;
	KlayGE::TexturePtr temp_x_tex_;
	KlayGE::TexturePtr temp_y_tex_;
	KlayGE::RenderTechniquePtr blur_x_tech_;
	KlayGE::RenderTechniquePtr blur_y_techs_[6];
	KlayGE::RenderEffectParameterPtr color_tex_param_;
	KlayGE::RenderEffectParameterPtr step_param_;
};
