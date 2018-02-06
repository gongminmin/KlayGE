/**
 * @file ArrayRef.hpp
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

#ifndef _KFL_ARRAYREF_HPP
#define _KFL_ARRAYREF_HPP

#include <KFL/Util.hpp>

#include <vector>

#include <boost/assert.hpp>

namespace KlayGE
{
	// ArrayRef, inspired by LLVM's ArrayRef (http://llvm.org/docs/doxygen/html/ArrayRef_8h_source.html)
	//
	// ArrayRef - Represent a constant reference to an array (0 or more elements
	// consecutively in memory), i.e. a start pointer and a size.  It allows
	// various APIs to take consecutive elements easily and conveniently.
	//
	// This class does not own the underlying data, it is expected to be used in
	// situations where the data resides in some other buffer, whose lifetime
	// extends past that of the ArrayRef. For this reason, it is not in general
	// safe to store an ArrayRef.
	//
	// This is intended to be trivially copyable, so it should be passed by value.
	template <typename T>
	class ArrayRef
	{
	public:
		typedef T const * iterator;
		typedef T const * const_iterator;
		typedef size_t size_type;

		typedef std::reverse_iterator<iterator> reverse_iterator;

	public:
		constexpr ArrayRef()
			: data_(nullptr), size_(0)
		{
		}

		ArrayRef(ArrayRef const & rhs)
			: data_(rhs.data()), size_(rhs.size())
		{
		}

		KFL_IMPLICIT constexpr ArrayRef(T const & t)
			: data_(&t), size_(1)
		{
		}

		constexpr ArrayRef(T const * data, size_t size)
			: data_(data), size_(size)
		{
		}

		constexpr ArrayRef(T const * begin, T const * end)
			: data_(begin), size_(end - begin)
		{
		}

		template <typename A>
		KFL_IMPLICIT constexpr ArrayRef(std::vector<T, A> const & v)
			: data_(v.data()), size_(v.size())
		{
		}

		template <size_t N>
		KFL_IMPLICIT constexpr ArrayRef(T const (&arr)[N])
			: data_(arr), size_(N)
		{
		}

		constexpr ArrayRef(std::initializer_list<T> const & v)
			: data_(v.begin() == v.end() ? nullptr : v.begin()), size_(v.size())
		{
		}

		template <typename U>
		KFL_IMPLICIT constexpr ArrayRef(ArrayRef<U*> const & rhs,
			typename std::enable_if<std::is_convertible<U* const *, T const *>::value>::type* = 0)
			: data_(rhs.data()), size_(rhs.size())
		{
		}

		template <typename U, typename A>
		KFL_IMPLICIT constexpr ArrayRef(std::vector<U*, A> const & v,
			typename std::enable_if<std::is_convertible<U* const *, T const *>::value>::type* = 0)
			: data_(v.data()), size_(v.size())
		{
		}

		constexpr iterator begin() const
		{
			return data_;
		}
		constexpr iterator end() const
		{
			return data_ + size_;
		}

		constexpr reverse_iterator rbegin() const
		{
			return reverse_iterator(this->end());
		}
		constexpr reverse_iterator rend() const
		{
			return reverse_iterator(this->begin());
		}

		constexpr T const * data() const
		{
			return data_;
		}

		constexpr size_t size() const
		{
			return size_;
		}

		constexpr bool empty() const
		{
			return size_ == 0;
		}

		constexpr T const & front() const
		{
			BOOST_ASSERT(!this->empty());
			return data_[0];
		}

		constexpr T const & back() const
		{
			BOOST_ASSERT(!this->empty());
			return data_[size_ - 1];
		}

		template <typename Alloc>
		ArrayRef<T> Copy(Alloc& alloc)
		{
			T* buff = alloc.template allocate<T>(size_);
			std::uninitialized_copy(this->begin(), this->end(), buff);
			return ArrayRef<T>(buff, size_);
		}

		constexpr ArrayRef<T> Slice(uint32_t n) const
		{
			BOOST_ASSERT_MSG(n <= this->size(), "Invalid specifier");
			return ArrayRef<T>(this->data() + n, this->size() - n);
		}

		constexpr ArrayRef<T> Slice(uint32_t n, uint32_t m) const
		{
			BOOST_ASSERT_MSG(n + m <= this->size(), "Invalid specifier");
			return ArrayRef<T>(this->data() + n, m);
		}

		constexpr ArrayRef<T> DropBack(uint32_t n = 1) const
		{
			BOOST_ASSERT_MSG(this->size() >= n, "Dropping more elements than exist");
			return this->Slice(0, this->Size() - n);
		}

		constexpr T const & operator[](size_t index) const
		{
			BOOST_ASSERT_MSG(index < size_, "Invalid index!");
			return data_[index];
		}

		constexpr std::vector<T> ToVector() const
		{
			return std::vector<T>(data_, data_ + size_);
		}

	private:
		T const * data_;
		size_type size_;
	};

	template <typename T>
	inline bool operator==(ArrayRef<T> lhs, ArrayRef<T> rhs)
	{
		if (lhs.size() != rhs.size())
		{
			return false;
		}
		return std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template <typename T>
	inline bool operator!=(ArrayRef<T> lhs, ArrayRef<T> rhs)
	{
		return !(lhs == rhs);
	}
}

#endif		// _KFL_ARRAYREF_HPP
