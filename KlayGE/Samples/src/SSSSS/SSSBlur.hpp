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
	bool mrt_support_;

	KlayGE::FrameBufferPtr blur_x_fb_;
	KlayGE::FrameBufferPtr blur_y_fb_;
	KlayGE::TexturePtr blur_x_tex_;
	KlayGE::TexturePtr blur_y_tex_;
	KlayGE::RenderTechniquePtr copy_tech_;
	KlayGE::RenderTechniquePtr blur_x_tech_;
	KlayGE::RenderTechniquePtr blur_y_techs_[3];
	KlayGE::RenderTechniquePtr accum_techs_[3];
	KlayGE::RenderEffectParameterPtr color_tex_param_;
	KlayGE::RenderEffectParameterPtr step_param_;
};
