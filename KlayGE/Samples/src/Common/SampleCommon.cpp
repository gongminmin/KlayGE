#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#include "SampleCommon.hpp"

int main()
{
	KlayGE::ResLoader::Instance().AddPath("../../Samples/media/Common");

	KlayGE::Context::Instance().LoadCfg("KlayGE.cfg");

	return SampleMain();
}
