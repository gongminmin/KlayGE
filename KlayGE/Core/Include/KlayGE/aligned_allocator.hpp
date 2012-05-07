// aligned_allocator.hpp
// KlayGE 地址对齐的分配器 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.5.0
// 初次建立 (2006.12.23)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _ALIGNED_ALLOCATOR_HPP
#define _ALIGNED_ALLOCATOR_HPP

#pragma once

#include <limits>
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>

namespace KlayGE
{
	template <typename T, int alignment>
	class aligned_allocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		static const int alignment_size = alignment;

		BOOST_STATIC_ASSERT(0 == (alignment & (alignment - 1)));
		BOOST_STATIC_ASSERT(alignment <= 65536);

		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, alignment> other;
		};

		pointer address(reference val) const
		{
			return &val;
		}

		const_pointer address(const_reference val) const
		{
			return &val;
		}

		aligned_allocator() throw()
		{
		}

		aligned_allocator(const aligned_allocator<T, alignment>&) throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator(const aligned_allocator<U, alignment2>&) throw()
		{
		}

		~aligned_allocator() throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator<T, alignment>& operator=(const aligned_allocator<U, alignment2>&)
		{
			return *this;
		}

		void deallocate(pointer p, size_type)
		{
			uint16_t* p16 = reinterpret_cast<uint16_t*>(p);
			uint8_t* org_p = reinterpret_cast<uint8_t*>(p16) - p16[-1];
			free(org_p);
		}

		pointer allocate(size_type count)
		{
			uint8_t* p = static_cast<uint8_t*>(malloc(count * sizeof (T) + 2 + (alignment - 1)));
			uint8_t* new_p = reinterpret_cast<uint8_t*>((reinterpret_cast<size_t>(p) + 2 + (alignment - 1)) & (-static_cast<int32_t>(alignment)));
			reinterpret_cast<uint16_t*>(new_p)[-1] = static_cast<uint16_t>(new_p - p);

			return reinterpret_cast<pointer>(new_p);
		}

		pointer allocate(size_type count, const void* hint)
		{
			pointer* p = this->allocate(count);
			memcpy(p, hint, count * sizeof(T));
			this->deallocate(hint);
			return p;
		}

		void construct(pointer p, const T& val)
		{
			void* vp = p;
			::new (vp) T(val);
		}

		void destroy(pointer p)
		{
			this->destroy_t<boost::has_trivial_destructor<boost::remove_pointer<pointer>::type>::value>(p);
		}

		template <bool trivial>
		void destroy_t(pointer p);

		template <>
		void destroy_t<true>(pointer /*p*/)
		{
		}

		template <>
		void destroy_t<false>(pointer p)
		{
			p->~T();
		}

		size_type max_size() const throw()
		{
			return std::numeric_limits<size_t>::max() / sizeof(T);
		}
	};

	template <typename T, int alignment1, typename U, int alignment2>
	inline bool operator==(const aligned_allocator<T, alignment1>&, const aligned_allocator<U, alignment2>&) throw()
	{
		return true;
	}

	template <typename T, int alignment1, typename U, int alignment2>
	inline bool operator!=(const aligned_allocator<T, alignment1>&, const aligned_allocator<U, alignment2>&) throw()
	{
		return false;
	}


	template <int alignment>
	class aligned_allocator<void, alignment>
	{
	public:
		typedef void* pointer;
		typedef const void* const_pointer;
		typedef void value_type;

		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, alignment> other;
		};

		aligned_allocator() throw()
		{
		}

		aligned_allocator(const aligned_allocator<void, alignment>&) throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator(const aligned_allocator<U, alignment2>&) throw()
		{
		}

		template <typename U, int alignment2>
		aligned_allocator<void, alignment>& operator=(const aligned_allocator<U, alignment2>&)
		{
			return *this;
		}
	};
}

#endif		// _ALIGNED_ALLOCATOR_HPP
