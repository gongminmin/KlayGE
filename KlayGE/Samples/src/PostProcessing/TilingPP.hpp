#ifndef _TILINGPP_HPP
#define _TILINGPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

class TilingPostProcess : public KlayGE::PostProcess
{
public:
	TilingPostProcess();

	void InputPin(KlayGE::uint32_t index, KlayGE::TexturePtr const & tex);
	KlayGE::TexturePtr const & InputPin(KlayGE::uint32_t index) const;
	void Apply();
	void OnRenderBegin();

private:
	KlayGE::PostProcessPtr downsampler_;
	KlayGE::TexturePtr downsample_tex_;

	KlayGE::RenderEffectParameter* tile_per_row_line_ep_;
};

#endif		// _TILINGPP_HPP
