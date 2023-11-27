#ifndef _SAMPLECOMMON_HPP
#define _SAMPLECOMMON_HPP

#pragma once

#include <nonstd/scope.hpp>

#ifndef NO_NvOptimus
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
	auto on_exit = nonstd::make_scope_exit([] { KlayGE::Context::Destroy(); });

	auto& context = KlayGE::Context::Instance();
	context.ResLoaderInstance().AddPath("../../Samples/media/Common");

	context.LoadCfg("KlayGE.cfg");

	return SampleMain();
}

#endif		// _SAMPLECOMMON_HPP
