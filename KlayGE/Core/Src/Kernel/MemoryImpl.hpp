// MemoryImpl.hpp
// KlayGE 内存函数库实现 头文件
// Ver 1.3.8.1
// 版权所有(C) 龚敏敏, 2002
// Homepage: http://www.enginedev.com
//
// 1.3.8.1
// 增加了正向和逆向拷贝函数 (2002.12.5)
//
// 最后修改: 2002.9.20
///////////////////////////////////////////////////////////////////////////////

#ifndef _MEMORYIMPL_HPP
#define _MEMORYIMPL_HPP

namespace KlayGE
{
	namespace KlayGEImpl
	{
		// 内存函数库，使用x86指令集
		///////////////////////////////////////////////////////////////////////////////
		class MemStandardLib : public MemoryLib
		{
		public:
			virtual ~MemStandardLib()
				{ }

			virtual void* Set(void* dest, int c, size_t count) const;
			virtual bool  Cmp(const void* buf1, const void* buf2, size_t count) const;
			virtual void* Cpy(void* dest, const void* src, size_t count) const;
		};

		// 内存函数库，使用MMX指令集
		/////////////////////////////////////////////////////////////////////////////////
		class MemMMXLib : public MemStandardLib
		{
		public:
			virtual ~MemMMXLib()
				{ }

			virtual void* Set(void* dest, int c, size_t count) const;
			virtual bool  Cmp(const void* buf1, const void* buf2, size_t count) const;
			virtual void* Cpy(void* dest, const void* src, size_t count) const;

		private:
			void Cpy64(void* dst, const void* src) const;
		};

		// 内存函数库，使用3DNowEx指令集
		/////////////////////////////////////////////////////////////////////////////////
		class Mem3DNowExLib : public MemMMXLib
		{
		public:
			virtual ~Mem3DNowExLib()
				{ }

			virtual void* Set(void* dest, int c, size_t count) const;
			virtual bool  Cmp(const void* buf1, const void* buf2, size_t count) const;
			virtual void* Cpy(void* dest, const void* src, size_t count) const;

		private:
			void Cpy64(void* dst, const void* src) const;
		};
	}
}

#endif		// _MEMORYIMPL_HPP
