// MapVector.hpp
// KlayGE 关联向量容器模板 头文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 默认分配器改用boost的 (2004.10.30)
//
// 2.0.2
// 初次建立 (2003.12.11)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _MAPVECTOR_HPP
#define _MAPVECTOR_HPP

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#pragma warning(disable : 4127)
#pragma warning(disable : 4800)
#include <boost/pool/pool_alloc.hpp>

namespace KlayGE
{
	template <typename Key, typename Type, 
				class Traits = std::less<Key>, 
				class Allocator = boost::fast_pool_allocator<std::pair<Key, Type> > >
	class MapVector
	{
		typedef std::vector<std::pair<Key, Type>, Allocator>	ContainerType;

		class CompareType : public Traits
		{
			typedef std::pair<typename Traits::first_argument_type, Type>	Data;
			typedef typename Traits::first_argument_type					first_argument_type;

		public:
			CompareType(Traits const & rhs)
				: Traits(rhs)
				{ }

			bool operator()(first_argument_type const & lhs, first_argument_type const & rhs) const
				{ return Traits::operator()(lhs, rhs); }

			bool operator()(Data const & lhs, Data const & rhs) const
				{ return this->operator()(lhs.first, rhs.first); }

			bool operator()(Data const & lhs, first_argument_type const & rhs) const
				{ return this->operator()(lhs.first, rhs); }

			bool operator()(first_argument_type const & lhs, Data const & rhs) const
				{ return this->operator()(lhs, rhs.first); }
		};

	public:
		typedef Traits			key_compare;
		typedef Key				key_type;
		typedef Type			mapped_type;
		typedef Allocator		allocator_type;

		typedef typename ContainerType::const_iterator			const_iterator;
		typedef typename ContainerType::const_reverse_iterator	const_reverse_iterator;
		typedef typename ContainerType::iterator				iterator;
		typedef typename ContainerType::reverse_iterator		reverse_iterator;

		typedef typename ContainerType::size_type				size_type;
		typedef typename ContainerType::difference_type			difference_type;
		typedef typename ContainerType::value_type				value_type;

		typedef typename Allocator::pointer					pointer;
		typedef typename Allocator::const_pointer			const_pointer;
		typedef typename Allocator::reference				reference;
		typedef typename Allocator::const_reference			const_reference;

		class value_compare
			: public std::binary_function<value_type, value_type, bool>, private key_compare
		{
			friend class MapVector;

		protected:
			value_compare(key_compare pred)
				: key_compare(pred)
				{ }

		public:
			bool operator()(value_type const & lhs, value_type const & rhs) const
				{ return key_compare::operator()(lhs.first, rhs.first); }
		};

	public:
		explicit MapVector(key_compare const & comp = key_compare(), 
								Allocator const & alloc = Allocator())
					: container_(alloc), compare_(comp)
			{ }

		template <class InputIterator>
        MapVector(InputIterator first, InputIterator last, 
						key_compare const & comp = key_compare(), 
						Allocator const & alloc = Allocator())
					: container_(first, last, alloc), compare_(comp)
			{ std::sort(this->begin(), this->end(), compare_); }

		MapVector(MapVector const & rhs)
			: container_(rhs.container_), compare_(rhs.compare_)
			{ }
        
        MapVector& operator=(MapVector const & rhs)
        { 
            MapVector(rhs).swap(*this);
            return *this;
        }

		mapped_type& operator[](key_type const & key)
			{ return this->insert(value_type(key, mapped_type())).first->second; }

		iterator begin()
			{ return container_.begin(); }
		const_iterator begin() const
			{ return container_.begin(); }

		iterator end()
			{ return container_.end(); }
		const_iterator end() const
			{ return container_.end(); }

		void clear()
			{ container_.clear(); }

		bool empty() const
			{ return container_.empty(); }

		size_type max_size() const
			{ return container_.max_size(); }

		size_type size() const
			{ return container_.size(); }

		size_type count(Key const & key) const
			{ return find(key) != end() }

		std::pair<iterator, iterator> equal_range(Key const & key)
			{ return std::equal_range(begin(), end(), key, compare_); }
		std::pair<const_iterator, const_iterator> equal_range(Key const & key) const
			{ return std::equal_range(begin(), end(), key, compare_); }

