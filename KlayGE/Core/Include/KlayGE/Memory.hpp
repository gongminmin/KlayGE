// Memory.hpp
// KlayGE 内存函数库 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2001--2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.1.20)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#ifndef _MEMORY_HPP
#define _MEMORY_HPP

#pragma comment(lib, "KlayGE_Core.lib")

// 优化的内存函数
/////////////////////////////////////////////////////////////////////
namespace KlayGE
{
	namespace MemoryLib
	{
		void* Set(void* dest, int c, size_t count);
		bool Compare(const void* buf1, const void* buf2, size_t count);
		void* Copy(void* dest, const void* src, size_t count);

		inline void* Zero(void* dest, size_t count)
			{ return Set(dest, 0, count); }
	};
}

#endif			// _MEMORY_HPP
