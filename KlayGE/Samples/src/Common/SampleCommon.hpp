#ifndef _SAMPLECOMMON_HPP
#define _SAMPLECOMMON_HPP

#pragma once

#ifndef SAMPLE_COMMON_SOURCE
#define KLAYGE_LIB_NAME SampleCommon
#include <KFL/Detail/AutoLink.hpp>

#ifdef KLAYGE_COMPILER_MSVC
extern "C"
{
	KLAYGE_SYMBOL_EXPORT KlayGE::uint32_t NvOptimusEnablement = 0x00000001;
}
#endif
#endif

int SampleMain();

inline int EntryFunc()
{
	KlayGE::ResLoader::Instance().AddPath("../../Samples/media/Common");

	KlayGE::Context::Instance().LoadCfg("KlayGE.cfg");

	return SampleMain();
}

#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ /*args*/)
{
	return EntryFunc();
}
#endif


#endif		// _SAMPLECOMMON_HPP
