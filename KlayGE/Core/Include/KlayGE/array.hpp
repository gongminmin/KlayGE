// array.hpp
// KlayGE 数组容器模板 头文件
// Ver 1.4.8.3
// 版权所有(C) 龚敏敏, 2002--2003
// Homepage: http://www.enginedev.com
//
// 1.2.8.10
// 初次建立 (2002.10.26)
//
// 1.4.8.3
// 增加了可调下限 (2003.3.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _ARRAY_HPP
#define _ARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <iterator>
#include <algorithm>

namespace KlayGE
{
	template <typename T, std::size_t N, int lowerbound = 0>
	class array
	{
	private:
		T _elems[N];    // 固定长度的数组

	public:
		// 类型定义
		typedef T&										reference;
		typedef const T&								const_reference;
		typedef T*										iterator;
		typedef const T*								const_iterator;
		typedef size_t									size_type;
		typedef ptrdiff_t								difference_type;
		typedef T										value_type;
		typedef T*										pointer;
		typedef const T*								const_pointer;
		typedef std::reverse_iterator<iterator>			reverse_iterator;
		typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;

		// 构造/拷贝
		array()
			{ }
		explicit array(const T& value)
			{ this->assign(value); }
		template <typename InputIterator>
		array(InputIterator first, InputIterator last)
			{ this->assign(first, last); }
		array(const array<T, N>& x)
			{ this->assign(x.begin(), x.end()); }
		template <typename U>
		array<T, N>& operator=(const array<U, N>& rhs)
		{
			std::copy(rhs.begin(), rhs.end(), this->begin());
			return *this;
		}
		template <typename InputIterator>
		void assign(InputIterator first, InputIterator last)
			{ std::copy(first, last, this->begin()); }
		void assign(const T& value)
			{ std::fill(this->begin(), this->end(), value); }

		// 迭代器
		iterator begin()
			{ return _elems; }
		const_iterator begin() const
			{ return _elems; }
		iterator end()
			{ return _elems + N; }
		const_iterator end() const
			{ return _elems + N; }
		reverse_iterator rbegin()
			{ return reverse_iterator(this->end()); }
		const_reverse_iterator rbegin() const
			{ return const_reverse_iterator(this->end()); }
		reverse_iterator rend()
			{ return reverse_iterator(this->begin()); }
		const_reverse_iterator rend() const
			{ return const_reverse_iterator(this->begin()); }

		// 容量
		static size_type size()
			{ return N; }
		static size_type max_size()
			{ return N; }
		static size_type capacity()
			{ return N; }
		static bool empty()
			{ return (0 == N); }

		// 元素访问
		reference operator[](size_type n)
			{ return _elems[n - lowerbound]; }
		const_reference operator[](size_type n) const
			{ return _elems[n - lowerbound]; }
		reference at(size_type n)
		{
			this->rangecheck(n);
			return _elems[n - lowerbound];
		}
		const_reference at(size_type n) const
		{
			this->rangecheck(n);
			return _elems[n - lowerbound];
		}
		reference front()
			{ return _elems[0]; }
		const_reference front() const
			{ return _elems[0]; }
		reference back()
			{ return _elems[N - 1]; }
		const_reference back() const
			{ return _elems[N - 1]; }

		// 直接访问数据
		const T* data() const
			{ return _elems; }

		// 变动函数
		void swap(array<T, N>& rhs)
			{ std::swap_ranges(this->begin(), this->end(), rhs.begin()); }

	private:
		// 范围检查
		void rangecheck(size_type n)
		{
			if ((n >= this->size() - lowerbound) || (n < lowerbound))
			{
				throw std::out_of_range("BettyGE::array");
			}
		}

	};

	// 比较
	template <typename T, std::size_t N>
	bool operator==(const array<T, N>& x, const array<T, N>& y)
	{
		return std::equal(x.begin(), x.end(), y.begin());
	}
	template <typename T, std::size_t N>
	bool operator<(const array<T, N>& x, const array<T, N>& y)
	{
		return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
	}
	template <typename T, std::size_t N>
	bool operator!=(const array<T, N>& x, const array<T, N>& y)
	{
		return !(x == y);
	}
	template <typename T, std::size_t N>
	bool operator>(const array<T, N>& lhs, const array<T, N>& y)
	{
		return y < x;
	}
	template <typename T, std::size_t N>
	bool operator>=(const array<T, N>& x, const array<T, N>& y)
	{
		return !(x < y);
	}
	template <typename T, std::size_t N>
	bool operator<=(const array<T, N>& x, const array<T, N>& y)
	{
		return !(y < x);
	}

	// 特殊算法
	template <typename T, std::size_t N>
	inline void swap(array<T, N>& x, array<T, N>& y)
	{
		x.swap(y);
	}
}

#endif			// _ARRAY_HPP
