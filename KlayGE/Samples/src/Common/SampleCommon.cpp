#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>

#include "SampleCommon.hpp"

using namespace KlayGE;

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS_METRO
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ /*args*/)
#else
int main()
#endif
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	return SampleMain();
}
