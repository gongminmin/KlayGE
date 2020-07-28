/**
 * @file com_ptr.hpp
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

#ifndef KFL_COM_PTR_HPP
#define KFL_COM_PTR_HPP

#pragma once

#include <KFL/ErrorHandling.hpp>
#include <boost/assert.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <guiddef.h>
#endif

namespace KlayGE
{
	template <typename T>
	class com_ptr
	{
		template <typename U>
		friend class com_ptr;

	public:
		using element_type = std::remove_extent_t<T>;

	public:
		com_ptr() noexcept = default;

		com_ptr(std::nullptr_t) noexcept
		{
		}

		com_ptr(T* ptr, bool add_ref = true) noexcept : ptr_(ptr)
		{
			if (add_ref)
			{
				this->internal_add_ref();
			}
		}

		template <typename U>
		com_ptr(U* ptr, bool add_ref = true) noexcept : ptr_(ptr)
		{
			if (add_ref)
			{
				this->internal_add_ref();
			}
		}

		com_ptr(com_ptr const& rhs) noexcept : com_ptr(rhs.ptr_, true)
		{
		}

		template <typename U>
		com_ptr(com_ptr<U> const& rhs) noexcept : com_ptr(rhs.ptr_, true)
		{
		}

		com_ptr(com_ptr&& rhs) noexcept : ptr_(std::exchange(rhs.ptr_, {}))
		{
		}

		template <typename U>
		com_ptr(com_ptr<U>&& rhs) noexcept : ptr_(std::exchange(rhs.ptr_, {}))
		{
		}

		~com_ptr() noexcept
		{
			this->internal_release();
		}

		com_ptr& operator=(com_ptr const& rhs) noexcept
		{
			if (ptr_ != rhs.ptr_)
			{
				this->internal_release();
				ptr_ = rhs.ptr_;
				this->internal_add_ref();
			}
			return *this;
		}

		template <typename U>
		com_ptr& operator=(com_ptr<U> const& rhs) noexcept
		{
			if (ptr_ != rhs.ptr_)
			{
				this->internal_release();
				ptr_ = rhs.ptr_;
				this->internal_add_ref();
			}
			return *this;
		}

		com_ptr& operator=(com_ptr&& rhs) noexcept
		{
			if (ptr_ != rhs.ptr_)
			{
				this->internal_release();
				ptr_ = std::exchange(rhs.ptr_, {});
			}
			return *this;
		}

		template <typename U>
		com_ptr& operator=(com_ptr<U>&& rhs) noexcept
		{
			this->internal_release();
			ptr_ = std::exchange(rhs.ptr_, {});
			return *this;
		}

		void swap(com_ptr& rhs) noexcept
		{
			std::swap(ptr_, rhs.ptr_);
		}

		explicit operator bool() const noexcept
		{
			return (ptr_ != nullptr);
		}

		T& operator*() const noexcept
		{
			return *ptr_;
		}

		T* operator->() const noexcept
		{
			return ptr_;
		}

		T* get() const noexcept
		{
			return ptr_;
		}

		T** put() noexcept
		{
			BOOST_ASSERT(ptr_ == nullptr);
			return &ptr_;
		}

		void** put_void() noexcept
		{
			return reinterpret_cast<void**>(this->put());
		}

		T** release_and_put() noexcept
		{
			this->reset();
			return this->put();
		}

		void** release_and_put_void() noexcept
		{
			return reinterpret_cast<void**>(this->release_and_put());
		}

		T* detach() noexcept
		{
			return std::exchange(ptr_, {});
		}

		void reset() noexcept
		{
			this->internal_release();
		}

		void reset(T* rhs, bool add_ref = true) noexcept
		{
			this->reset();
			ptr_ = rhs;
			if (add_ref)
			{
				this->internal_add_ref();
			}
		}

#ifdef KLAYGE_PLATFORM_WINDOWS
		template <typename U>
		com_ptr<U> try_as(GUID const& riid) const noexcept
		{
#ifdef KLAYGE_COMPILER_MSVC
			BOOST_ASSERT(riid == __uuidof(U));
#endif

			com_ptr<U> ret;
			ptr_->QueryInterface(riid, ret.put_void());
			return ret;
		}

		template <typename U>
		bool try_as(GUID const& riid, com_ptr<U>& to) const noexcept
		{
			to = this->try_as<U>(riid);
			return static_cast<bool>(to);
		}

		template <typename U>
		com_ptr<U> as(GUID const& riid) const
		{
#ifdef KLAYGE_COMPILER_MSVC
			BOOST_ASSERT(riid == __uuidof(U));
#endif

			com_ptr<U> ret;
			TIFHR(ptr_->QueryInterface(riid, ret.put_void()));
			return ret;
		}

		template <typename U>
		void as(GUID const& riid, com_ptr<U>& to) const
		{
			to = this->as<U>(riid);
		}
#endif

	private:
		void internal_add_ref() noexcept
		{
			if (ptr_ != nullptr)
			{
				ptr_->AddRef();
			}
		}

		void internal_release() noexcept
		{
			if (ptr_ != nullptr)
			{
				std::exchange(ptr_, {})->Release();
			}
		}

	private:
		T* ptr_ = nullptr;
	};

	template <typename T, typename U>
	bool operator==(com_ptr<T> const& lhs, com_ptr<U> const& rhs) noexcept
	{
		return lhs.get() == rhs.get();
	}

	template <typename T>
	bool operator==(com_ptr<T> const& lhs, std::nullptr_t rhs) noexcept
	{
		KFL_UNUSED(rhs);
		return lhs.get() == nullptr;
	}

	template <typename T>
	bool operator==(std::nullptr_t lhs, com_ptr<T> const& rhs) noexcept
	{
		return rhs == lhs;
	}

	template <typename T, typename U>
	bool operator!=(com_ptr<T> const& lhs, com_ptr<U> const& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <typename T>
	bool operator!=(com_ptr<T> const& lhs, std::nullptr_t rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <typename T>
	bool operator!=(std::nullptr_t lhs, com_ptr<T> const& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <typename T, typename U>
	bool operator<(com_ptr<T> const& lhs, com_ptr<U> const& rhs) noexcept
	{
		return lhs.get() < rhs.get();
	}

	template <typename T>
	bool operator<(com_ptr<T> const& lhs, std::nullptr_t rhs) noexcept
	{
		KFL_UNUSED(rhs);
		return lhs.get() < nullptr;
	}

	template <typename T>
	bool operator<(std::nullptr_t lhs, com_ptr<T> const& rhs) noexcept
	{
		return rhs > lhs;
	}

	template <typename T, typename U>
	bool operator<=(com_ptr<T> const& lhs, com_ptr<U> const& rhs) noexcept
	{
		return !(rhs < lhs);
	}

	template <typename T>
	bool operator<=(com_ptr<T> const& lhs, std::nullptr_t rhs) noexcept
	{
		return !(rhs < lhs);
	}

	template <typename T>
	bool operator<=(std::nullptr_t lhs, com_ptr<T> const& rhs) noexcept
	{
		return !(rhs < lhs);
	}

	template <typename T, typename U>
	bool operator>(com_ptr<T> const& lhs, com_ptr<U> const& rhs) noexcept
	{
		return rhs < lhs;
	}

	template <typename T>
	bool operator>(com_ptr<T> const& lhs, std::nullptr_t rhs) noexcept
	{
		return rhs < lhs;
	}

	template <typename T>
	bool operator>(std::nullptr_t lhs, com_ptr<T> const& rhs) noexcept
	{
		return rhs < lhs;
	}

	template <typename T, typename U>
	bool operator>=(com_ptr<T> const& lhs, com_ptr<U> const& rhs) noexcept
	{
		return !(lhs < rhs);
	}

	template <typename T>
	bool operator>=(com_ptr<T> const& lhs, std::nullptr_t rhs) noexcept
	{
		return !(lhs < rhs);
	}

	template <typename T>
	bool operator>=(std::nullptr_t lhs, com_ptr<T> const& rhs) noexcept
	{
		return !(lhs < rhs);
	}
} // namespace KlayGE

namespace std
{
	template <typename T>
	void swap(KlayGE::com_ptr<T>& lhs, KlayGE::com_ptr<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	template <typename T>
	struct hash<KlayGE::com_ptr<T>>
	{
		using argument_type = KlayGE::com_ptr<T>;
		using result_type = std::size_t;

		result_type operator()(argument_type const& p) const noexcept
		{
			return std::hash<typename argument_type::element_type*>()(p.get());
		}
	};
} // namespace std

#endif // KFL_COM_PTR_HPP
