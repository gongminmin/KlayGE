#ifndef _TILINGPP_HPP
#define _TILINGPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

class TilingPostProcess : public KlayGE::PostProcess
{
public:
	TilingPostProcess();

	void Source(KlayGE::TexturePtr const & tex, bool flipping);
	void Apply();
	void OnRenderBegin();

private:
	KlayGE::PostProcessPtr downsampler_;
	KlayGE::TexturePtr downsample_tex_;
	KlayGE::FrameBufferPtr downsample_fb_;

	KlayGE::RenderEffectParameterPtr tile_per_row_line_ep_;
};

#endif		// _TILINGPP_HPP
