#ifndef _NIGHTVISIONPP_HPP
#define _NIGHTVISIONPP_HPP

#include <KlayGE/PostProcess.hpp>

class NightVisionPostProcess : public KlayGE::PostProcess
{
public:
	NightVisionPostProcess();

	void OnRenderBegin();
};

#endif		// _NIGHTVISIONPP_HPP
