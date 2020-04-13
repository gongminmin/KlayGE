#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#endif

#include "SampleCommon.hpp"

#ifdef KLAYGE_PLATFORM_ANDROID
void android_main(android_app* state)
{
	KlayGE::Context::Instance().AppState(state);
	EntryFunc();
}
#else
int main()
{
	return EntryFunc();
}
#endif
