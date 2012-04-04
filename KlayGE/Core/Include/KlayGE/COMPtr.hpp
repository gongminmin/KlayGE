// COMPtr.hpp
// KlayGE 建立COM智能指针 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// 初次建立 (2006.7.20)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _COMPTR_HPP
#define _COMPTR_HPP

#pragma once

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_PLATFORM_WIN32
	#ifndef KLAYGE_CPU_ARM
		#ifndef BOOST_MEM_FN_ENABLE_STDCALL
			#define BOOST_MEM_FN_ENABLE_STDCALL
		#endif
	#endif
#endif
#include <boost/mem_fn.hpp>

namespace KlayGE
{
	// 得到COM对象的智能指针
	template <typename T>
	inline boost::shared_ptr<T>
	MakeCOMPtr(T* p)
	{
		return p ? boost::shared_ptr<T>(p, boost::mem_fn(&T::Release)) : boost::shared_ptr<T>();
	}
}

#endif		// _COMPTR_HPP
