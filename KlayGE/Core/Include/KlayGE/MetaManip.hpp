// MetaManip.hpp
// KlayGE 源编程库 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
// 主要来自Loki
//
// 2.0.0
// 初次建立 (2003.8.6)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _METAMANIP_HPP
#define _METAMANIP_HPP

namespace KlayGE
{
	// 编译期断言
	////////////////////////////////////////////////////////////////////////////////
	template <bool T>
	class StaticAssert;

	template <>
	class StaticAssert<true>
	{
	};

	// 把常量转化成唯一的类型
	// 注意: Int2Type<v> 的 v 是一个编译期整数常量
	////////////////////////////////////////////////////////////////////////////////
	template <int v>
	struct Int2Type
	{
		enum { value = v };
	};

	// 根据bool从两种类型种选择一种类型
	// 注意: If<flag, T, U>::Result 的
	// flag 是一个编译期bool常量
	// T 和 U 是类型
	// 如果flag == true, Result是T, 否则是U
	////////////////////////////////////////////////////////////////////////////////
	template <bool flag, typename T, typename U>
	struct If
	{
	private:
		template<bool>
		struct In
		{
			typedef T Result;
		};

		template<>
		struct In<false>
		{
			typedef U Result;
		};

	public:
		typedef typename In<flag>::Result Result;
	};

	// 如果给定的两种类型相同，返回true
	// 注意: SameType<T, U>::value 的
	// T 和 U 是类型
	////////////////////////////////////////////////////////////////////////////////
	template <typename T, typename U>
	struct SameType
	{
	private:
		template<typename>
		struct In 
		{
			enum { value = false };
		};

		template<>
		struct In<T>
		{
			enum { value = true };
		};

	public:
		enum { value = In<U>::value };
	};

	// 判断T是否可以隐式转换为U
	////////////////////////////////////////////////////////////////////////////////
	template <class T, class U>
	class Conversion
	{
		typedef char Small;
		struct Big { char dummy[2]; };
		static Big Test(...);
		static Small Test(U);
		static T MakeT();

	public:
		enum { exists = sizeof(typename Small) == sizeof(Test(MakeT())) };
	};
}

#endif			// _METAMANIP_HPP
