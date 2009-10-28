#ifndef _NIGHTVISIONPP_HPP
#define _NIGHTVISIONPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/PostProcess.hpp>

class NightVisionPostProcess : public KlayGE::PostProcess
{
public:
	NightVisionPostProcess();

	void OnRenderBegin();

private:
	KlayGE::Timer timer_;
};

#endif		// _NIGHTVISIONPP_HPP
