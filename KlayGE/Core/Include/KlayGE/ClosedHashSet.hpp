// ClosedHashSet.hpp
// KlayGE 封闭式hash set 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2007.10.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _CLOSEDHASHSET_HPP
#define _CLOSEDHASHSET_HPP

#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <KlayGE/Util.hpp>
#include <KlayGE/ClosedHashTable.hpp>

namespace KlayGE
{
	template <typename Value, typename HashFunc, typename EqualKey, typename Allocator = std::allocator<Value> >
	class closed_hash_set
	{
		typedef closed_hash_table<Value, Value, HashFunc, identity<Value>, EqualKey, Allocator> ht_type;

	public:
		typedef typename ht_type::key_type key_type;
		typedef typename ht_type::value_type value_type;
		typedef typename ht_type::hasher hasher;
		typedef typename ht_type::key_equal key_equal;

		typedef typename ht_type::size_type size_type;
		typedef typename ht_type::difference_type difference_type;
		typedef typename ht_type::pointer pointer;
		typedef typename ht_type::const_pointer const_pointer;
		typedef typename ht_type::reference reference;
		typedef typename ht_type::const_reference const_reference;

		typedef typename ht_type::iterator iterator;
		typedef typename ht_type::const_iterator const_iterator;

	public:
		explicit closed_hash_set(size_type n = 0, hasher const & hf = hasher(),
				key_equal const & eql = key_equal())
			: rep_(n, hf, eql)
		{
		}

		template <class InputIterator>
		closed_hash_set(InputIterator f, InputIterator l,
				size_type n = 0, hasher const & hf = hasher(),
				key_equal const & eql = key_equal())
		{
			rep_.insert(f, l);
		}

		iterator begin()
		{
			return rep_.begin();
		}
		iterator end()
		{
			return rep_.end();
		}
		const_iterator begin() const
		{
			return rep_.begin();
		}
		const_iterator end() const
		{
			return rep_.end();
		}

		hasher hash_function() const
		{
			return rep_.hash_function();
		}
		key_equal key_eq() const
		{
			return rep_.key_eq();
		}

		void clear()
		{
			rep_.clear();
		}
		void clear_no_resize()
		{
			rep_.clear_no_resize();
		}
		void swap(closed_hash_set& hs)
		{
			rep_.swap(hs.rep_);
		}

		size_type size() const
		{
			return rep_.size();
		}
		size_type max_size() const
		{
			return rep_.max_size();
		}
		bool empty() const
		{
			return rep_.empty();
		}
		size_type bucket_count() const
		{
			return rep_.bucket_count();
		}
		size_type max_bucket_count() const
		{
			return rep_.max_bucket_count();
		}

		void resize(size_type hint)
		{
			rep_.resize(hint);
		}

		iterator find(key_type const & key)
		{
			return rep_.find(key);
		}
		const_iterator find(key_type const & key) const
		{
			return rep_.find(key);
		}

		size_type count(key_type const & key) const
		{
			return rep_.count(key);
		}

		std::pair<iterator, iterator> equal_range(key_type const & key) const
		{
			return rep_.equal_range(key);
		}

		std::pair<iterator, bool> insert(value_type const & obj)
		{
			std::pair<typename ht_type::iterator, bool> p = rep_.insert(obj);
			return std::pair<iterator, bool>(p.first, p.second);
		}
		template <typename InputIterator>
		void insert(InputIterator f, InputIterator l)
		{
			rep_.insert(f, l);
		}
		void insert(const_iterator f, const_iterator l)
		{
			rep_.insert(f, l);
		}
		iterator insert(iterator, value_type const & obj)
		{
			return this->insert(obj).first;
		}

		size_type erase(key_type const & key)
		{
			return rep_.erase(key);
		}
		iterator erase(iterator it)
		{
			return rep_.erase(it);
		}
		void erase(iterator f, iterator l)
		{
			rep_.erase(f, l);
		}

		friend bool operator==(closed_hash_set const & lhs, closed_hash_set const & rhs)
		{
			return lhs.rep_ == rhs.rep_;
		}
		friend bool operator!=(closed_hash_set const & lhs, closed_hash_set const & rhs)
		{
			return lhs.rep_ != rhs.rep_;
		}

	private:
		ht_type rep_;
	};
}

namespace std
{
	template <typename Val, typename HashFunc, typename EqualKey, typename Allocator>
	inline void
	swap(KlayGE::closed_hash_set<Val, HashFunc, EqualKey, Allocator>& lhs,
		KlayGE::closed_hash_set<Val, HashFunc, EqualKey, Allocator>& rhs)
	{
		lhs.swap(rhs);
	}
}

#endif		// _CLOSEDHASHSET_HPP
