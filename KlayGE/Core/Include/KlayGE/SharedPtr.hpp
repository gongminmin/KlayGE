// SharedPtr.hpp
// KlayGE 共享智能指针 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SHAREDPTR_HPP
#define _SHAREDPTR_HPP

#include <KlayGE/ResPtr.hpp>

namespace KlayGE
{
	// 引用计数指针
	/////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class SharedPtr : public ResPtr<T>
	{
	public:
		SharedPtr()
			: ResPtr<T>()
			{ }
		explicit SharedPtr(const stored_type& p)
			: ResPtr<T>(p)
			{ }
		SharedPtr(const SharedPtr& rhs)
			: ResPtr<T>(rhs)
			{ }
		template <typename U>
		SharedPtr(const SharedPtr<U>& rhs)
			: ResPtr<T>(rhs)
			{ }
	};
}

#endif			// _SHAREDPTR_HPP
