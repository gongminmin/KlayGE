// Trace.hpp
// KlayGE 跟踪器 头文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.2.0
// 初次建立 (2003.11.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _TRACE_HPP
#define _TRACE_HPP

#pragma once

#include <KlayGE/Log.hpp>

namespace KlayGE
{
	// 可以跟踪函数执行的类
	// 开销较大，最好只在调试的时候用一下
	class Trace
	{
	public:
		Trace(char const * func, int line = 0, char const * file = nullptr)
			: func_(func), line_(line), file_(file)
		{
#ifdef KLAYGE_DEBUG
			LOG_Info("Enter %s in file %s (line %d)", func_, file_ != nullptr ? file_ : "", line_);
#endif
		}

		~Trace()
		{
#ifdef KLAYGE_DEBUG
			LOG_Info("Leave %s in file %s (line %d)", func_, file_ != nullptr ? file_ : "", line_);
#endif
		}

	private:
		char const * func_;
		int line_;
		char const * file_;
	};
}

#endif		// _TRACE_HPP
