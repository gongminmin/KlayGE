// SharePtr.hpp
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

#ifndef _SHAREPTR_HPP
#define _SHAREPTR_HPP

#include <KlayGE/ResPtr.hpp>

namespace KlayGE
{
	// 引用计数指针
	/////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class SharePtr : public ResPtr<T>
	{
	public:
		SharePtr()
			: ResPtr<T>()
			{ }
		explicit SharePtr(const stored_type& p)
			: ResPtr<T>(p)
			{ }
		SharePtr(const SharePtr& rhs)
			: ResPtr<T>(rhs)
			{ }
		template <typename U>
		SharePtr(const SharePtr<U>& rhs)
			: ResPtr<T>(rhs)
			{ }
	};
}

#endif			// _SHAREPTR_HPP