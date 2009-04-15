#ifndef _ASCIIARTSPP_HPP
#define _ASCIIARTSPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

class AsciiArtsPostProcess : public KlayGE::PostProcess
{
public:
	AsciiArtsPostProcess();

	void Source(KlayGE::TexturePtr const & tex, bool flipping);
	void Apply();
	void OnRenderBegin();

private:
	KlayGE::PostProcessPtr downsampler_;
	KlayGE::TexturePtr downsample_tex_;
	KlayGE::FrameBufferPtr downsample_fb_;

	KlayGE::RenderEffectParameterPtr cell_per_row_line_ep_;
};

#endif		// _ASCIIARTSPP_HPP