		iterator erase(iterator where)
			{ container_.erase(where); }
		iterator erase(iterator first, iterator last)
			{ container_.erase(first, last); }
		size_type erase(key_type const & key)
		{
			iterator iter(this->find(key));
			if (iter != this->end())
			{
				this->erase(iter);
				return 1;
			}
			return 0;
		}

		iterator find(Key const & key)
		{
			iterator iter(this->lower_bound(key));
			if (iter != this->end() && compare_(key, iter->first))
			{
				iter = end();
			}
			return iter;
		}
		const_iterator find(Key const & key) const
		{
			const_iterator iter(this->lower_bound(key));
			if (iter != this->end() && compare_(key, iter->first))
			{
				iter = end();
			}
			return iter;
		}

		Allocator get_allocator() const
		{
			return container_.get_allocator();
		}

		std::pair<iterator, bool> insert(value_type const & val)
		{
			bool found(true);
			iterator iter(this->lower_bound(val.first));

			if ((iter == end()) || compare_(val.first, iter->first))
			{
				iter = container_.insert(iter, val);
				found = false;
			}
			return std::make_pair(iter, !found);
		}
		iterator insert(iterator where, value_type const & val)
		{
			if (where != this->end() && compare_(*pos, val)
				&& (where == this->end() - 1
					|| (!compare_(val, where + 1)
						&& compare_(where + 1, val))))
			{
				return container_.insert(where, val);
			}
			return this->insert(val).first;
		}
		template<class InputIterator>
		void insert(InputIterator first, InputIterator last)
		{
			for (; first != last; ++ first)
			{
				this->insert(*first);
			}
		}

		key_compare key_comp() const
			{ return compare_; }

		iterator lower_bound(Key const & key)
			{ return std::lower_bound(this->begin(), this->end(), key, compare_); }
		const_iterator lower_bound(Key const & key) const
			{ return std::lower_bound(this->begin(), this->end(), key, compare_); }

		iterator upper_bound(Key const & key)
			{ return std::upper_bound(this->begin(), this->end(), key, compare_); }
		const_iterator upper_bound(Key const & key) const
			{ return std::upper_bound(this->begin(), this->end(), key, compare_); }

		reverse_iterator rbegin()
			{ return container_.rbegin(); }
		const_reverse_iterator rbegin() const
			{ return container_.rbegin(); }

		reverse_iterator rend()
			{ return container_.rend(); }
		const_reverse_iterator rend() const
			{ return container_.rend(); }

		void swap(MapVector& rhs)
		{
			std::swap(container_, rhs.container_);
			std::swap(compare_, rhs.compare_);
		}

		value_compare value_comp() const
			{ return value_compare(compare_); }

			
		friend bool operator==(MapVector const & lhs, MapVector const & rhs)
			{ return lhs.container_ == rhs.container_; } 

		friend bool operator<(MapVector const & lhs, MapVector const & rhs)
			{ return lhs.container_ < rhs.container_; } 

	private:
		ContainerType	container_;
		CompareType		compare_;
	};

	template <class Key, class Type, class Traits, class Allocator>
	inline void
	swap(MapVector<Key, Type, Traits, Allocator>& lhs, MapVector<Key, Type, Traits, Allocator>& rhs)
	{
		lhs.swap(rhs);
	}

	template <class Key, class Type, class Traits, class Allocator>
	inline bool
	operator!=(MapVector<Key, Type, Traits, Allocator> const & lhs,
					MapVector<Key, Type, Traits, Allocator> const & rhs)
	{
		return !(lhs == rhs);
	}

	template <class Key, class Type, class Traits, class Allocator>
	inline bool
	operator>(MapVector<Key, Type, Traits, Allocator> const & lhs,
					MapVector<Key, Type, Traits, Allocator> const & rhs)
	{
		return rhs < lhs;
	}

	template <class Key, class Type, class Traits, class Allocator>
	inline bool
	operator>=(MapVector<Key, Type, Traits, Allocator> const & lhs,
					MapVector<Key, Type, Traits, Allocator> const & rhs)
	{
		return !(lhs < rhs);
	}

	template <class Key, class Type, class Traits, class Allocator>
	inline bool
	operator<=(MapVector<Key, Type, Traits, Allocator> const & lhs,
					MapVector<Key, Type, Traits, Allocator> const & rhs)
	{
		return !(rhs < lhs);
	}
}

#endif			// _MAPVECTOR_HPP
