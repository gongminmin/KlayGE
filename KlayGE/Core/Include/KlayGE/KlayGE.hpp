// KlayGE.hpp
// KlayGE 头文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 去掉了汇编代码 (2004.4.20)
//
// 2.0.0
// 初次建立 (2003.8.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _KLAYGE_HPP
#define _KLAYGE_HPP

#define KLAYGEMAINVER		2
#define KLAYGESECVER		1
#define KLAYGERELEASEVER	0
#define KLAYGEVERSTR		"KLAYGE 2.1.0"

namespace KlayGE
{
	template <typename T>
	inline void
	SafeRelease(T& p)
	{
		if (p != 0)
		{
			p->Release();
			p = 0;
		}
	}

	template <typename T>
	inline void
	SafeDelete(T& p)
	{
		if (p != 0)
		{
			delete p;
			p = 0;
		}
	}

	template <typename T>
	inline void
	SafeDeleteArray(T& p)
	{
		if (p != 0)
		{
			delete[] p;
			p = 0;
		}
	}
}

#include <KlayGE/Types.hpp>

#endif		// _KLAYGE_HPP
