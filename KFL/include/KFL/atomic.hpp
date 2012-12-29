/**
 * @file Atomic.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _KFL_ATOMIC_HPP
#define _KFL_ATOMIC_HPP

#pragma once

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace KlayGE
{
	template <typename T>
	class atomic
	{
	public:
		explicit atomic(T const & rhs);
		atomic(atomic const & rhs);

		T value() const;
		void value(T const & rhs);

		atomic& operator=(T const & rhs);
		atomic& operator=(atomic const & rhs);

		bool operator<(T const & rhs) const;
		bool operator<(atomic const & rhs) const;
		bool operator<=(int32_t const & rhs) const;
		bool operator<=(T const & rhs) const;
		bool operator>(T const & rhs) const;
		bool operator>(atomic const & rhs) const;
		bool operator>=(T const & rhs) const;
		bool operator>=(atomic const & rhs) const;

		bool operator==(T const & rhs) const;
		bool operator==(atomic const & rhs) const;

		bool operator!=(T const & rhs) const;
		bool operator!=(atomic const & rhs) const;

		T const & operator++();
		T const & operator--();
		T operator++(int);
		T operator--(int);

		atomic& operator+=(T const & rhs);
		atomic& operator+=(atomic const & rhs);
		atomic& operator-=(T const & rhs);
		atomic& operator-=(atomic const & rhs);
		atomic& operator&=(T const & rhs);
		atomic& operator&=(atomic const & rhs);
		atomic& operator|=(T const & rhs);
		atomic& operator|=(atomic const & rhs);
		atomic& operator^=(T const & rhs);
		atomic& operator^=(atomic const & rhs);

		bool cas(T const & old_val, T const & new_val);
	};

	template <>
	class atomic<int32_t>
	{
	public:
		atomic()
		{
		}

		explicit atomic(int32_t rhs)
		{
			this->value(rhs);
		}

		int32_t value() const
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			return value_;
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			return __sync_fetch_and_add(&value_, 0);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			return __gnu_cxx::__exchange_and_add(&value_, 0);
#endif
		}

		void value(int32_t const & rhs)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedExchange(reinterpret_cast<long*>(&value_), rhs);
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			value_ = rhs;
			__sync_synchronize();
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			value_ = rhs;
#endif
		}

		atomic& operator=(int32_t const & rhs)
		{
			this->value(rhs);
			return *this;
		}
		atomic& operator=(atomic const & rhs)
		{
			this->value(rhs.value_);
			return *this;
		}

		bool cas(int32_t const & old_val, int32_t const & new_val)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			return old_val == InterlockedCompareExchange(reinterpret_cast<long*>(&value_), new_val, old_val);
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			return __sync_bool_compare_and_swap(&value_, old_val, new_val);
#else
			return old_val == __cmpxchg(&value_, old_val, new_val, sizeof(old_val));
#endif
		}

		bool operator<(int32_t const & rhs) const
		{
			return this->value() < rhs;
		}
		bool operator<(atomic const & rhs) const
		{
			return this->value() < rhs.value();
		}
		bool operator<=(int32_t const & rhs) const
		{
			return this->value() <= rhs;
		}
		bool operator<=(atomic const & rhs) const
		{
			return this->value() <= rhs.value();
		}
		bool operator>(int32_t const & rhs) const
		{
			return this->value() > rhs;
		}
		bool operator>(atomic const & rhs) const
		{
			return this->value() > rhs.value();
		}
		bool operator>=(int32_t const & rhs) const
		{
			return this->value() >= rhs;
		}
		bool operator>=(atomic const & rhs) const
		{
			return this->value() >= rhs.value();
		}

		bool operator==(int32_t const & rhs) const
		{
			return this->value() == rhs;
		}
		bool operator==(atomic const & rhs) const
		{
			return this->value() == rhs.value();
		}

		bool operator!=(int32_t const & rhs) const
		{
			return this->value() != rhs;
		}
		bool operator!=(atomic const & rhs) const
		{
			return this->value() != rhs.value();
		}

		atomic& operator+=(int32_t const & rhs)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), rhs);
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			__sync_add_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator+=(atomic const & rhs)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), rhs.value_);
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			__sync_add_and_fetch(&value_, rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs.value_);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator-=(int32_t const & rhs)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), -rhs);
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			__sync_sub_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator-=(atomic const & rhs)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), -rhs.value_);
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			__sync_add_and_fetch(&value_, -rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs.value_);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
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
			} while (!this->cas(comperand, exchange));
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
			} while (!this->cas(comperand, exchange));
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
			} while (!this->cas(comperand, exchange));
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
			} while (!this->cas(comperand, exchange));
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
			} while (!this->cas(comperand, exchange));
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
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		int32_t const & operator++()
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedIncrement(reinterpret_cast<long*>(&value_));
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			__sync_add_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, 1);
#else
			this->operator+=(1);
#endif
			return value_;
		}

		int32_t const & operator--()
		{
#ifdef KLAYGE_PLATFORM_WINDOWS
			InterlockedDecrement(reinterpret_cast<long*>(&value_));
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
			__sync_sub_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -1);
#else
			this->operator-=(1);
#endif
			return value_;
		}

		int32_t operator++(int)
		{
			long old_val;
			long new_val;
			do
			{
				old_val = value_;
				new_val = old_val + 1;		
			} while (!this->cas(old_val, new_val));
			return old_val;
		}

		int32_t operator--(int)
		{
			long old_val;
			long new_val;
			do
			{
				old_val = value_;
				new_val = old_val - 1;		
			} while (!this->cas(old_val, new_val));
			return old_val;
		}

	private:
#ifdef KLAYGE_PLATFORM_WINDOWS
		mutable int32_t value_;
#elif defined(KLAYGE_COMPILER_GCC) && KLAYGE_COMPILER_VERSION >= 41
		mutable int32_t value_;
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
		mutable _Atomic_word value_;
#else
		mutable int32_t value_;
#endif
	};
}

#endif		// _KFL_ATOMIC_HPP
