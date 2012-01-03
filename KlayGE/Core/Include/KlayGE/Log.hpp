// Log.hpp
// KlayGE Cross-platform log utilities header file
// Ver 4.1.0
// Copyright(C) Minmin Gong, 2012
// Homepage: http://www.klayge.org
//
// 4.1.0
// First release (2012.1.3)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#ifndef _LOG_HPP
#define _LOG_HPP

#pragma once

namespace KlayGE
{
	void LogInfo(char const * fmt, ...);
	void LogWarn(char const * fmt, ...);
	void LogError(char const * fmt, ...);
}

#endif		// _TRACE_HPP
