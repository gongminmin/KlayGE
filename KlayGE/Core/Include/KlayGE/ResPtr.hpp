// ResPtr.hpp
// KlayGE 资源智能指针 头文件
// Ver 2.0.1
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 修改记录
//
// 2.0.1
// 初次建立 (2003.10.11)
/////////////////////////////////////////////////////////////////////////////////

#ifndef _RESPTR_HPP
#define _RESPTR_HPP

#include <KlayGE/alloc.hpp>

namespace KlayGE
{
	// 默认存储策略
	/////////////////////////////////////////////////////////////////////////////////
	template <class T>
    class DefaultSP
    {
    protected:
        typedef T					value_type;
		typedef value_type*			stored_type;

		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;

		typedef value_type&			reference;
		typedef const value_type&	const_reference;

    public:
        DefaultSP()
			: pointee_(Default()) 
			{ }

        // The storage policy doesn't initialize the stored pointer 
        //     which will be initialized by the OwnershipPolicy's Clone fn
        DefaultSP(const DefaultSP&)
			{ }

        template <class U>
        DefaultSP(const DefaultSP<U>&) 
			{ }

        DefaultSP(const stored_type& p)
			: pointee_(p)
			{ }

        pointer operator->() const
			{ return pointee_; }

        reference operator*() const
			{ return *pointee_; }

        void Swap(DefaultSP& rhs)
			{ std::swap(pointee_, rhs.pointee_); }

        // Accessors
		pointer Get() const
			{ return pointee_; }

    protected:
        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
			{ delete pointee_; }
     
        // Default value to initialize the pointer
        static stored_type Default()
			{ return 0; }

		pointer& GetRef()
			{ return pointee_; }
    
    private:
        // Data
        stored_type pointee_;
    };

	// 引用计数策略
	/////////////////////////////////////////////////////////////////////////////////
	template <class T>
	class RefCountedOP
	{
		template <typename U>
		friend class RefCountedOP;

	public:
		RefCountedOP() 
		{
			count_	= alloc<unsigned int>().allocate(1);
			*count_	= 1;
		}

		RefCountedOP(const RefCountedOP& rhs) 
			: count_(rhs.count_)
			{ }

		template <typename U>
		RefCountedOP(const RefCountedOP<U>& rhs) 
			: count_(rhs.count_)
			{ }

		T Clone(const T& p)
		{
			++ *count_;
			return p;
		}

		bool Release(const T& p)
		{
			if (p != 0)
			{
				-- *count_;
				if (0 == *count_)
				{
					alloc<unsigned int>().deallocate(count_, 1);
					return true;
				}
			}
			return false;
		}

		void Swap(RefCountedOP& rhs)
			{ std::swap(count_, rhs.count_); }

	private:
		unsigned int* count_;
	};

	// 资源指针
	/////////////////////////////////////////////////////////////////////////////////
	template <typename T,
		template <class> class OwnershipPolicy = RefCountedOP,
		template <class> class StoragePolicy = DefaultSP>
	class ResPtr : public StoragePolicy<T>,
					public OwnershipPolicy<typename StoragePolicy<T>::pointer>
	{
		typedef typename StoragePolicy<T>		SP;
		typedef typename OwnershipPolicy<typename SP::pointer>	OP;

		template <typename U,
			template <class> class OwnershipPolicy2, template <class> class StoragePolicy2>
		friend class ResPtr;

	public:
		typedef typename SP::value_type		value_type;
		typedef typename SP::stored_type	stored_type;

		typedef typename SP::pointer		pointer;
		typedef typename SP::const_pointer	const_pointer;

		typedef typename SP::reference			reference;
		typedef typename SP::const_reference	const_reference;

		ResPtr()
			: SP(0)
			{ }
		explicit ResPtr(const stored_type& p)
			: SP(p)
			{ }
		ResPtr(const ResPtr& rhs)
			: SP(rhs), OP(rhs)
			{ SP::GetRef() = OP::Clone(rhs.Get()); }

		template <typename U>
		ResPtr(const ResPtr<U, OwnershipPolicy, StoragePolicy>& rhs)
			: SP(rhs), OP(rhs)
			{ SP::GetRef() = OP::Clone(static_cast<typename SP::pointer>(rhs.Get())); }

		~ResPtr()
			{ this->Release(); }

		reference operator*()
			{ return SP::operator*(); }
		reference operator*() const
			{ return SP::operator*(); }
		pointer operator->()
			{ return SP::operator->(); }
		pointer operator->() const
			{ return SP::operator->(); }

		void Swap(ResPtr& rhs)
		{
			SP::Swap(rhs);
			OP::Swap(rhs);
		}

		ResPtr& operator=(const ResPtr& rhs)
		{
			ResPtr temp(rhs);
			this->Swap(temp);
			return *this;
		}
		template <typename U>
		ResPtr& operator=(const ResPtr<U, OwnershipPolicy, StoragePolicy>& rhs)
		{
			ResPtr temp(rhs);
			this->Swap(temp);
			return *this;
		}

		void Release()
		{
			if (OP::Release(SP::Get()))
			{
				SP::Destroy();
			}
			SP::GetRef() = SP::Default();
		}

		// 重载操作符
		operator void*() const
			{ return 0 != SP::Get(); }
		bool operator!() const
			{ return 0 == SP::Get(); }
	};

	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator==(const ResPtr<T, OP, SP>& lhs, U* rhs)
		{ return lhs.Get() == rhs; }
	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator==(U* lhs, const ResPtr<T, OP, SP>& rhs)
		{ return rhs != lhs; }

	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator!=(const ResPtr<T, OP, SP>& lhs, U* rhs)
		{ return !(lhs.Get() == rhs); }
	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator!=(U* lhs, const ResPtr<T, OP, SP>& rhs)
		{ return rhs != lhs; }

