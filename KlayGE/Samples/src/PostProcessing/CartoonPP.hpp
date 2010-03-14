#ifndef _CARTOONPP_HPP
#define _CARTOONPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

class CartoonPostProcess : public KlayGE::PostProcess
{
public:
	CartoonPostProcess();

	void InputPin(KlayGE::uint32_t index, KlayGE::TexturePtr const & tex, bool flipping);
};

#endif		// _CARTOONPP_HPP
