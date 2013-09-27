/**
 * @file AlignedAllocator.hpp
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

#ifndef _KFL_ALIGNEDALLOCATOR_HPP
#define _KFL_ALIGNEDALLOCATOR_HPP

#pragma once

#include <limits>

namespace KlayGE
{
	template <typename pointer, bool trivial>
	class destroy_t
	{
	public:
		void operator()(pointer p)
		{
			p->~T();
		}
	};
	
	template <typename pointer>
	class destroy_t<pointer, true>
	{
	public:
		void operator()(pointer /*p*/)
		{
		}
	};

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

		KLAYGE_STATIC_ASSERT(0 == (alignment & (alignment - 1)));
		KLAYGE_STATIC_ASSERT(alignment <= 65536);

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
			destroy_t<pointer, is_trivially_destructible<value_type>::value>()(p);
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

#endif		// _KFL_ALIGNEDALLOCATOR_HPP