	template <typename T, typename U,
		template <class> class OP1, template <class> class SP1,
		template <class> class OP2, template <class> class SP2>
	inline bool
	operator==(const ResPtr<T, OP1, SP1>& lhs, const ResPtr<U, OP2, SP2>& rhs)
		{ return lhs.Get() == rhs.Get(); }
	template <typename T, typename U,
		template <class> class OP1, template <class> class SP1,
		template <class> class OP2, template <class> class SP2>
	inline bool
	operator!=(const ResPtr<T, OP1, SP1>& lhs, const ResPtr<U, OP2, SP2>& rhs)
		{ return !(lhs == rhs); }


	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator<(const ResPtr<T, OP, SP>& lhs, const U* rhs)
		{ return lhs.Get() < rhs; }
	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator<(const T* lhs, const ResPtr<U, OP, SP>& rhs)
		{ return rhs > lhs; }

	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator<=(const ResPtr<T, OP, SP>& lhs, const U* rhs)
		{ return lhs.Get() <= rhs; }
	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator<=(const T* lhs, const ResPtr<U, OP, SP>& rhs)
		{ return lhs <= rhs.Get(); }

	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator>(const ResPtr<T, OP, SP>& lhs, const U* rhs)
		{ return lhs.Get() > rhs; }
	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator>(const T* lhs, const ResPtr<U, OP, SP>& rhs)
		{ return lhs > rhs.Get(); }

	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator>=(const ResPtr<T, OP, SP>& lhs, const U* rhs)
		{ return lhs.Get() >= rhs; }
	template <typename T, typename U,
		template <class> class OP, template <class> class SP>
	inline bool
	operator>=(const T* lhs, const ResPtr<U, OP, SP>& rhs)
		{ return lhs >= rhs.Get(); }

	template <typename T, typename U,
		template <class> class OP1, template <class> class SP1,
		template <class> class OP2, template <class> class SP2>
	inline bool
	operator<(const ResPtr<T, OP1, SP1>& lhs, const ResPtr<U, OP2, SP2>& rhs)
		{ return lhs.Get() < rhs.Get(); }
	template <typename T, typename U,
		template <class> class OP1, template <class> class SP1,
		template <class> class OP2, template <class> class SP2>
	inline bool
	operator<=(const ResPtr<T, OP1, SP1>& lhs, const ResPtr<U, OP2, SP2>& rhs)
		{ return lhs.Get() <= rhs.Get(); }
	template <typename T, typename U,
		template <class> class OP1, template <class> class SP1,
		template <class> class OP2, template <class> class SP2>
	inline bool
	operator>(const ResPtr<T, OP1, SP1>& lhs, const ResPtr<U, OP2, SP2>& rhs)
		{ return lhs.Get() > rhs.Get(); }
	template <typename T, typename U,
		template <class> class OP1, template <class> class SP1,
		template <class> class OP2, template <class> class SP2>
	inline bool
	operator>=(const ResPtr<T, OP1, SP1>& lhs, const ResPtr<U, OP2, SP2>& rhs)
		{ return lhs.Get() >= rhs.Get(); }
}

#endif			// _RESPTR_HPP