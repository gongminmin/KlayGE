// Memory.hpp
// KlayGE 内存函数库 头文件
// Ver 1.4.8.5
// 版权所有(C) 龚敏敏, 2001--2003
// Homepage: http://www.enginedev.com
//
// 1.3.8.3
// 改为虚拟基类 (2003.1.20)
//
// 1.4.8.5
// 增加了MemoryInterface (2003.4.20)
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
	class MemoryLib
	{
	public:
		static MemoryLib* Create(CPUOptimal cpu);

		virtual ~MemoryLib()
			{ }

		virtual void* Set(void* dest, int c, size_t count) const = 0;
		virtual bool  Cmp(const void* buf1, const void* buf2, size_t count) const = 0;
		virtual void* Cpy(void* dest, const void* src, size_t count) const = 0;

		void* Zero(void* dest, size_t count) const
			{ return this->Set(dest, 0, count); }
	};
}

#endif			// _MEMORY_HPP