// COMPtr.hpp
// KlayGE COM智能指针 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.1.2
// 改用Boost::shared_ptr实现 (2004.8.11)
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _COMPTR_HPP
#define _COMPTR_HPP

#define NOMINMAX
#include <windows.h>

#include <boost/smart_ptr.hpp>
#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

namespace KlayGE
{
	// COM指针
	/////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class COMPtr
	{
	public:
		typedef T element_type;

		COMPtr()
			{ }
		template<class Y>
		explicit COMPtr(Y* p)
			: p_(p, boost::mem_fn(&Y::Release))
			{ }

		COMPtr(const COMPtr& r)
			: p_(r.p_)
			{ }
		template <typename Y>
		COMPtr(const COMPtr<Y>& r)
			: p_(r.p_)
			{ }
		template <typename Y>
		explicit COMPtr(const boost::weak_ptr<Y>& r)
			: p_(r)
			{ }

		COMPtr& operator=(const COMPtr& r)
		{
			p_ = r.p_;
			return *this;
		}
		template <typename Y>
		COMPtr& operator=(const COMPtr<Y>& r)
		{
			p_ = r.p_;
			return *this;
		}

		void reset()
		{
			p_.reset();
		}
		template <typename Y>
		void reset(Y* p)
		{
			p_.reset(p, boost::mem_fn(&Y::Release));
		}

		T& operator*() const
			{ return p_.operator*(); }
		T* operator->() const
			{ return p_.operator->(); }
		T* get() const
			{ return p_.get(); }

		bool unique() const
			{ return p_.unique(); }
		long use_count() const
			{ return p_.use_count(); }

		operator bool() const
			{ return bool(p_); }

		void swap(COMPtr& rhs)
			{ return p_.swap(rhs.p_); }


		template <REFIID iid>
		HRESULT CoCreateInstance(REFCLSID rclsid, U32 clsContext = CLSCTX_ALL, IUnknown* pUnkOuter = NULL)
		{
			this->reset();

			T* ref;
			HRESULT hr = ::CoCreateInstance(rclsid, pUnkOuter, clsContext, iid, reinterpret_cast<void**>(&ref));
			this->reset(ref);

			return hr;
		}

		template <REFIID iid, typename U>
		HRESULT QueryInterface(COMPtr<U>& u) const
		{
			U* p(0);

			HRESULT hr(this->get()->QueryInterface(iid, reinterpret_cast<void**>(&p)));
			if (SUCCEEDED(hr))
			{
				u = COMPtr<U>(p);
			}

			return hr;
		}

	private:
		boost::shared_ptr<T> p_;
	};
}

#endif			// _COMPTR_HPP