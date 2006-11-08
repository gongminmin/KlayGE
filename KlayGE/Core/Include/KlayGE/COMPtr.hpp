// COMPtr.hpp
// KlayGE 建立COM智能指针 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.20)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _COMPTR_HPP
#define _COMPTR_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <boost/smart_ptr.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_PLATFORM_WIN32
#define BOOST_MEM_FN_ENABLE_STDCALL
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
		return boost::shared_ptr<T>(p, boost::mem_fn(&T::Release));
	}
}

#endif		// _COMPTR_HPP
