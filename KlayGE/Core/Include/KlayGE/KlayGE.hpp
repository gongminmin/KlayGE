// KlayGE.hpp
// KlayGE 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _KLAYGE_HPP
#define _KLAYGE_HPP

#define KLAYGEMAINVER		2
#define KLAYGESECVER		0
#define KLAYGERELEASEVER	0
#define KLAYGEVERSTR		"KLAYGE 2.0.0"

#define CREATOR(p)	friend class p

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

	enum CPUOptimal
	{
		CPU_Standard,
		CPU_MMX,
		CPU_AMD3DNowEx,
	};
}

#include <KlayGE/Types.hpp>

#endif		// _KLAYGE_HPP