#ifndef _SAMPLECOMMON_HPP
#define _SAMPLECOMMON_HPP

#pragma once

/*#ifdef KLAYGE_COMPILER_MSVC
	#ifdef KLAYGE_DEBUG
		#define DEBUG_SUFFIX "_d"
	#else
		#define DEBUG_SUFFIX ""
	#endif

	#define LIB_FILE_NAME "SampleCommon_"KFL_STRINGIZE(KLAYGE_COMPILER_NAME)"_"KFL_STRINGIZE(KLAYGE_COMPILER_TARGET) DEBUG_SUFFIX ".lib"

	#pragma comment(lib, LIB_FILE_NAME)

	#undef LIB_FILE_NAME
	#undef DEBUG_SUFFIX
#endif*/

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	_declspec(dllexport) KlayGE::uint32_t NvOptimusEnablement = 0x00000001;
}
#endif

int SampleMain();

#ifdef KLAYGE_PLATFORM_WINDOWS_METRO
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ /*args*/)
#else
int main()
#endif
{
	KlayGE::ResLoader::Instance().AddPath("../../Samples/media/Common");

	KlayGE::Context::Instance().LoadCfg("KlayGE.cfg");

	return SampleMain();
}

#endif		// _SAMPLECOMMON_HPP
