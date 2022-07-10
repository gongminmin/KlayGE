#ifndef _CARTOONPP_HPP
#define _CARTOONPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

class CartoonPostProcess : public KlayGE::PostProcess
{
public:
	CartoonPostProcess();

	void InputPin(KlayGE::uint32_t index, KlayGE::ShaderResourceViewPtr const& srv) override;
	using PostProcess::InputPin;
};

#endif		// _CARTOONPP_HPP
