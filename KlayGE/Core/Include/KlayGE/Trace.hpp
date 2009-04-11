// Trace.hpp
// KlayGE 跟踪器 头文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 初次建立 (2003.11.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _TRACE_HPP
#define _TRACE_HPP

#pragma KLAYGE_ONCE

#include <iostream>

namespace KlayGE
{
	// 可以跟踪函数执行的类
	// 开销较大，最好只在调试的时候用一下
	class Trace
	{
	public:
		Trace(char* const func, int line = 0, char* const file = NULL)
			: func_(func), line_(line), file_(file)
		{
#ifdef KLAYGE_DEBUG
			std::clog << "Calling " << func_ << " in file " << file_ << " on line " << line_ << std::endl;
#endif
		}

		~Trace()
		{
#ifdef KLAYGE_DEBUG
			std::clog << "Leaving " << func_ << " in file " << file_ << " on line " << line_ << std::endl;
#endif
		}

	private:
		char* const func_;
		int line_;
		char* const file_;
	};
}

#endif		// _TRACE_HPP
