// Memory.cpp
// KlayGE 内存函数库 实现文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 去掉了汇编代码 (2004.4.20)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Memory.hpp>

#include <memory.h>

/*
算法:
void* MemSet(void* pDest, int c, size_t Count)
{
	while (0 != Count)
	{
		*pDest = c;
		++ pDest;
		-- Count;
	}

	return pDest;
}

bool MemCmp(const void* pBuf1, const void* pBuf2, size_t Count)
{
	while ((0 != Count) && (*pBuf1 == *pBuf2))
	{
		++ pBuf1;
		++ pBuf2
		-- Count;
	}

	return (0 == (*pBuf1 - *pBuf2));
}

void* MemCpy(void* pDest, const void* pSrc, size_t Count)
{
	while (0 != Count)
	{
		*pDest = *pSrc;
		++ pDest;
		++ pSrc;
		-- Count;
	}

	return pDest;
}
*/

namespace KlayGE
{
	namespace MemoryLib
	{
		// 把dest指向的大小为count的内存填充为c
		/////////////////////////////////////////////////////////////////////
		void* Set(void* dest, int c, size_t count)
		{
			return memset(dest, c, count);
		}

		// 把buf1和buf2的内容比较，相同返回true
		/////////////////////////////////////////////////////////////////////
		bool Compare(const void* dest, const void* src, size_t count)
		{
			return (0 == memcmp(dest, src, count));
		}

		// 把src指向的大小为count的内存拷贝到dest
		//////////////////////////////////////////////////////////////////////////
		void* Copy(void* dest, const void* src, size_t count)
		{
			return memcpy(dest, src, count);
		}
	}
};
