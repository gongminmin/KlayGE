// atomic.hpp
// KlayGE 原子类型 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2008.2.23)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _ATOMIC_HPP
#define _ATOMIC_HPP

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace KlayGE
{
	template <typename T>
	class atomic
	{
	public:
		atomic(T const & rhs);

		operator T();
		atomic& operator=(T const & rhs);

		atomic const & operator++();
		atomic const & operator--();
		atomic operator++(int);
		atomic operator--(int);

		atomic& operator+=(T const & rhs);
		atomic& operator-=(T const & rhs);
		atomic& operator&=(T const & rhs);
		atomic& operator|=(T const & rhs);
		atomic& operator^=(T const & rhs);

		void swap(T const & rhs);
		bool compare_swap(T const & old_val, T const & new_val);
	};

#ifdef KLAYGE_PLATFORM_WINDOWS
	template <>
	class atomic<int32_t>
	{
	public:
		atomic()
		{
		}

		explicit atomic(int32_t rhs)
		{
			InterlockedExchange(reinterpret_cast<long*>(&value_), rhs);
		}

		int32_t value() const
		{
			return value_;
		}

		atomic& operator=(int32_t const & rhs)
		{
			InterlockedExchange(reinterpret_cast<long*>(&value_), rhs);
			return *this;
		}
		atomic& operator=(atomic const & rhs)
		{
			InterlockedExchange(reinterpret_cast<long*>(&value_), rhs.value_);
			return *this;
		}

		bool compare_swap(int32_t const & old_val, int32_t const & new_val)
		{
			return old_val == InterlockedCompareExchange(reinterpret_cast<long*>(&value_), new_val, old_val);
		}

		bool operator<(int32_t const & rhs) const
		{
			return value_ < rhs;
		}
		bool operator<(atomic const & rhs) const
		{
			return value_ < rhs.value_;
		}
		bool operator<=(int32_t const & rhs) const
		{
			return value_ <= rhs;
		}
		bool operator<=(atomic const & rhs) const
		{
			return value_ <= rhs.value_;
		}
		bool operator>(int32_t const & rhs) const
		{
			return value_ > rhs;
		}
		bool operator>(atomic const & rhs) const
		{
			return value_ > rhs.value_;
		}
		bool operator>=(int32_t const & rhs) const
		{
			return value_ >= rhs;
		}
		bool operator>=(atomic const & rhs) const
		{
			return value_ >= rhs.value_;
		}

		bool operator==(int32_t const & rhs) const
		{
			return value_ == rhs;
		}
		bool operator==(atomic const & rhs) const
		{
			return value_ == rhs.value_;
		}

		bool operator!=(int32_t const & rhs) const
		{
			return value_ != rhs;
		}
		bool operator!=(atomic const & rhs) const
		{
			return value_ != rhs.value_;
		}

		atomic& operator+=(int32_t const & rhs)
		{
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), rhs);
			return *this;
		}
		atomic& operator+=(atomic const & rhs)
		{
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), rhs.value_);
			return *this;
		}

		atomic& operator-=(int32_t const & rhs)
		{
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), -rhs);
			return *this;
		}

		atomic& operator*=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand * rhs;
			}
			while (comperand != InterlockedCompareExchange(reinterpret_cast<long*>(&value_), exchange, comperand));
			return *this;
		}

		atomic& operator/=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand / rhs;
			}
			while (comperand != InterlockedCompareExchange(reinterpret_cast<long*>(&value_), exchange, comperand));
			return *this;
		}

		atomic& operator%=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand % rhs;
			}
			while (comperand != InterlockedCompareExchange(reinterpret_cast<long*>(&value_), exchange, comperand));
			return *this;
		}

		atomic& operator&=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand & rhs;
			}
			while (comperand != InterlockedCompareExchange(reinterpret_cast<long*>(&value_), exchange, comperand));
			return *this;
		}

		atomic& operator|=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand | rhs;
			}
			while (comperand != InterlockedCompareExchange(reinterpret_cast<long*>(&value_), exchange, comperand));
			return *this;
		}

		atomic& operator^=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand ^ rhs;
			}
			while (comperand != InterlockedCompareExchange(reinterpret_cast<long*>(&value_), exchange, comperand));
			return *this;
		}

		atomic const & operator++()
		{
			InterlockedIncrement(reinterpret_cast<long*>(&value_));
			return *this;
		}

		atomic const & operator--()
		{
			InterlockedDecrement(reinterpret_cast<long*>(&value_));
			return *this;
		}

		atomic operator++(int)
		{
			atomic tmp = *this;
			++ *this;
			return tmp;
		}

		atomic operator--(int)
		{
			atomic tmp = *this;
			-- *this;
			return tmp;
		}

	private:
		int32_t value_;
	};
#endif
}

#endif		// _ATOMIC_HPP
