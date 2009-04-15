#ifndef _CARTOONPP_HPP
#define _CARTOONPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

class CartoonPostProcess : public KlayGE::PostProcess
{
public:
	CartoonPostProcess();

	void Source(KlayGE::TexturePtr const & tex, bool flipping);
	void ColorTex(KlayGE::TexturePtr const & tex);
	void OnRenderBegin();
};

#endif		// _CARTOONPP_HPP
